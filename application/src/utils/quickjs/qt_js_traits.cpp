#include "qt_js_traits.h"

namespace qjs
{
    QByteArray qjs::js_traits<QByteArray>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = qjs::js_traits<std::string_view>::unwrap(ctx, v);
        return QByteArray(sv.data(), sv.size());
    }

    JSValue qjs::js_traits<QByteArray>::wrap(JSContext* ctx, const QByteArray& v)
    {
        return JS_NewStringLen(ctx, v.data(), v.size());
    }

    QString qjs::js_traits<QString>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = qjs::js_traits<std::string_view>::unwrap(ctx, v);
        return QString::fromUtf8(sv.data(), sv.size());
    }

    JSValue qjs::js_traits<QString>::wrap(JSContext* ctx, const QString& v)
    {
        const QByteArray ba = v.toUtf8();
        return JS_NewStringLen(ctx, ba.data(), ba.size());
    }

    QLatin1String qjs::js_traits<QLatin1String>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto sv = qjs::js_traits<std::string_view>::unwrap(ctx, v);
        return QLatin1String(sv.data(), sv.size());
    }

    JSValue qjs::js_traits<QLatin1String>::wrap(JSContext* ctx, QLatin1String v)
    {
        return JS_NewStringLen(ctx, v.data(), v.size());
    }

    QVariant qjs::js_traits<QVariant>::unwrap(JSContext* ctx, JSValueConst v)
    {
        auto convert = [&](JSValueConst v) -> QVariant {
            switch (JS_VALUE_GET_TAG(v))
            {
            case JS_TAG_STRING:
            case JS_TAG_STRING_ROPE:
                return qjs::detail::unwrap_free<QByteArray>(ctx, v);
            case JS_TAG_INT:
                return qjs::detail::unwrap_free<int>(ctx, v);
            case JS_TAG_BOOL:
                return qjs::detail::unwrap_free<bool>(ctx, v);
            case JS_TAG_FLOAT64:
                return qjs::detail::unwrap_free<double>(ctx, v);
            default: return {};
            }
        };

        if (JS_IsArray(v))
        {
            int64_t length;
            JS_GetLength(ctx, v, &length);

            QVariantList list;
            list.reserve(length);

            for (int64_t i = 0; i < length; ++i)
            {
                JSValue elem = JS_GetPropertyInt64(ctx, v, i);
                if (QVariant item = convert(elem); item.isValid())
                    list.append(std::move(item));
                else
                    JS_FreeValue(ctx, elem);
            }

            return list;
        }
        else
        {
            return convert(v);
        }
    }

    JSValue qjs::js_traits<QVariant>::wrap(JSContext* ctx, const QVariant& v)
    {
    #define TYPE_CHECK(T) \
        if (v.metaType() == QMetaType::fromType<T>()) \
            return qjs::js_traits<T>::wrap(ctx, *static_cast<const T*>(v.data()))

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
        }

        return JS_NULL;
    }

    std::any qjs::js_traits<std::any>::unwrap(JSContext* ctx, JSValueConst v) noexcept
    {
        return v;
    }

    JSValue qjs::js_traits<std::any>::wrap(JSContext* ctx, const std::any& v)
    {
        return std::any_cast<JSValue>(v);
    }

    std::monostate qjs::js_traits<std::monostate>::unwrap(JSContext* ctx, JSValueConst v) noexcept
    {
        return {};
    }

    JSValue qjs::js_traits<std::monostate>::wrap(JSContext* ctx, std::monostate v) noexcept
    {
        return JS_NULL;
    }

    JSValue qjs::property_traits<QByteArray>::get(
        JSContext* ctx, JSValueConst this_obj, const QByteArray& name) noexcept
    {
        return JS_GetPropertyStr(ctx, this_obj, name.data());
    }

    void qjs::property_traits<QByteArray>::set(
        JSContext* ctx, JSValueConst this_obj, const QByteArray& name, JSValue val)
    {
        if (JS_SetPropertyStr(ctx, this_obj, name.data(), val) < 0)
            throw exception(ctx);
    }

    JSValue qjs::property_traits<QString>::get(
        JSContext* ctx, JSValueConst this_obj, const QString& name)
    {
        const QByteArray ba = name.toUtf8();
        return JS_GetPropertyStr(ctx, this_obj, ba.data());
    }

    void qjs::property_traits<QString>::set(
        JSContext* ctx, JSValueConst this_obj, const QString& name, JSValue val)
    {
        const QByteArray ba = name.toUtf8();
        if (JS_SetPropertyStr(ctx, this_obj, ba.data(), val) < 0)
            throw exception(ctx);
    }

    JSValue qjs::property_traits<QLatin1String>::get(
        JSContext* ctx, JSValueConst this_obj, QLatin1String name) noexcept
    {
        return JS_GetPropertyStr(ctx, this_obj, name.data());
    }

    void qjs::property_traits<QLatin1String>::set(
        JSContext* ctx, JSValueConst this_obj, QLatin1String name, JSValue val)
    {
        if (JS_SetPropertyStr(ctx, this_obj, name.data(), val) < 0)
            throw exception(ctx);
    }
}