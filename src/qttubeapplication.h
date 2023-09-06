#ifndef QTTUBEAPPLICATION_H
#define QTTUBEAPPLICATION_H
#include <QApplication>

class QtTubeApplication final : public QApplication
{
public:
    QtTubeApplication(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* receiver, QEvent* event) override;
};

#endif // QTTUBEAPPLICATION_H
