#pragma once
#include <QString>

class QFileInfo;

namespace OSUtils
{
    QString getFullPath(const QFileInfo& fileInfo);
    void suspendIdleSleep(bool suspend);
}
