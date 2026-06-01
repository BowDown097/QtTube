#include "intl.h"
#include "utils/hashfunctions.h"
#include "utils/quickjs/qt_js_traits.h"
#include <quickjs++/context.h>
#include <unicode/reldatefmt.h>
#include <unordered_set>

#define INVALID_VALUE(name) "invalid value \"%s\" for option " #name

template<typename T>
T getValue(const qjs::value& options, const char* key)
{
    if (!JS_IsUndefined(options.v))
        return options[key].as<T>();
    else if constexpr (std::same_as<T, qjs::value>)
        return qjs::value(JS_UNDEFINED);
    else
        return {};
}

namespace Intl
{
    QStringList IntlBase::getLocaleList(const qjs::value& locales)
    {
        if (JS_IsString(locales.v))
            return QStringList { locales.as<QString>() };
        if (JS_IsArray(locales.v))
            return locales.as<QStringList>();
        return {};
    }

    QString IntlBase::resolveLocale(const qjs::rest<qjs::value>& input)
    {
        using LocaleSet = std::unordered_set<QString, CaseInsensitiveHash, CaseInsensitiveEqual>;

        if (!input.empty())
        {
            static const LocaleSet available = [] {
                const QList<QLocale> locales = QLocale::matchingLocales(
                    QLocale::AnyLanguage,
                    QLocale::AnyScript,
                    QLocale::AnyTerritory
                );

                auto v = locales | std::views::transform([](const QLocale& l) { return l.bcp47Name(); });
                return LocaleSet(v.begin(), v.end());
            }();

            const QStringList requested = getLocaleList(input[0]);
            for (QString locale : requested)
            {
                while (!locale.isEmpty())
                {
                    if (available.contains(locale))
                        return locale;

                    qsizetype pos = locale.lastIndexOf(u'-');
                    if (pos < 0)
                        break;

                    locale = locale.left(pos);
                }
            }
        }

        return QLocale::system().name(QLocale::TagSeparator::Dash);
    }

