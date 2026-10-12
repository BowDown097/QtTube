#include "intl.hpp"
#include "utils/hashfunctions.hpp"
#include "utils/quickjs/qt_js_traits.hpp"
#include <quickjs++/context.h>
#include <unicode/dtptngen.h>
#include <unordered_set>

#define INVALID_VALUE(name) "invalid value \"%s\" for option " #name

template<> struct qjs::js_traits<icu::UnicodeString>
{
    static icu::UnicodeString unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = qjs::js_traits<std::u16string_view>::unwrap(ctx, v);
        return icu::UnicodeString(sv.data(), sv.size());
    }

    static JSValue wrap(JSContext* ctx, const icu::UnicodeString& v)
    {
        return JS_NewStringUTF16(ctx, reinterpret_cast<const uint16_t*>(v.getBuffer()), v.length());
    }
};

namespace
{
    template<typename T>
    T getValue(const qjs::value& options, const char* key)
    {
        if (!options.is_undefined())
            return options[key].as<T>();
        else if constexpr (std::same_as<T, qjs::value>)
            return qjs::value(JS_UNDEFINED);
        else
            return {};
    }

    icu::Locale makeIcuLocale(const QLocale& locale)
    {
        UErrorCode status = U_ZERO_ERROR;
        const QByteArray ba = locale.bcp47Name().toUtf8();
        icu::Locale result = icu::Locale::forLanguageTag(
            icu::StringPiece(ba.data(), ba.size()), status);
        return U_SUCCESS(status) && !result.isBogus() ? result : icu::Locale::getDefault();
    }
}

