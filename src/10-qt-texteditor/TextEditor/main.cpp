#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    //初始化资源文件
    Q_INIT_RESOURCE(application);

#ifdef Q_OS_ANDROID
    QApplication::setAttributes(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationName("Application Example");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    //利用command line parser来处理文件参数被传给应用的情况.
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(a);

    MainWindow w;
    const QStringList args = parser.positionalArguments();
    if(!args.isEmpty()){
        w.loadFile(args.first());
    }
    w.show();
    return a.exec();
}
