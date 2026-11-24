#include "qt_js_traits.hpp"
#include <quickjs++/value.h>
#include <QUrl>

namespace qjs
{
    QByteArray js_traits<QByteArray>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = js_traits<std::string_view>::unwrap(ctx, v);
        return QByteArray(sv.data(), sv.size());
    }

    JSValue js_traits<QByteArray>::wrap(JSContext* ctx, const QByteArray& v)
    {
        return JS_NewStringLen(ctx, v.data(), v.size());
    }

    QString js_traits<QString>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = js_traits<std::u16string_view>::unwrap(ctx, v);
        return QString(reinterpret_cast<const QChar*>(sv.data()), sv.size());
    }

    JSValue js_traits<QString>::wrap(JSContext* ctx, const QString& v)
    {
        return JS_NewStringUTF16(ctx, reinterpret_cast<const uint16_t*>(v.data()), v.size());
    }

    QLatin1String js_traits<QLatin1String>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = js_traits<std::string_view>::unwrap(ctx, v);
        return QLatin1String(sv.data(), sv.size());
    }

    JSValue js_traits<QLatin1String>::wrap(JSContext* ctx, QLatin1String v)
    {
        return JS_NewStringLen(ctx, v.data(), v.size());
    }

    QUrl js_traits<QUrl>::unwrap(JSContext* ctx, JSValueConst v)
    {
        return js_traits<QString>::unwrap(ctx, v);
    }

    JSValue js_traits<QUrl>::wrap(JSContext* ctx, const QUrl& v)
    {
        return js_traits<QString>::wrap(ctx, v.toString());
    }

    QVariant js_traits<QVariant>::unwrap(JSContext* ctx, JSValueConst v)
    {
        if (JS_IsArray(v))
        {
            return detail::unwrap_free<QVariantList>(ctx, v);
        }
        else if (JS_IsObject(v))
        {
            return detail::unwrap_free<QVariantHash>(ctx, v);
        }
        else
        {
            switch (JS_VALUE_GET_TAG(v))
            {
            case JS_TAG_STRING:
            case JS_TAG_STRING_ROPE:
                return detail::unwrap_free<QByteArray>(ctx, v);
            case JS_TAG_INT:
                return detail::unwrap_free<int>(ctx, v);
            case JS_TAG_BOOL:
                return detail::unwrap_free<bool>(ctx, v);
            case JS_TAG_FLOAT64:
                return detail::unwrap_free<double>(ctx, v);
            default: return {};
            }
        }
    }

    JSValue js_traits<QVariant>::wrap(JSContext* ctx, const QVariant& v)
    {
    #define TYPE_CHECK(T) \
        if (v.metaType() == QMetaType::fromType<T>()) \
            return js_traits<T>::wrap(ctx, *static_cast<const T*>(v.data()))

        if (v.isValid())
        {
            TYPE_CHECK(double);
            TYPE_CHECK(float);
            TYPE_CHECK(int);
            TYPE_CHECK(qint64);
            TYPE_CHECK(quint64);
            TYPE_CHECK(quint32);
            TYPE_CHECK(bool);
            TYPE_CHECK(QByteArray);
            TYPE_CHECK(QString);
            TYPE_CHECK(QVariantList);
            TYPE_CHECK(QVariantHash);
        }

        return JS_NULL;
    }

    std::any js_traits<std::any>::unwrap(JSContext* ctx, JSValueConst v)
    {
        return value(ctx, JS_DupValue(ctx, v));
    }

    JSValue js_traits<std::any>::wrap(JSContext* ctx, const std::any& v)
    {
        if (!v.has_value())
            return JS_UNDEFINED;
        if (const auto* val = std::any_cast<value>(&v))
            return JS_DupValue(ctx, val->v);
        throw exception(ctx, JS_TYPE_ERROR, "Unexpected std::any value (holding %s)", v.type().name());
    }

    JSValue property_traits<QByteArray>::get(
        JSContext* ctx, JSValueConst this_obj, const QByteArray& name) noexcept
    {
        return JS_GetPropertyStr(ctx, this_obj, name.data());
    }

    void property_traits<QByteArray>::set(
        JSContext* ctx, JSValueConst this_obj, const QByteArray& name, JSValue val)
    {
        if (JS_SetPropertyStr(ctx, this_obj, name.data(), val) < 0)
            throw exception(ctx);
    }

    JSValue property_traits<QString>::get(
        JSContext* ctx, JSValueConst this_obj, const QString& name)
    {
        const QByteArray ba = name.toUtf8();
        return JS_GetPropertyStr(ctx, this_obj, ba.data());
    }

    void property_traits<QString>::set(
        JSContext* ctx, JSValueConst this_obj, const QString& name, JSValue val)
    {
        const QByteArray ba = name.toUtf8();
        if (JS_SetPropertyStr(ctx, this_obj, ba.data(), val) < 0)
            throw exception(ctx);
    }

    JSValue property_traits<QLatin1String>::get(
        JSContext* ctx, JSValueConst this_obj, QLatin1String name) noexcept
    {
        return JS_GetPropertyStr(ctx, this_obj, name.data());
    }

    void property_traits<QLatin1String>::set(
        JSContext* ctx, JSValueConst this_obj, QLatin1String name, JSValue val)
    {
        if (JS_SetPropertyStr(ctx, this_obj, name.data(), val) < 0)
            throw exception(ctx);
    }
}