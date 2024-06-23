#pragma once
#include <QObject>

class GenericStore : public QObject
{
    Q_OBJECT
public:
    explicit GenericStore(const QString& filename, QObject* parent = nullptr);
    const QString& configPath() const { return m_configPath; }

    void clear();
    virtual void initialize() = 0;
    virtual void save() = 0;
private:
    QString m_configPath;
};
