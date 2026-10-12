#pragma once
#include <quickjs++/context.h>

struct TextEncoder
{
    JSValue encode(JSContext* ctx, JSValueConst string);
    JSValue encodeInto(JSContext* ctx, JSValueConst string, JSValue uint8Array);
    static void registerFor(qjs::context& ctx);
};
