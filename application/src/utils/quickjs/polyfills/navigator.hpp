#pragma once
#include <QStringList>
#include <quickjs++/quickjs_fwd.h>

class Navigator
{
public:
    const int deviceMemory;
    const int hardwareConcurrency;
    const QString language;
    const QStringList languages;

    Navigator();
    static void registerFor(qjs::context& ctx);
private:
    static QString getLanguage();
    static QStringList getLanguages();
    static quint64 getTotalRamBytes();
    static int getTotalRamGb();
};
