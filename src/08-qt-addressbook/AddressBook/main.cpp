#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
#ifdef Q_OS_ANDROID
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