namespace Intl
{
    QStringList IntlBase::getLocaleList(const qjs::value& locales)
    {
        if (locales.is_string())
            return QStringList { locales.as<QString>() };
        if (locales.is_array())
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
          m_options(input.size() > 1 ? input[1] : qjs::value(ctx, JS_UNDEFINED)) {}

    DateTimeFormat::DateTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : IntlBase(ctx, input)
    {
        icu::DateFormat::EStyle dateStyle = parseFormatStyle("dateStyle");
        icu::DateFormat::EStyle timeStyle = parseFormatStyle("timeStyle");

        if (auto timeZoneVal = getValue<qjs::value>(m_options, "timeZone"); timeZoneVal.is_string())
            m_timeZone.reset(icu::TimeZone::createTimeZone(timeZoneVal.as<icu::UnicodeString>()));
        else
            m_timeZone.reset(icu::TimeZone::createDefault());

        icu::Locale locale = makeIcuLocale(m_locale);
        UErrorCode status = U_ZERO_ERROR;

        if (dateStyle != icu::DateFormat::kNone || timeStyle != icu::DateFormat::kNone)
        {
            m_formatter.reset(icu::DateFormat::createDateTimeInstance(dateStyle, timeStyle, locale));
            m_formatter->setTimeZone(*m_timeZone);
        }
        else
        {
            m_formatter.reset(icu::DateFormat::createDateInstance(icu::DateFormat::kDefault, locale));
        }
    }

    icu::UnicodeString DateTimeFormat::format(double offset) const
    {
        UErrorCode status = U_ZERO_ERROR;
        icu::UnicodeString result;

        m_formatter->format(static_cast<UDate>(offset), result, nullptr, status);
        if (U_FAILURE(status))
            throw qjs::exception(m_ctx, JS_INTERNAL_ERROR, "Unexpected formatting error: %s", u_errorName(status));

        return result;
    }

    icu::DateFormat::EStyle DateTimeFormat::parseFormatStyle(const char* key) const
    {
        qjs::value value = getValue<qjs::value>(m_options, key);
        if (value.is_undefined())
            return icu::DateFormat::kNone;

        std::string_view name = value.as<std::string_view>();
        if (name == "full")
            return icu::DateFormat::kFull;
        if (name == "long")
            return icu::DateFormat::kLong;
        if (name == "medium")
            return icu::DateFormat::kMedium;
        if (name == "short")
            return icu::DateFormat::kShort;

        throw qjs::exception(m_ctx, JS_RANGE_ERROR, INVALID_VALUE("dateStyle"), name.data());
    }

    qjs::value DateTimeFormat::resolvedOptions() const
    {
        UErrorCode status = U_ZERO_ERROR;
        icu::UnicodeString timeZoneId, timeZoneIanaId;

        m_timeZone->getID(timeZoneId);
        icu::TimeZone::getIanaID(timeZoneId, timeZoneIanaId, status);

        if (U_FAILURE(status))
        {
            std::string timeZoneIdUTF8;
            timeZoneId.toUTF8String(timeZoneIdUTF8);
            throw qjs::exception(
                m_ctx, JS_RANGE_ERROR,
                "invalid time zone: %s", timeZoneIdUTF8.c_str());
        }

        qjs::value result(m_ctx, JS_NewObject(m_ctx));
        result["locale"] = m_locale.bcp47Name();
        result["timeZone"] = timeZoneIanaId;

        if (auto dateStyle = getValue<qjs::value>(m_options, "dateStyle"); dateStyle.is_string())
            result["dateStyle"] = dateStyle.as<std::string>();
        if (auto timeStyle = getValue<qjs::value>(m_options, "timeStyle"); timeStyle.is_string())
            result["timeStyle"] = timeStyle.as<std::string>();

        return result;
    }

    QString NumberFormat::format(double number) const
    {
        auto style = getValue<std::string_view>(m_options, "style");

        if (style == "currency")
        {
            QString symbol;

            if (auto currencyVal = getValue<qjs::value>(m_options, "currency"); currencyVal.is_string())
            {
                symbol = currencyVal.as<QString>();
            }
            else if (auto displayVal = getValue<qjs::value>(m_options, "currencyDisplay"); displayVal.is_string())
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

    RelativeTimeFormat::RelativeTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : IntlBase(ctx, input)
    {
        UDateRelativeDateTimeFormatterStyle style = UDAT_STYLE_LONG;
        if (auto styleValue = getValue<qjs::value>(m_options, "style"); !styleValue.is_undefined())
        {
            std::string_view styleName = styleValue.as<std::string_view>();
            if (styleName == "short")
                style = UDAT_STYLE_SHORT;
            else if (styleName == "narrow")
                style = UDAT_STYLE_NARROW;
            else if (styleName != "long")
                throw qjs::exception(ctx, JS_RANGE_ERROR, INVALID_VALUE("style"), styleName.data());
        }

        if (auto numericValue = getValue<qjs::value>(m_options, "numeric"); !numericValue.is_undefined())
        {
            std::string_view numericName = numericValue.as<std::string_view>();
            if (numericName == "auto")
                m_alwaysNumeric = false;
            else if (numericName != "always")
                throw qjs::exception(ctx, JS_RANGE_ERROR, INVALID_VALUE("numeric"), numericName.data());
        }

        icu::Locale locale = makeIcuLocale(m_locale);
        UErrorCode status = U_ZERO_ERROR;

        if (auto systemValue = getValue<qjs::value>(m_options, "numberingSystem"); !systemValue.is_undefined())
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

    icu::UnicodeString RelativeTimeFormat::format(double offset, const std::string& unit) const
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
            ? m_formatter->formatNumericToValue(offset, unitValue, status)
            : m_formatter->formatToValue(offset, unitValue, status);
        if (U_FAILURE(status))
            throw qjs::exception(m_ctx, JS_INTERNAL_ERROR, "Unexpected formatting error: %s", u_errorName(status));

        icu::UnicodeString result = formatted.toTempString(status);
        if (U_FAILURE(status))
            throw qjs::exception(m_ctx, JS_INTERNAL_ERROR, "Unexpected formatting error: %s", u_errorName(status));

        return result;
    }

    void registerFor(qjs::context& ctx)
    {
        qjs::value intl = ctx.new_object();

        qjs::class_registrar<Intl::DateTimeFormat>(ctx, intl)
            .constructor<JSContext*, const qjs::rest<qjs::value>&>()
            .member<&Intl::DateTimeFormat::format>()
            .member<&Intl::DateTimeFormat::resolvedOptions>();
        qjs::class_registrar<Intl::NumberFormat>(ctx, intl)
            .constructor<JSContext*, const qjs::rest<qjs::value>&>()
            .member<&Intl::NumberFormat::format>();
        qjs::class_registrar<Intl::RelativeTimeFormat>(ctx, intl)
            .constructor<JSContext*, const qjs::rest<qjs::value>&>()
            .member<&Intl::RelativeTimeFormat::format>();

        ctx.global()["Intl"] = std::move(intl);

        auto call_nf = [](double number, JSContext* ctx, const qjs::rest<qjs::value>& input) {
            return NumberFormat(ctx, input).format(number);
        };

        ctx.global()["Number"]["prototype"]["toLocaleString"] =
            qjs::fwrapper<decltype(call_nf), true> { call_nf, "toLocaleString" };
    }
}