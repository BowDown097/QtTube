#pragma once
#include "qt_js_traits.hpp"
#include <quickjs++/value.h>
#include <QJsonArray>
#include <QJsonObject>

// "need" macro because QObject does not work with template classes
// could write the classes out manually but ehhhh
#define MAKE_VALUE_WRAPPER(T, U, method) \
class T##Wrapper : public QObject \
{ \
    Q_OBJECT \
    Q_PROPERTY(T value MEMBER value CONSTANT) \
public: \
    T value; \
    explicit T##Wrapper(const qjs::value& jsval) : value(T::method(jsval.as<U>())) {} \
}

MAKE_VALUE_WRAPPER(QJsonArray,  QVariantList, fromVariantList);
MAKE_VALUE_WRAPPER(QJsonObject, QVariantHash, fromVariantHash);
MAKE_VALUE_WRAPPER(QJsonValue,  QVariant,     fromVariant);