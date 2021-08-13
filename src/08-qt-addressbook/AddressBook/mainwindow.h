#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "addresswidget.h"
#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

    //slots的方法可以被任意的Component调用, 无论这个方法是什么
    //访问层级
private slots:
    void updateActions(const QItemSelection & selection);
    void openFile();
    void saveFile();

private:
    void createMenus();

    AddressWidget* addressWidget;
    QAction* editAct;
    QAction* removeAct;
};

#endif // MAINWINDOW_H
