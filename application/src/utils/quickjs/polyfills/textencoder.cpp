#include "textencoder.hpp"
#include <QStringEncoder>

JSValue TextEncoder::encode(JSContext* ctx, JSValueConst string)
{
    auto jsview = qjs::js_traits<std::u16string_view>::unwrap(ctx, string);
    QStringView view(reinterpret_cast<const QChar*>(jsview.data()), jsview.size());
    const QByteArray utf8 = view.toUtf8();
    return JS_NewUint8ArrayCopy(ctx, reinterpret_cast<const uint8_t*>(utf8.data()), utf8.size());
}

JSValue TextEncoder::encodeInto(JSContext* ctx, JSValueConst string, JSValue uint8Array)
{
    if (JS_GetTypedArrayType(uint8Array) != JS_TYPED_ARRAY_UINT8)
        return JS_ThrowTypeError(ctx, "destination must be a Uint8Array");

    auto jsview = qjs::js_traits<std::u16string_view>::unwrap(ctx, string);
    auto qstr = reinterpret_cast<const QChar*>(jsview.data());

    size_t offset, destLength, bytesPerElement;
    qjs::value buffer(ctx, JS_GetTypedArrayBuffer(
        ctx, uint8Array, &offset, &destLength, &bytesPerElement));

    size_t bufferLength;
    auto data = reinterpret_cast<char*>(JS_GetArrayBuffer(ctx, &bufferLength, buffer.v));
    if (!data || offset > bufferLength || destLength > bufferLength - offset)
        return JS_ThrowTypeError(ctx, "invalid destination buffer");

    size_t read = 0, written = 0;
    QStringEncoder encoder(QStringEncoder::Utf8);

    while (read < jsview.size())
    {
        bool isSurrogatePair =
            read < jsview.size() - 1 &&
            qstr[read].isHighSurrogate() &&
            qstr[read + 1].isLowSurrogate();
        size_t units = isSurrogatePair ? 2 : 1;

        if (encoder.requiredSpace(units) > destLength - written)
            break;

        char* out = data + offset + written;
        char* end = encoder.appendToBuffer(out, QStringView(qstr + read, units));

        written += end - out;
        read += units;
    }

    qjs::value result(ctx, JS_NewObject(ctx));
    result["read"] = read;
    result["written"] = written;
    return result.release();
}

void TextEncoder::registerFor(qjs::context& ctx)
{
    qjs::class_registrar<TextEncoder>(ctx)
        .constructor<>()
        .member<&TextEncoder::encode>()
        .member<&TextEncoder::encodeInto>();
}