    IntlBase::IntlBase(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : m_ctx(ctx),
          m_locale(resolveLocale(input)),
          m_options(input.size() > 1 ? input[1] : qjs::value(JS_UNDEFINED)) {}

    QString NumberFormat::format(double number)
    {
        auto style = getValue<std::string_view>(m_options, "style");

        if (style == "currency")
        {
            QString symbol;

            if (auto currencyVal = getValue<qjs::value>(m_options, "currency"); JS_IsString(currencyVal.v))
            {
                symbol = currencyVal.as<QString>();
            }
            else if (auto displayVal = getValue<qjs::value>(m_options, "currencyDisplay"); JS_IsString(displayVal.v))
            {
                std::string_view display = displayVal.as<std::string_view>();
                if (display == "code")
                    symbol = m_locale.currencySymbol(QLocale::CurrencyIsoCode);
                else if (display == "name")
                    symbol = m_locale.currencySymbol(QLocale::CurrencyDisplayName);
            }

            return m_locale.toCurrencyString(number, symbol);
        }

        char fmt = getValue<std::string_view>(m_options, "notation") == "scientific" ? 'E' : 'f';
        QString result = m_locale.toString(number, fmt, QLocale::FloatingPointShortest);

        if (style == "percent")
            result += m_locale.percent();

        return result;
    }

    RelativeTimeFormat::~RelativeTimeFormat() = default;

    RelativeTimeFormat::RelativeTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : IntlBase(ctx, input)
    {
        UDateRelativeDateTimeFormatterStyle style = UDAT_STYLE_LONG;
        if (auto styleValue = getValue<qjs::value>(m_options, "style"); !JS_IsUndefined(styleValue.v))
        {
            std::string_view styleName = styleValue.as<std::string_view>();
            if (styleName == "short")
                style = UDAT_STYLE_SHORT;
            else if (styleName == "narrow")
                style = UDAT_STYLE_NARROW;
            else if (styleName != "long")
                throw qjs::exception(ctx, JS_RANGE_ERROR, INVALID_VALUE("style"), styleName.data());
        }

        if (auto numericValue = getValue<qjs::value>(m_options, "numeric"); !JS_IsUndefined(numericValue.v))
        {
            std::string_view numericName = numericValue.as<std::string_view>();
            if (numericName == "auto")
                m_alwaysNumeric = false;
            else if (numericName != "always")
                throw qjs::exception(ctx, JS_RANGE_ERROR, INVALID_VALUE("numeric"), numericName.data());
        }

        icu::Locale locale = icu::Locale::getDefault();
        UErrorCode status = U_ZERO_ERROR;

        if (auto systemValue = getValue<qjs::value>(m_options, "numberingSystem"); !JS_IsUndefined(systemValue.v))
        {
            std::string_view systemName = systemValue.as<std::string_view>();
            locale.setKeywordValue("numbers", systemName.data(), status);
            if (U_FAILURE(status))
                throw qjs::exception(ctx, JS_RANGE_ERROR, INVALID_VALUE("numberingSystem"), systemName.data());
        }

        m_formatter = std::make_unique<icu::RelativeDateTimeFormatter>(
            locale,
            nullptr,
            style,
            UDISPCTX_CAPITALIZATION_NONE,
            status);
        if (U_FAILURE(status))
            throw qjs::exception(ctx, JS_RANGE_ERROR, "invalid language tag: \"%s\"", locale.getName());
    }

    QString RelativeTimeFormat::format(double value, const std::string& unit)
    {
        URelativeDateTimeUnit unitValue;
        if (unit == "year" || unit == "years")
            unitValue = UDAT_REL_UNIT_YEAR;
        else if (unit == "quarter" || unit == "quarters")
            unitValue = UDAT_REL_UNIT_QUARTER;
        else if (unit == "month" || unit == "months")
            unitValue = UDAT_REL_UNIT_MONTH;
        else if (unit == "week" || unit == "weeks")
            unitValue = UDAT_REL_UNIT_WEEK;
        else if (unit == "day" || unit == "days")
            unitValue = UDAT_REL_UNIT_DAY;
        else if (unit == "hour" || unit == "hours")
            unitValue = UDAT_REL_UNIT_HOUR;
        else if (unit == "minute" || unit == "minutes")
            unitValue = UDAT_REL_UNIT_MINUTE;
        else if (unit == "second" || unit == "seconds")
            unitValue = UDAT_REL_UNIT_SECOND;
        else
            throw qjs::exception(m_ctx, JS_RANGE_ERROR, INVALID_VALUE("unit"), unit.c_str());

        UErrorCode status = U_ZERO_ERROR;
        icu::FormattedRelativeDateTime formatted = m_alwaysNumeric
            ? m_formatter->formatNumericToValue(value, unitValue, status)
            : m_formatter->formatToValue(value, unitValue, status);
        if (U_FAILURE(status))
            throw qjs::exception(m_ctx, JS_INTERNAL_ERROR, "Unexpected formatting error: %s", u_errorName(status));

        icu::UnicodeString str = formatted.toTempString(status);
        if (U_FAILURE(status))
            throw qjs::exception(m_ctx, JS_INTERNAL_ERROR, "Unexpected formatting error: %s", u_errorName(status));

        return QString(reinterpret_cast<const QChar*>(str.getBuffer()), str.length());
    }

    void registerFor(qjs::context& ctx)
    {
        qjs::module& mod = ctx.add_module("Intl");
        mod.register_class<Intl::NumberFormat>("NumberFormat")
            .constructor<JSContext*, const qjs::rest<qjs::value>&>()
            .member<&Intl::NumberFormat::format>("format");
        mod.register_class<Intl::RelativeTimeFormat>("RelativeTimeFormat")
            .constructor<JSContext*, const qjs::rest<qjs::value>&>()
            .member<&Intl::RelativeTimeFormat::format>("format");

        auto call_nf = [](double number, JSContext* ctx, const qjs::rest<qjs::value>& input) {
            return NumberFormat(ctx, input).format(number);
        };

        ctx.global()["Number"]["prototype"]["toLocaleString"] =
            qjs::fwrapper<decltype(call_nf), true> { call_nf, "toLocaleString" };
    }
}