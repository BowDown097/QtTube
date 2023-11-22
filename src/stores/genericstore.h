#ifndef GENERICSTORE_H
#define GENERICSTORE_H
#include <QObject>

class GenericStore : public QObject
{
    Q_OBJECT
public:
    explicit GenericStore(const QString& filename, QObject* parent = nullptr);
    QString configPath() const { return m_configPath; }

    void clear();
    virtual void initialize() = 0;
    virtual void save() = 0;
private:
    QString m_configPath;
};

#endif // GENERICSTORE_H
