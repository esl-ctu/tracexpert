#include "tpluginunitcontainer.h"

#include <QApplication>
#include <QStyle>

TPluginUnitContainer::TPluginUnitContainer(QObject * parent)
    : QAbstractTableModel(parent)
{

}

int TPluginUnitContainer::rowCount(const QModelIndex & parent) const
{
    return count();
}

int TPluginUnitContainer::columnCount(const QModelIndex & parent) const
{
    return 3;
}

QVariant TPluginUnitContainer::data(const QModelIndex & index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (index.column() == 1) {
            return at(index.row())->name();
        }
        else if (index.column() == 2) {
            return at(index.row())->info();
        }
    }
    else if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return (at(index.row())->isInit() ? QApplication::style()->standardIcon(QStyle::SP_DialogYesButton) : QApplication::style()->standardIcon(QStyle::SP_DialogNoButton));
        }
    }
    return QVariant();
}

QVariant TPluginUnitContainer::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == 0) {
                return tr("Status");
            }
            else if (section == 1) {
                return tr("Name");
            }
            else if (section == 2) {
                return tr("Description");
            }
        }
    }

    return QVariant();
}

int TPluginUnitContainer::childrenCount() const
{
    return count();
}

TProjectItem * TPluginUnitContainer::child(int row) const
{
    return at(row);
}

QVariant TPluginUnitContainer::status() const
{
    return QVariant();
}
