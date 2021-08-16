#include "mainwindow.h"
//如果在header file中include <QtWidgets>, 会造成性能问题
//但是使用class forward declaration则不会有这个问题.
#include <QtWidgets>

MainWindow::MainWindow()
    : textEdit(new QPlainTextEdit)
{

    setCentralWidget(textEdit);

    createActions();
    createStatusBar();

    readSettings();
    connect(textEdit->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::documentWasModified);

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    //当应用提交所有的数据的时候, 绑定slot.
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &MainWindow::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}









































