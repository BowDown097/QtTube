#pragma once
#include <any>
#include <quickjs++/js_traits.h>
#include <QVariant>

namespace qjs
{
    template<> struct js_traits<QByteArray>
    {
        static QByteArray unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, const QByteArray& v);
    };

    template<> struct js_traits<QString>
    {
        static QString unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, const QString& v);
    };

    template<> struct js_traits<QLatin1String>
    {
        static QLatin1String unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, QLatin1String v);
    };

    template<> struct js_traits<QUrl>
    {
        static QUrl unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, const QUrl& v);
    };

    template<> struct js_traits<QVariant>
    {
        static QVariant unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, const QVariant& v);
    };

    template<typename Key, typename T>
    struct js_traits<QHash<Key, T>>
    {
        static QHash<Key, T> unwrap(JSContext* ctx, JSValueConst val)
        {
            return detail::make_from_range<QHash<Key, T>>(
                detail::get_property_entries<Key, T>(ctx, val));
        }

        static JSValue wrap(JSContext* ctx, const QHash<Key, T>& val)
        {
            JSValue result = JS_NewObject(ctx);

            for (auto it = val.begin(); it != val.end(); ++it)
            {
                JSValue wrapped = js_traits<T>::wrap(ctx, it.value());
                property_traits<Key>::set(ctx, result, it.key(), wrapped);
            }

            return result;
        }
    };

    template<> struct js_traits<std::any>
    {
        static std::any unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, const std::any& v);
    };

    template<> struct property_traits<QByteArray>
    {
        static JSValue get(JSContext* ctx, JSValueConst this_obj, const QByteArray& name) noexcept;
        static void set(JSContext* ctx, JSValueConst this_obj, const QByteArray& name, JSValue val);
    };

    template<> struct property_traits<QString>
    {
        static JSValue get(JSContext* ctx, JSValueConst this_obj, const QString& name);
        static void set(JSContext* ctx, JSValueConst this_obj, const QString& name, JSValue val);
    };

    template<> struct property_traits<QLatin1String>
    {
        static JSValue get(JSContext* ctx, JSValueConst this_obj, QLatin1String name) noexcept;
        static void set(JSContext* ctx, JSValueConst this_obj, QLatin1String name, JSValue val);
    };
}