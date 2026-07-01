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

    template<> struct js_traits<QVariant>
    {
        static QVariant unwrap(JSContext* ctx, JSValueConst v);
        static JSValue wrap(JSContext* ctx, const QVariant& v);
    };

    template<> struct js_traits<std::any>
    {
        static std::any unwrap(JSContext* ctx, JSValueConst v) noexcept;
        static JSValue wrap(JSContext* ctx, const std::any& v);
    };

    template<> struct js_traits<std::monostate>
    {
        static std::monostate unwrap(JSContext* ctx, JSValueConst v) noexcept;
        static JSValue wrap(JSContext* ctx, std::monostate v) noexcept;
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