#pragma once
#include <quickjs++/value.h>
#include <unicode/datefmt.h>
#include <unicode/numberformatter.h>
#include <unicode/reldatefmt.h>
#include <unicode/timezone.h>

namespace Intl
{
    class IntlBase
    {
    public:
        IntlBase(JSContext* ctx, const qjs::rest<qjs::value>& input);
        virtual ~IntlBase() = default;
    protected:
        JSContext* m_ctx;
        qjs::value m_options;
        icu::Locale m_locale;
    private:
        const icu::Locale& resolveLocale(const qjs::rest<qjs::value>& input);
    };

    class DateTimeFormat : public IntlBase
    {
    public:
        DateTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input);
        icu::UnicodeString format(double offset) const;
        qjs::value resolvedOptions() const;
    private:
        std::unique_ptr<icu::DateFormat> m_formatter;
        std::unique_ptr<icu::TimeZone> m_timeZone;

        icu::DateFormat::EStyle parseFormatStyle(const char* key) const;
    };

    class NumberFormat : public IntlBase
    {
    public:
        NumberFormat(JSContext* ctx, const qjs::rest<qjs::value>& input);
        icu::UnicodeString format(double number) const;
        static icu::UnicodeString toLocaleString(
            double number, JSContext* ctx, const qjs::rest<qjs::value>& input);
    private:
        struct StyleProperties
        {
            std::string styleIn;
            icu::MeasureUnit unit;
            UNumberSignDisplay unitSign = UNUM_SIGN_AUTO;
            UNumberUnitWidth unitWidth = UNUM_UNIT_WIDTH_SHORT;
            int32_t currencyDigits = -1;
        };

        std::optional<icu::number::LocalizedNumberFormatter> m_formatter;

        std::pair<icu::number::Notation, std::string> getNotation() const;
        icu::number::Precision getPrecision(
            std::string_view styleIn, std::string_view notationIn, int32_t currencyDigits) const;
        StyleProperties getStyleProperties() const;
    };

    class RelativeTimeFormat : public IntlBase
    {
    public:
        RelativeTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input);
        icu::UnicodeString format(double offset, std::string_view unit) const;
    private:
        bool m_alwaysNumeric = true;
        std::optional<icu::RelativeDateTimeFormatter> m_formatter;
    };

    void registerFor(qjs::context& ctx);
}