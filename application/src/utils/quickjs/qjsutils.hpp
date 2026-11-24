#pragma once
#include "qt_js_traits.hpp"
#include <quickjs++/value.h>

namespace QJSUtils
{
    namespace detail
    {
        template<typename T>
        T empty()
        {
            if constexpr (std::same_as<T, qjs::value>)
                return qjs::value(JS_UNDEFINED);
            else
                return T{};
        }

        template<typename T, typename Default>
        T uop(JSContext* ctx, JSValueConst this_obj, const char* prop, Default&& default_value)
        {
            if (JS_IsUndefined(this_obj) || JS_IsNull(this_obj))
                return std::forward<Default>(default_value)();

            JSValue val = JS_GetPropertyStr(ctx, this_obj, prop);
            if (JS_IsUndefined(val) || JS_IsNull(val))
                return std::forward<Default>(default_value)();

            return qjs::detail::unwrap_free<T>(ctx, val);
        }
    }

    inline QString generateErrorString(const qjs::value& error, const std::source_location* loc = nullptr)
    {
        if (!error.is_error())
            return error.as<QString>();

        QString stack = error["stack"].as<QString>();
        if (stack.isEmpty() && loc)
        {
            stack = QStringLiteral("%1:%2:%3 in %4")
                .arg(loc->file_name())
                .arg(loc->line())
                .arg(loc->column())
                .arg(loc->function_name());
        }

        return QStringLiteral("%1: %2\n%3").arg(
            error["name"].as<QString>(), error["message"].as<QString>(), stack);
    }

    inline QString generateErrorString(const qjs::exception& ex)
    {
        return generateErrorString(ex.get_value(), &ex.location());
    }

    template<typename T>
    inline T getStringStrict(
        const qjs::value& val,
        const char* error = "Expected string for value",
        const char* defaultValue = nullptr)
    {
        if (val.is_string())
            return val.as<T>();
        else if (defaultValue)
            return T(defaultValue);
        else
            throw qjs::exception(val.ctx, JS_TYPE_ERROR, error);
    }

    template<typename T>
    T unwrapObjectProperty(JSContext* ctx, JSValueConst this_obj, const char* prop)
    { return detail::uop<T>(ctx, this_obj, prop, detail::empty<T>); }
    template<typename T>
    T unwrapObjectProperty(JSContext* ctx, JSValueConst this_obj, const char* prop, T default_value)
    { return detail::uop<T>(ctx, this_obj, prop, [&] { return std::move(default_value); }); }
    template<typename T>
    T unwrapObjectProperty(const qjs::value& this_obj, const char* prop)
    { return detail::uop<T>(this_obj.ctx, this_obj.v, prop, detail::empty<T>); }
    template<typename T>
    T unwrapObjectProperty(const qjs::value& this_obj, const char* prop, T default_value)
    { return detail::uop<T>(this_obj.ctx, this_obj.v, prop, [&] { return std::move(default_value); }); }
}