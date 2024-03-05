#ifndef TMESSAGEMODEL_H
#define TMESSAGEMODEL_H

#include <QApplication>
#include <QStyle>

#include "tabstracttablemodel.h"
#include "tmessage.h"

class TMessageSimpleContainer : public TAbstractTableModel<TMessage> {

public:
    explicit TMessageSimpleContainer(QList<TMessage> initialItems, QObject * parent = nullptr) : TAbstractTableModel<TMessage>(initialItems, parent) {
        for(TMessage & message : m_items) {
            message.validateMessage();
        }
    }

    int columnCount(const QModelIndex & parent) const {
        return 4;
    }

    QVariant data(const QModelIndex & index, int role) const {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_items.size())
            return QVariant();

        if (role == Qt::DisplayRole) {
            switch(index.column()) {
                case 1:
                    return m_items[index.row()].isResponse() ? QStringLiteral("Response") : QStringLiteral("Command");
                case 2:
                    return m_items[index.row()].getName();
                case 3:
                    return m_items[index.row()].getDescription();
            }
        }
        else if (role == Qt::DecorationRole) {
            if (index.column() == 0) {
                switch(m_items[index.row()].getState()) {
                    case TMessage::TState::TOk:
                        return QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
                    case TMessage::TState::TWarning:
                        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
                    case TMessage::TState::TInfo:
                        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
                    default:
                        return QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
                }
            }
        } else if (role == Qt::ToolTipRole) {
            if (index.column() == 0) {
                return m_items[index.row()].getStateMessage();
            }
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return QStringLiteral("Status");
            case 1:
                return QStringLiteral("Type");
            case 2:
                return QStringLiteral("Name");
            case 3:
                return QStringLiteral("Description");
            default:
                return QVariant();
            }
        }

        return QVariant();
    }
};

#endif // TMESSAGEMODEL_H
