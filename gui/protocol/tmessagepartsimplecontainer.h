#ifndef TMESSAGEPARTMODEL_H
#define TMESSAGEPARTMODEL_H

#include "qapplication.h"
#include "qstyle.h"
#include "tabstracttablemodel.h"
#include "tmessagepart.h"

class TMessagePartSimpleContainer : public TAbstractTableModel<TMessagePart> {

public:
    explicit TMessagePartSimpleContainer(QList<TMessagePart> initialItems, QObject *parent = nullptr) : TAbstractTableModel<TMessagePart>(initialItems, parent) { }

    int columnCount(const QModelIndex & parent) const {
        return 5;
    }

    QVariant data(const QModelIndex & index, int role) const {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_items.size())
            return QVariant();

        if (role == Qt::DisplayRole) {
            switch(index.column()) {
            case 1:
                switch(m_items[index.row()].getType()) {
                case TMessagePart::TType::TBool:
                    return QStringLiteral("Boolean");
                case TMessagePart::TType::TByte:
                case TMessagePart::TType::TUChar:
                    return QStringLiteral("Byte");
                case TMessagePart::TType::TChar:
                    return QStringLiteral("Character");
                case TMessagePart::TType::TByteArray:
                    return QStringLiteral("Byte array");
                case TMessagePart::TType::TString:
                    return QStringLiteral("String");
                case TMessagePart::TType::TShort:
                    return QStringLiteral("Short");
                case TMessagePart::TType::TUShort:
                    return QStringLiteral("Unsigned short");
                case TMessagePart::TType::TInt:
                    return QStringLiteral("Integer");
                case TMessagePart::TType::TUInt:
                    return QStringLiteral("Unsigned integer");
                case TMessagePart::TType::TLongLong:
                    return QStringLiteral("Long long");
                case TMessagePart::TType::TULongLong:
                    return QStringLiteral("Unsigned long long");
                case TMessagePart::TType::TReal:
                    return QStringLiteral("Real");
                default:
                    return QStringLiteral("Unknown type");
                }
            case 2:
                return m_items[index.row()].getName();
            case 3:
                return m_items[index.row()].getDescription();
            case 4:
                if(m_items[index.row()].hasStaticLength()) {
                    return QString::number(m_items[index.row()].getLength()) + QStringLiteral(" bytes");
                } else {
                    qsizetype refIndex = m_items[index.row()].getLength();

                    if(refIndex >= 0 && refIndex < m_items.size()) {
                    return QStringLiteral("dynamic, based on \"") + m_items[m_items[index.row()].getLength()].getName() + QStringLiteral("\"");
                    }

                    return QStringLiteral("dynamic, reference lost! ");
                };
            }
        }
        else if (role == Qt::DecorationRole) {
            if (index.column() == 0) {
                switch(m_items[index.row()].getState()) {
                    case TMessagePart::TState::TOk:
                        return QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
                    case TMessagePart::TState::TWarning:
                        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
                    case TMessagePart::TState::TInfo:
                        return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
                    case TMessagePart::TState::TUnevaluated:
                        return QApplication::style()->standardIcon(QStyle::SP_TitleBarContextHelpButton);
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
            case 4:
                return QStringLiteral("Length");
            default:
                return QVariant();
            }
        }

        return QVariant();
    }
};

#endif // TMESSAGEPARTMODEL_H
