#include "newaddresstab.h"
#include "adddialog.h"

#include <QtWidgets>

NewAddressTab::NewAddressTab(QWidget* parent)
    : QWidget(parent)
{
    auto descriptionLabel = new QLabel(tr("There are currently no contacts in your address book. "
                                          "\nClick Add to add new contacts."));
    auto addButton = new QPushButton(tr("Add"));
    connect(addButton, &QAbstractButton::clicked, this, &NewAddressTab::addEntry);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addWidget(addButton, 0, Qt::AlignCenter);

    setLayout(mainLayout);
}

void NewAddressTab::addEntry()
{
    AddDialog addDialog;

    if(addDialog.exec()){
        emit sendDetails(addDialog.name(), addDialog.address());
    }
}
