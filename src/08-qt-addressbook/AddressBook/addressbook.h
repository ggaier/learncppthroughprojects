#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <QWidget>

class AddressBook : public QWidget
{
    Q_OBJECT

public:
    AddressBook(QWidget *parent = nullptr);
    ~AddressBook();
};
#endif // ADDRESSBOOK_H
