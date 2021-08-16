#include "mainwindow.h"

#include <QAction>
#include <QMenuBar>
#include <QFileDialog>

MainWindow::MainWindow() : QMainWindow(),
      addressWidget(new AddressWidget)
{
    setCentralWidget(addressWidget);
    createMenus();
    setWindowTitle(tr("Address Book"));
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openAct = new QAction(tr("&Open..."), this);
    fileMenu->addAction(openAct);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    QMenu* toolMenu = menuBar()->addMenu(tr("&Tools"));

    QAction* addAct = new QAction("&Add Entry", this);
    addAct->setEnabled(true);
    toolMenu->addAction(addAct);
    connect(addAct, &QAction::triggered, addressWidget, &AddressWidget::showAddEntryDialog);


    editAct = new QAction(tr("&Edit Entry..."), this);
    editAct->setEnabled(false);
    toolMenu->addAction(editAct);
    connect(editAct, &QAction::triggered, addressWidget, &AddressWidget::editEntry);
    toolMenu->addSeparator();

    removeAct = new QAction(tr("&Remove Entry"), this);
    removeAct->setEnabled(false);
    toolMenu->addAction(removeAct);
    connect(removeAct, &QAction::triggered, addressWidget, &AddressWidget::removeEntry);

    connect(addressWidget, &AddressWidget::selectionChanged, this, &MainWindow::updateActions);
}


void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty()){
        addressWidget->readFromFile(fileName);
    }
}


void MainWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if(!fileName.isEmpty()){
        addressWidget->writeToFile(fileName);
    }
}

void MainWindow::updateActions(const QItemSelection & selection)
{
    QModelIndexList indexes = selection.indexes();
    if(!indexes.isEmpty()){
        removeAct->setEnabled(true);
        editAct->setEnabled(true);
    } else {
        removeAct->setEnabled(false);
        editAct->setEnabled(false);
    }
}

























