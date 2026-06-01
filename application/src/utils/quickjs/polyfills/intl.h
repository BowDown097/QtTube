#pragma once
#include <QLocale>
#include <quickjs++/value.h>
#include <unicode/uversion.h>

U_NAMESPACE_BEGIN
class RelativeDateTimeFormatter;
U_NAMESPACE_END

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

    struct NumberFormat : IntlBase
    {
        NumberFormat(JSContext* ctx, const qjs::rest<qjs::value>& input) : IntlBase(ctx, input) {}
        QString format(double number);
    };

    class RelativeTimeFormat : public IntlBase
    {
    public:
        RelativeTimeFormat(JSContext* ctx, const qjs::rest<qjs::value>& input);
        ~RelativeTimeFormat();
        QString format(double value, const std::string& unit);
    private:
        bool m_alwaysNumeric = true;
        std::unique_ptr<icu::RelativeDateTimeFormatter> m_formatter;
    };

    void registerFor(qjs::context& ctx);
}