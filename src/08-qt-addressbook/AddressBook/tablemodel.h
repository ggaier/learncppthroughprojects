#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>


struct Contact
{
    QString name;
    QString address;

    bool operator==(const Contact &other) const
    {
        return name == other.name && address == other.address;
    }
};

inline QDataStream &operator<<(QDataStream &stream, const Contact &contact)
{
    return stream << contact.name << contact.address;
}

inline QDataStream &operator>>(QDataStream &stream, Contact &contact)
{
    return stream >> contact.name >> contact.address;
}

//QAbstractTableModel: 一套保准的API, 用来提供二维数组的数据.
class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TableModel(QObject *parent = nullptr);
    TableModel(const QVector<Contact> &contacts, QObject *parent = nullptr);

    //override表示继承父类的一个虚函数.
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    const QVector<Contact> &getContacts() const;

private:
    QVector<Contact> contacts;
};


#endif // TABLEMODEL_H
