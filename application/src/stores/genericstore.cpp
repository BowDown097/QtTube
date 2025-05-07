#include "genericstore.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

GenericStore::GenericStore(const QString& filename, QObject* parent)
    : m_configPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + filename),
      QObject(parent) {}

void GenericStore::clear()
{
    QSettings(m_configPath, QSettings::IniFormat).clear();
}
