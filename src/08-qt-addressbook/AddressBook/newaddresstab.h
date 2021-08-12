#ifndef NEWADDRESSTAB_H
#define NEWADDRESSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class VBoxLayout;
QT_END_NAMESPACE

class NewAddressTab : public QWidget {

    Q_OBJECT

public:
    NewAddressTab(QWidget* parent = nullptr);

public slots:
    void addEntry();

signals:
    void sendDetails(const QString& name, const QString& address);
};

#endif // NEWADDRESSTAB_H
