// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Adam Švehla (initial author)

#ifndef TMESSAGECONTAINER_H
#define TMESSAGECONTAINER_H

#include <QApplication>
#include <QStyle>

#include "tabstracttablemodel.h"
#include "tmessage.h"

class TMessageContainer : public TAbstractTableModel<TMessage> {

public:
    explicit TMessageContainer(QList<TMessage> initialItems, QObject * parent = nullptr) : TAbstractTableModel<TMessage>(initialItems, parent) {
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

#endif // TMESSAGECONTAINER_H
