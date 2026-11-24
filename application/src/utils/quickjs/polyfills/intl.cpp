#include "intl.hpp"
#include "utils/hashfunctions.hpp"
#include "utils/quickjs/qjsutils.hpp"
#include <quickjs++/context.h>
#include <unicode/dtptngen.h>
#include <unicode/localematcher.h>

#define THROW_INCOMPATIBLE_OPTIONS(a, b) \
    throw qjs::exception(m_ctx, JS_TYPE_ERROR, "can't set option " a " when " b " is used")
#define THROW_INTERNAL_ERROR(value) \
    throw qjs::exception(m_ctx, JS_INTERNAL_ERROR, "unexpected formatting error: %s", value)
#define THROW_INVALID_VALUE_FMT(name, value, fmt) \
    throw qjs::exception(m_ctx, JS_RANGE_ERROR, "invalid value " fmt " for option " name, value)
#define THROW_INVALID_VALUE(name, value) THROW_INVALID_VALUE_FMT(name, value, "\"%s\"")
#define THROW_UNDEFINED_STYLE(formatter, name) \
    throw qjs::exception(m_ctx, JS_TYPE_ERROR, "undefined " name " in " formatter "() with " name " style")

constexpr std::array goodIncrementValues = {
    1, 2, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000, 2000, 2500, 5000
};

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
    template<typename T = icu::Locale>
    std::span<const icu::Locale> getAvailableLocales()
    {
        int32_t count;
        const icu::Locale* locales = T::getAvailableLocales(count);
        return std::span(locales, count);
    }

    template<typename T = std::string_view>
    T getValue(const qjs::value& options, const char* key)
    {
        return QJSUtils::unwrapObjectProperty<T>(options, key);
    }

    std::optional<int32_t> getValueDigits(const qjs::value& options, const char* key)
    {
        if (auto value = QJSUtils::unwrapObjectProperty<qjs::value>(options, key); value.is_number())
        {
            double _value;
            JS_ToFloat64(value.ctx, &_value, value.v);
            if (std::isnan(_value) || std::isinf(_value))
                throw qjs::exception(value.ctx, JS_RANGE_ERROR, "invalid digits value: %f", _value);
            else
                return static_cast<int32_t>(_value);
        }
        else
        {
            return std::nullopt;
        }
    }

    const icu::Locale& localeBestFit(const std::vector<std::string>& requested)
    {
        static const icu::LocaleMatcher matcher = [] {
            std::span<const icu::Locale> locales = getAvailableLocales();
            UErrorCode status = U_ZERO_ERROR;
            icu::LocaleMatcher result = icu::LocaleMatcher::Builder()
                .setSupportedLocales(locales.begin(), locales.end())
                .setNoDefaultLocale()
                .build(status);

            if (U_FAILURE(status))
                throw std::runtime_error(u_errorName(status));

            return result;
        }();

        for (const std::string& req : requested)
        {
            UErrorCode status = U_ZERO_ERROR;
            icu::Locale locale = icu::Locale::forLanguageTag(req, status);
            if (U_SUCCESS(status))
                if (const icu::Locale* match = matcher.getBestMatch(locale, status))
                    return *match;
        }

        return icu::Locale::getDefault();
    }

    const icu::Locale& localeLookup(const std::vector<std::string>& requested)
    {
        using LocaleMap = std::unordered_map<
            std::string, const icu::Locale*, CaseInsensitiveHash, CaseInsensitiveEqual>;
        static const auto availableLocales = qjs::detail::make_from_range<LocaleMap>(
            getAvailableLocales() | std::views::transform([](const icu::Locale& loc) {
                UErrorCode status = U_ZERO_ERROR;
                return std::make_pair(loc.toLanguageTag<std::string>(status), &loc);
            })
        );

        for (std::string_view req : requested)
        {
            if (size_t pos = req.find("-u-"); pos != std::string_view::npos)
                req = req.substr(0, pos);

            while (!req.empty())
            {
                if (auto it = availableLocales.find(req); it != availableLocales.end())
                    return *it->second;

                size_t pos = req.rfind('-');
                if (pos == std::string_view::npos)
                    break;

                req = req.substr(0, pos);
            }
        }

        return icu::Locale::getDefault();
    }
}

