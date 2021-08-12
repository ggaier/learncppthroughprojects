#ifndef ADDRESSWIDGET_H
#define ADDRESSWIDGET_H

#include "newaddresstab.h"
#include "tablemodel.h"

#include <QItemSelection>
#include <QTabWidget>

class AddressWidget : public QTabWidget {
    Q_OBJECT
public:
    AddressWidget(QWidget* parent = nullptr);
    void readFromFile(const QString& fileName);
    void writeToFile(const QString& fileName);

public slots:
    void showAddEntryDialog();
    void addEntry(const QString& name, const QString& address);
    void editEntry();
    void removeEntry();

signals:
    void selectionChanged(const QItemSelection& selected);

private:
    void setupTabs();

    TableModel* table;
    NewAddressTab* newAddressTab;
};

#endif // ADDRESSWIDGET_H
