#include "navigator.h"
#include "utils/quickjs/qt_js_traits.h"
#include <QLocale>
#include <QThread>
#include <quickjs++/context.h>

#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(Q_OS_UNIX)
#include <sys/sysinfo.h>
#endif

bool doesScriptMatter(QLocale::Language lang)
{
    return lang == QLocale::Serbian || lang == QLocale::Chinese ||
           lang == QLocale::Kazakh || lang == QLocale::Uyghur;
}

Navigator::Navigator()
    : deviceMemory(getTotalRamGb()),
      hardwareConcurrency(QThread::idealThreadCount()),
      language(getLanguage()),
      languages(getLanguages()) {}

QString Navigator::getLanguage()
{
    QLocale loc = QLocale::system();
    QString languageCode = QLocale::languageToCode(loc.language(), QLocale::AnyLanguageCode);
    QLocale::Territory territory = loc.territory();
    QLocale::Language language = loc.language();

    if (language == QLocale::AnyLanguage || territory == QLocale::AnyTerritory)
        return languageCode;

    QString territoryCode = QLocale::territoryToCode(territory);
    if (doesScriptMatter(language))
        return languageCode + u'-' + QLocale::scriptToCode(loc.script()) + u'-' + territoryCode;
    else
        return languageCode + u'-' + territoryCode;
}

QStringList Navigator::getLanguages()
{
    QStringList result;

    const QStringList langs = QLocale::system().uiLanguages();
    for (const QString& langName : langs)
    {
        QStringList parts = langName.split(u'-', Qt::KeepEmptyParts);
        if (parts.begin() == parts.end())
            continue;

        QLocale::Language lang = QLocale::codeToLanguage(parts[0]);
        if (doesScriptMatter(lang))
        {
            result.append(langName);
        }
        else
        {
            if (parts.size() > 1 && QLocale::codeToTerritory(parts[1]) == QLocale::AnyTerritory)
                parts.removeAt(1);
            result.append(parts.join(u'-'));
        }
    }

    result.removeDuplicates();
    return result;
}

quint64 Navigator::getTotalRamBytes()
{
#if defined(Q_OS_WIN)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return static_cast<std::uint64_t>(status.ullTotalPhys);
    }
    return 0;
#elif defined(__APPLE__)
    quint64 size{};
    size_t len = sizeof(size);
    sysctlbyname("hw.memsize", &size, &len, nullptr, 0);
    return size;
#elif defined(Q_OS_UNIX)
    struct sysinfo info;
    sysinfo(&info);
    return static_cast<quint64>(info.totalram) * static_cast<quint64>(info.mem_unit);
#else
    return 0;
#endif
}

int Navigator::getTotalRamGb()
{
    double gb = getTotalRamBytes() / (1024.0 * 1024.0 * 1024.0);
    double exp = std::round(std::log2(gb));
    return (int)std::pow(2, exp);
}

void Navigator::registerFor(qjs::context& ctx)
{
    qjs::class_registrar<Navigator>(ctx)
        .member<&Navigator::deviceMemory>()
        .member<&Navigator::hardwareConcurrency>()
        .member<&Navigator::language>()
        .member<&Navigator::languages>();

    ctx.global()["navigator"] = std::make_shared<Navigator>();
}