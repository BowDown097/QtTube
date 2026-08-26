#pragma once
#include "qt_js_traits.hpp"
#include <quickjs++/value.h>

namespace QJSUtils
{
    inline QString generateErrorString(const qjs::value& error)
    {
        if (!error.is_error())
            return error.as<QString>();

        return QStringLiteral("%1: %2\n%3").arg(
            error["name"].as<QString>(),
            error["message"].as<QString>(),
            error["stack"].as<QString>());
    }

    inline QString generateErrorString(const qjs::exception& ex)
    {
        qjs::value error = ex.get_value();
        if (!error.is_error())
            return error.as<QString>();

        QString stack = error["stack"].as<QString>();
        if (stack.isEmpty())
        {
            stack = QStringLiteral("%1:%2:%3 in %4")
                .arg(ex.location().file_name())
                .arg(ex.location().line())
                .arg(ex.location().column())
                .arg(ex.location().function_name());
        }

        return QStringLiteral("%1: %2\n%3").arg(
            error["name"].as<QString>(),
            error["message"].as<QString>(),
            stack);
    }

    template<typename T, bool ReturnDefault = false>
    T getStringStrict(const qjs::value& val, const char* error = "Expected string for value", const char* defaultValue = "")
    {
        if (val.is_string())
            return val.as<T>();
        else if constexpr (ReturnDefault)
            return T(defaultValue);
        else
            throw qjs::exception(val.ctx, JS_TYPE_ERROR, error);
    }

    template<typename T>
    auto unwrapObjectProperty(JSContext* ctx, JSValueConst this_obj, const char* prop)
    {
        JSValue val = JS_GetPropertyStr(ctx, this_obj, prop);
        if constexpr (std::is_same_v<T, std::any>)
            return qjs::value(ctx, std::move(val));
        else if (!JS_IsUndefined(val))
            return qjs::detail::unwrap_free<T>(ctx, val);
        else
            return T{};
    }
}