namespace Intl
{
    const icu::Locale& IntlBase::resolveLocale(const qjs::rest<qjs::value>& input)
    {
        std::vector<std::string> locales;
        if (!input.empty())
        {
            if (input[0].is_string())
                locales.push_back(input[0].as<std::string>());
            else if (input[0].is_array())
                locales = input[0].as<std::vector<std::string>>();
        }

        if (!locales.empty())
        {
            try
            {
                if (getValue(m_options, "localeMatcher") == "lookup")
                    return localeLookup(locales);
                else
                    return localeBestFit(locales);
            }
            catch (const std::runtime_error& ex)
            {
                THROW_INTERNAL_ERROR(ex.what());
            }
        }

        return icu::Locale::getDefault();
    }

    IntlBase::IntlBase(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : m_ctx(ctx),
          m_options(input.size() > 1 ? input[1] : qjs::value(ctx, JS_UNDEFINED)),
          m_locale(resolveLocale(input)) {}

    DateTimeFormat::DateTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : IntlBase(ctx, input)
    {
        icu::DateFormat::EStyle dateStyle = parseFormatStyle("dateStyle");
        icu::DateFormat::EStyle timeStyle = parseFormatStyle("timeStyle");

        if (auto timeZoneIn = getValue<icu::UnicodeString>(m_options, "timeZone"); !timeZoneIn.isEmpty())
            m_timeZone.reset(icu::TimeZone::createTimeZone(timeZoneIn));
        else
            m_timeZone.reset(icu::TimeZone::createDefault());

        if (dateStyle != icu::DateFormat::kNone || timeStyle != icu::DateFormat::kNone)
        {
            m_formatter.reset(icu::DateFormat::createDateTimeInstance(dateStyle, timeStyle, m_locale));
            m_formatter->setTimeZone(*m_timeZone);
        }
        else
        {
            m_formatter.reset(icu::DateFormat::createDateInstance(icu::DateFormat::kDefault, m_locale));
        }
    }

    icu::UnicodeString DateTimeFormat::format(double offset) const
    {
        UErrorCode status = U_ZERO_ERROR;
        icu::UnicodeString result;

        m_formatter->format(static_cast<UDate>(offset), result, nullptr, status);
        if (U_FAILURE(status))
            THROW_INTERNAL_ERROR(u_errorName(status));

        return result;
    }

    icu::DateFormat::EStyle DateTimeFormat::parseFormatStyle(const char* key) const
    {
        std::string_view name = getValue(m_options, key);
        if (name.empty())
            return icu::DateFormat::kNone;

        if (name == "full")
            return icu::DateFormat::kFull;
        if (name == "long")
            return icu::DateFormat::kLong;
        if (name == "medium")
            return icu::DateFormat::kMedium;
        if (name == "short")
            return icu::DateFormat::kShort;

        THROW_INVALID_VALUE("dateStyle", name.data());
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
        result["locale"] = m_locale.toLanguageTag<std::string>(status);
        result["timeZone"] = timeZoneIanaId;

        if (auto dateStyleIn = getValue<qjs::value>(m_options, "dateStyle"); dateStyleIn.is_string())
            result["dateStyle"] = dateStyleIn.as<std::string>();
        if (auto timeStyleIn = getValue<qjs::value>(m_options, "timeStyle"); timeStyleIn.is_string())
            result["timeStyle"] = timeStyleIn.as<std::string>();

        return result;
    }

    NumberFormat::NumberFormat(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : IntlBase(ctx, input)
    {
        auto [styleIn, unit, unitSign, unitWidth, currencyDigits] = getStyleProperties();
        auto [notation, notationIn] = getNotation();

        int32_t minimumIntegerDigits = getValueDigits(m_options, "minimumIntegerDigits").value_or(1);
        if (minimumIntegerDigits < 1 || minimumIntegerDigits > 21)
            THROW_INVALID_VALUE_FMT("minimumIntegerDigits", minimumIntegerDigits, "%d");

        UNumberFormatRoundingMode roundingMode = UNUM_ROUND_HALFUP;
        if (std::string_view roundingModeIn = getValue(m_options, "roundingMode"); !roundingModeIn.empty())
        {
            if (roundingModeIn == "ceil")
                roundingMode = UNUM_ROUND_CEILING;
            else if (roundingModeIn == "floor")
                roundingMode = UNUM_ROUND_FLOOR;
            else if (roundingModeIn == "expand")
                roundingMode = UNUM_ROUND_UP;
            else if (roundingModeIn == "trunc")
                roundingMode = UNUM_ROUND_DOWN;
            else if (roundingModeIn == "halfCeil")
                roundingMode = UNUM_ROUND_HALF_CEILING;
            else if (roundingModeIn == "halfFloor")
                roundingMode = UNUM_ROUND_HALF_FLOOR;
            else if (roundingModeIn == "halfTrunc")
                roundingMode = UNUM_ROUND_HALFDOWN;
            else if (roundingModeIn == "halfEven")
                roundingMode = UNUM_ROUND_HALFEVEN;
            else if (roundingModeIn != "halfExpand")
                THROW_INVALID_VALUE("roundingMode", roundingModeIn.data());
        }

        UNumberGroupingStrategy grouping = UNUM_GROUPING_AUTO;
        if (std::string_view groupingIn = getValue(m_options, "grouping"); !groupingIn.empty())
        {
            if (groupingIn == "always" || groupingIn == "true")
                grouping = UNUM_GROUPING_ON_ALIGNED;
            else if (groupingIn == "min2")
                grouping = UNUM_GROUPING_MIN2;
            else if (groupingIn == "false")
                grouping = UNUM_GROUPING_OFF;
            else if (groupingIn != "auto")
                THROW_INVALID_VALUE("grouping", groupingIn.data());
        }

        if (std::string_view signDisplayIn = getValue(m_options, "signDisplay"); !signDisplayIn.empty())
        {
            bool accounting = unitSign == UNUM_SIGN_ACCOUNTING;
            if (signDisplayIn == "always")
                unitSign = accounting ? UNUM_SIGN_ACCOUNTING_ALWAYS : UNUM_SIGN_ALWAYS;
            else if (signDisplayIn == "exceptZero")
                unitSign = accounting ? UNUM_SIGN_ACCOUNTING_EXCEPT_ZERO : UNUM_SIGN_EXCEPT_ZERO;
            else if (signDisplayIn == "negative")
                unitSign = accounting ? UNUM_SIGN_ACCOUNTING_NEGATIVE : UNUM_SIGN_NEGATIVE;
            else if (signDisplayIn == "never")
                unitSign = UNUM_SIGN_NEVER;
            else if (signDisplayIn != "auto")
                THROW_INVALID_VALUE("signDisplay", signDisplayIn.data());
        }

        m_formatter = icu::number::NumberFormatter::withLocale(m_locale)
            .notation(notation)
            .unit(unit)
            .precision(getPrecision(styleIn, notationIn, currencyDigits))
            .roundingMode(roundingMode)
            .grouping(grouping)
            .integerWidth(icu::number::IntegerWidth::zeroFillTo(minimumIntegerDigits))
            .unitWidth(unitWidth)
            .sign(unitSign);
    }

    icu::UnicodeString NumberFormat::format(double number) const
    {
        UErrorCode status = U_ZERO_ERROR;
        icu::UnicodeString result = m_formatter->formatDouble(number, status).toString(status);
        if (U_FAILURE(status))
            THROW_INTERNAL_ERROR(u_errorName(status));

        return result;
    }

    std::pair<icu::number::Notation, std::string> NumberFormat::getNotation() const
    {
        auto notation = icu::number::Notation::simple();
        std::string_view notationIn = getValue(m_options, "notation");
        if (!notationIn.empty())
        {
            if (notationIn == "scientific")
            {
                notation = icu::number::Notation::scientific();
            }
            else if (notationIn == "compact")
            {
                if (std::string_view displayIn = getValue(m_options, "compactDisplay"); !displayIn.empty())
                {
                    if (displayIn == "short")
                        notation = icu::number::Notation::compactShort();
                    else if (displayIn == "long")
                        notation = icu::number::Notation::compactLong();
                    else
                        THROW_INVALID_VALUE("compactDisplay", displayIn.data());
                }
                else
                {
                    notation = icu::number::Notation::compactShort();
                }
            }
            else if (notationIn != "standard")
            {
                THROW_INVALID_VALUE("notation", notationIn.data());
            }
        }

        return std::make_pair(std::move(notation), std::string(notationIn));
    }

    icu::number::Precision NumberFormat::getPrecision(
        std::string_view styleIn, std::string_view notationIn, int32_t currencyDigits) const
    {
        std::optional<int32_t> minFdIn = getValueDigits(m_options, "minimumFractionDigits");
        if (minFdIn.has_value() && (minFdIn < 0 || minFdIn > 100))
            THROW_INVALID_VALUE_FMT("minimumFractionDigits", minFdIn.value(), "%d");

        std::optional<int32_t> maxFdIn = getValueDigits(m_options, "maximumFractionDigits");
        if (maxFdIn.has_value() && (maxFdIn < 0 || maxFdIn > 100))
            THROW_INVALID_VALUE_FMT("maximumFractionDigits", maxFdIn.value(), "%d");

        std::optional<int32_t> minSdIn = getValueDigits(m_options, "minimumSignificantDigits");
        if (minSdIn.has_value() && (minSdIn < 1 || minSdIn > 21))
            THROW_INVALID_VALUE_FMT("minimumSignificantDigits", minSdIn.value(), "%d");

        std::optional<int32_t> maxSdIn = getValueDigits(m_options, "maximumSignificantDigits");
        if (maxSdIn.has_value() && (maxSdIn < 1 || maxSdIn > 21))
            THROW_INVALID_VALUE_FMT("maximumSignificantDigits", maxSdIn.value(), "%d");

        int32_t roundingIncrement = getValueDigits(m_options, "roundingIncrement").value_or(1);
        if (!std::ranges::binary_search(goodIncrementValues, roundingIncrement))
            THROW_INVALID_VALUE_FMT("roundingIncrement", roundingIncrement, "%d");

        std::string_view roundingPriorityIn = getValue(m_options, "roundingPriority");
        if (!roundingPriorityIn.empty() &&
            roundingPriorityIn != "auto" &&
            roundingPriorityIn != "morePrecision" &&
            roundingPriorityIn != "lessPrecision")
        {
            THROW_INVALID_VALUE("roundingPriority", roundingPriorityIn.data());
        }

        UNumberTrailingZeroDisplay tzd = UNUM_TRAILING_ZERO_AUTO;
        if (std::string_view tzdIn = getValue(m_options, "trailingZeroDisplay"); !tzdIn.empty())
        {
            if (tzdIn == "stripIfInteger")
                tzd = UNUM_TRAILING_ZERO_HIDE_IF_WHOLE;
            else if (tzdIn != "auto")
                THROW_INVALID_VALUE("trailingZeroDisplay", tzdIn.data());
        }

        bool hasFd = minFdIn.has_value() || maxFdIn.has_value();
        bool hasSd = minSdIn.has_value() || maxSdIn.has_value();
        bool useCurrencyDigits = styleIn == "currency" && notationIn != "compact";

        int32_t minFd = useCurrencyDigits ? currencyDigits : 0;
        int32_t maxFd = useCurrencyDigits ? currencyDigits : (styleIn == "percent" ? 0 : 3);
        int32_t minSd = minSdIn.value_or(1);
        int32_t maxSd = maxSdIn.value_or(21);

        if (roundingIncrement != 1)
            maxFd = minFd;

        bool needSd = true, needFd = true;
        if (roundingPriorityIn == "auto")
        {
            needSd = hasSd;
            if (needSd || (!hasFd && notationIn == "compact"))
                needFd = false;
        }

        if (needFd)
        {
            if (minFdIn.has_value() && maxFdIn.has_value())
            {
                minFd = minFdIn.value();
                maxFd = maxFdIn.value();
            }
            else if (minFdIn.has_value())
            {
                minFd = minFdIn.value();
                maxFd = std::max(maxFd, minFd);
            }
            else if (maxFdIn.has_value())
            {
                maxFd = maxFdIn.value();
                minFd = std::min(minFd, maxFd);
            }
        }
        // if we aren't using anything, establish some defaults
        else if (!needSd)
        {
            minFd = 0;
            maxFd = 0;
            minSd = 1;
            maxSd = 2;
            roundingPriorityIn = "morePrecision";
            needSd = true;
            needFd = true;
        }

        if (roundingIncrement != 1)
        {
            if (maxSdIn.has_value())
                THROW_INCOMPATIBLE_OPTIONS("roundingIncrement", "maximumSignificantDigits");
            if (minSdIn.has_value())
                THROW_INCOMPATIBLE_OPTIONS("roundingIncrement", "minimumSignificantDigits");
            if (roundingPriorityIn != "auto")
                THROW_INCOMPATIBLE_OPTIONS("roundingIncrement", "roundingPriority");

            if (minFd != maxFd)
            {
                throw qjs::exception(m_ctx, JS_RANGE_ERROR,
                    "fraction digits must be the same when roundingIncrement is used");
            }

            return icu::number::Precision::incrementExact(
                static_cast<uint64_t>(roundingIncrement),
                static_cast<int16_t>(-maxFd)
            ).withMinFraction(minFd).trailingZeroDisplay(tzd);
        }

        if (!needSd)
            return icu::number::Precision::minMaxFraction(minFd, maxFd).trailingZeroDisplay(tzd);
        if (!needFd)
            return icu::number::Precision::minMaxSignificantDigits(minSd, maxSd).trailingZeroDisplay(tzd);

        UNumberRoundingPriority roundingPriority = (roundingPriorityIn == "lessPrecision")
            ? UNUM_ROUNDING_PRIORITY_STRICT : UNUM_ROUNDING_PRIORITY_RELAXED;
        return icu::number::Precision::minMaxFraction(minFd, maxFd)
            .withSignificantDigits(minSd, maxSd, roundingPriority)
            .trailingZeroDisplay(tzd);
    }

    NumberFormat::StyleProperties NumberFormat::getStyleProperties() const
    {
        NumberFormat::StyleProperties props = { .styleIn = getValue<std::string>(m_options, "style") };
        if (!props.styleIn.empty())
        {
            if (props.styleIn == "currency")
            {
                if (std::string_view currencyIn = getValue(m_options, "currency"); !currencyIn.empty())
                {
                    UErrorCode status = U_ZERO_ERROR;
                    icu::CurrencyUnit currencyUnit(currencyIn, status);
                    if (U_FAILURE(status))
                        THROW_INVALID_VALUE("currency", currencyIn.data());

                    int32_t currencyDigits = ucurr_getDefaultFractionDigits(
                        currencyUnit.getISOCurrency(), &status);
                    props.currencyDigits = U_SUCCESS(status) ? currencyDigits : 2;
                    props.unit = std::move(currencyUnit);
                }
                else
                {
                    THROW_UNDEFINED_STYLE("NumberFormat", "currency");
                }

                if (std::string_view displayIn = getValue(m_options, "currencyDisplay"); !displayIn.empty())
                {
                    if (displayIn == "code")
                        props.unitWidth = UNUM_UNIT_WIDTH_ISO_CODE;
                    else if (displayIn == "narrowSymbol")
                        props.unitWidth = UNUM_UNIT_WIDTH_NARROW;
                    else if (displayIn == "name")
                        props.unitWidth = UNUM_UNIT_WIDTH_FULL_NAME;
                    else if (displayIn != "symbol")
                        THROW_INVALID_VALUE("currencyDisplay", displayIn.data());
                }

                if (std::string_view signIn = getValue(m_options, "currencySign"); !signIn.empty())
                {
                    if (signIn == "accounting")
                        props.unitSign = UNUM_SIGN_ACCOUNTING;
                    else if (signIn != "standard")
                        THROW_INVALID_VALUE("currencySign", signIn.data());
                }
            }
            else if (props.styleIn == "percent")
            {
                props.unit = icu::NoUnit::percent();
            }
            else if (props.styleIn == "unit")
            {
                if (std::string_view unitIn = getValue(m_options, "unit"); !unitIn.empty())
                {
                    UErrorCode status = U_ZERO_ERROR;
                    props.unit = icu::MeasureUnit::forIdentifier(unitIn, status);
                    if (U_FAILURE(status))
                        THROW_INVALID_VALUE("unit", unitIn.data());
                }
                else
                {
                    THROW_UNDEFINED_STYLE("NumberFormat", "unit");
                }

                if (std::string_view displayIn = getValue(m_options, "unitDisplay"); !displayIn.empty())
                {
                    if (displayIn == "narrow")
                        props.unitWidth = UNUM_UNIT_WIDTH_NARROW;
                    else if (displayIn == "long")
                        props.unitWidth = UNUM_UNIT_WIDTH_FULL_NAME;
                    else if (displayIn != "short")
                        THROW_INVALID_VALUE("unitDisplay", displayIn.data());
                }
            }
            else if (props.styleIn != "decimal")
            {
                THROW_INVALID_VALUE("style", props.styleIn.data());
            }
        }

        return props;
    }

    icu::UnicodeString NumberFormat::toLocaleString(
        double number, JSContext* ctx, const qjs::rest<qjs::value>& input)
    {
        return NumberFormat(ctx, input).format(number);
    }

    RelativeTimeFormat::RelativeTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input)
        : IntlBase(ctx, input)
    {
        UDateRelativeDateTimeFormatterStyle style = UDAT_STYLE_LONG;
        if (std::string_view styleIn = getValue(m_options, "style"); !styleIn.empty())
        {
            if (styleIn == "short")
                style = UDAT_STYLE_SHORT;
            else if (styleIn == "narrow")
                style = UDAT_STYLE_NARROW;
            else if (styleIn != "long")
                THROW_INVALID_VALUE("style", styleIn.data());
        }

        if (std::string_view numericIn = getValue(m_options, "numeric"); !numericIn.empty())
        {
            if (numericIn == "auto")
                m_alwaysNumeric = false;
            else if (numericIn != "always")
                THROW_INVALID_VALUE("numeric", numericIn.data());
        }

        UErrorCode status = U_ZERO_ERROR;

        if (std::string_view systemIn = getValue(m_options, "numberingSystem"); !systemIn.empty())
        {
            m_locale.setKeywordValue("numbers", systemIn.data(), status);
            if (U_FAILURE(status))
                THROW_INVALID_VALUE("numberingSystem", systemIn.data());
        }

        m_formatter.emplace(m_locale, nullptr, style, UDISPCTX_CAPITALIZATION_NONE, status);
        if (U_FAILURE(status))
            throw qjs::exception(ctx, JS_RANGE_ERROR, "invalid language tag: \"%s\"", m_locale.getName());
    }

    icu::UnicodeString RelativeTimeFormat::format(double offset, std::string_view unit) const
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
            THROW_INVALID_VALUE("unit", unit.data());

        UErrorCode status = U_ZERO_ERROR;
        icu::FormattedRelativeDateTime formatted = m_alwaysNumeric
            ? m_formatter->formatNumericToValue(offset, unitValue, status)
            : m_formatter->formatToValue(offset, unitValue, status);
        if (U_FAILURE(status))
            THROW_INTERNAL_ERROR(u_errorName(status));

        icu::UnicodeString result = formatted.toTempString(status);
        if (U_FAILURE(status))
            THROW_INTERNAL_ERROR(u_errorName(status));

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
        ctx.global()["Number"]["prototype"]["toLocaleString"] =
            qjs::make_fwrapper<true>(&NumberFormat::toLocaleString, "toLocaleString");
    }
}