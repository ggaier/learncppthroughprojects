#include "tablemodel.h"

//在方法中要尽量限制new关键字的使用. 在生命周期更长的情况的时候,
//或者申请内存过大的时候才考虑.
TableModel::TableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

TableModel::TableModel(const QVector<Contact>& contacts, QObject* parent)
    : QAbstractTableModel(parent)
    , contacts(contacts)
{
}

int TableModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : contacts.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= contacts.size() || index.row() < 0) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        const auto& contact = contacts.at(index.row());
        switch (index.column()) {
        case 0:
            return contact.name;
        case 1:
            return contact.address;
        default:
            break;
        }
    }
    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Address");
        }
    }
    return QVariant();
}

bool TableModel::insertRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        contacts.insert(position, { QString(), QString() });
    }
    endInsertRows();
    return true;
}

bool TableModel::removeRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        contacts.removeAt(position);
    }
    endRemoveRows();
    return true;
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();
        auto contact = contacts.value(row);
        switch (index.column()) {
        case 0:
            contact.name = value.toString();
            break;
        case 1:
            contact.address = value.toString();
            break;
        default:
            return false;
        }
        contacts.replace(row, contact);
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
        return true;
    }
    return false;
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

const QVector<Contact>& TableModel::getContacts() const
{
    return contacts;
}
