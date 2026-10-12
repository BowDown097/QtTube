#pragma once
#include <QLocale>
#include <quickjs++/value.h>
#include <unicode/datefmt.h>
#include <unicode/reldatefmt.h>
#include <unicode/timezone.h>
#include <unicode/uversion.h>

namespace Intl
{
    class IntlBase
    {
    public:
        IntlBase(JSContext* ctx, const qjs::rest<qjs::value>& input);
        virtual ~IntlBase() = default;
    protected:
        JSContext* m_ctx;
        QLocale m_locale;
        qjs::value m_options;
    private:
        static QStringList getLocaleList(const qjs::value& locales);
        static QString resolveLocale(const qjs::rest<qjs::value>& input);
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

    struct NumberFormat : IntlBase
    {
        NumberFormat(JSContext* ctx, const qjs::rest<qjs::value>& input) : IntlBase(ctx, input) {}
        QString format(double number) const;
    };

    class RelativeTimeFormat : public IntlBase
    {
    public:
        RelativeTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input);
        icu::UnicodeString format(double offset, const std::string& unit) const;
    private:
        bool m_alwaysNumeric = true;
        std::unique_ptr<icu::RelativeDateTimeFormatter> m_formatter;
    };

    void registerFor(qjs::context& ctx);
}