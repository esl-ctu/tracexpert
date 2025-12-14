#include "tconfigparamwidget.h"

#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QPainter>
#include <limits>

#include "tcodeedit.h"
#include "ttimeedit.h"
#include "tcombobox.h"
#include "tfilenameedit.h"

TConfigParamWidget::TConfigParamWidget(const TConfigParam & param, QWidget * parent, bool readOnly)
    : QTreeWidget(parent), m_readOnly(readOnly)
{
    setParam(param);
}

TConfigParamWidget::~TConfigParamWidget()
{
}

TConfigParam TConfigParamWidget::param(bool * ok)
{
    bool i_ok = readParam(m_param, topLevelItem(0));
    if (ok) {
        *ok = i_ok;
    }
    return m_param;
}

void TConfigParamWidget::setParam(const TConfigParam & param)
{
    m_param = param;

    refreshParam();
}

void TConfigParamWidget::refreshParam()
{
    // prevent GUI from repainting any intermediate changes
    blockSignals(true);

    delete takeTopLevelItem(0);

    setColumnCount(3);

    QTreeWidgetItem * topItem = new QTreeWidgetItem(this);
    topItem->setExpanded(true);

    addParam(m_param, topItem);

    addTopLevelItem(topItem);

    setAlternatingRowColors(true);

    headerItem()->setText(0, tr("Parameter"));
    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(2, "");

    header()->setSectionResizeMode(0, QHeaderView::Interactive);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    header()->setSectionResizeMode(2, QHeaderView::Fixed);
    header()->setStretchLastSection(false);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);

    // allow GUI to repaint again
    blockSignals(false);
}

bool TConfigParamWidget::readParam(TConfigParam & param, QTreeWidgetItem * parent)
{
    bool ok = checkInput(param, parent);

    QList<TConfigParam> & subParams = param.getSubParams();

    for (int i = 0; i < subParams.size(); i++) {
        QTreeWidgetItem * item = parent->child(i);
        TConfigParam & subParam = subParams[i];

        bool iOk = readParam(subParam, item);
        ok = ok && iOk;
    }

    return ok;
}

void TConfigParamWidget::addParam(TConfigParam & param, QTreeWidgetItem * parent)
{
    drawLabel(param, parent);

    drawState(param, parent);

    drawInput(param, parent);

    QList<TConfigParam> & subParams = param.getSubParams();

    for (int i = 0; i < subParams.size(); i++) {
        QTreeWidgetItem * item = new QTreeWidgetItem(parent);
        item->setExpanded(true);

        TConfigParam & subParam = subParams[i];
        addParam(subParam, item);
        parent->addChild(item);
    }
}

void TConfigParamWidget::drawLabel(const TConfigParam & param, QTreeWidgetItem *parent)
{
    parent->setText(0, param.getName());
    parent->setToolTip(0, param.getHint());
}

void TConfigParamWidget::drawState(const TConfigParam & param, QTreeWidgetItem * parent)
{
    TConfigParam::TState state = param.getState();
    if (state != TConfigParam::TState::TOk) {
        parent->setToolTip(2, QString("<span style=\"color:%1;font-weight:bold;\">%2</span>").arg(stateColor(state), tr(qPrintable(param.getStateMessage()))));
    }
    else {
        parent->setToolTip(2, QString());
    }
    parent->setIcon(2, stateIcon(state));
}

void TConfigParamWidget::drawInput(const TConfigParam & param, QTreeWidgetItem * parent)
{
    auto lightenIcon = [&, parent]
    {
        if (!parent->icon(2).isNull()) {
            parent->setIcon(2, stateIcon(param.getState(), true));
        }

        emit inputValueChanged();
    };

    TConfigParam::TType type = param.getType();
    if(param.isReadonly() || m_readOnly) {
        // plain text to optimize performance
        parent->setText(1, param.getValue());
    }
    else if (
        type == TConfigParam::TType::TBool ||
        type == TConfigParam::TType::TEnum ||
        type == TConfigParam::TType::TTime ||
        type == TConfigParam::TType::TDirectoryName ||
        type == TConfigParam::TType::TFileName ||
        type == TConfigParam::TType::TString ||
        type == TConfigParam::TType::TByteArray ||
        type == TConfigParam::TType::TReal ||
        type == TConfigParam::TType::TInt ||
        type == TConfigParam::TType::TLongLong ||
        type == TConfigParam::TType::TShort ||
        type == TConfigParam::TType::TUInt ||
        type == TConfigParam::TType::TULongLong ||
        type == TConfigParam::TType::TUShort||
        type == TConfigParam::TType::TCode
    ) {
        QWidget * input;
        if (type == TConfigParam::TType::TBool || type == TConfigParam::TType::TEnum) {
            QComboBox * comboBox = new TComboBox(this);
            if (type == TConfigParam::TType::TBool) {
                comboBox->addItem(tr("True"));
                comboBox->addItem(tr("False"));
                comboBox->setCurrentIndex(param.getValue() == QString("true") ? 0 : 1);
            }
            else if (type == TConfigParam::TType::TEnum) {
                const QStringList & enumValues = param.getEnumValues();
                for (int i = 0; i < enumValues.size(); i++) {
                    comboBox->addItem(tr(qPrintable(enumValues[i])));
                }
                comboBox->setCurrentIndex(qMax(param.getEnumValues().indexOf(param.getValue()), 0));
            }
            connect(comboBox, &QComboBox::currentIndexChanged, this, lightenIcon);
            input = comboBox;
        }
        else if (type == TConfigParam::TType::TTime) {
            TTimeEdit * edit = new TTimeEdit(this);
            edit->setText(param.getValue());
            connect(edit, &TTimeEdit::textEdited, this, lightenIcon);
            input = edit;
        }
        else if (type == TConfigParam::TType::TFileName) {
            TFileNameEdit * edit = new TFileNameEdit(this);
            edit->setText(param.getValue());
            connect(edit, &QLineEdit::textEdited, this, lightenIcon);
            input = edit;
        }
        else if (type == TConfigParam::TType::TDirectoryName) {
            TFileNameEdit * edit = new TFileNameEdit(QFileDialog::FileMode::Directory, this);
            edit->setText(param.getValue());
            connect(edit, &QLineEdit::textEdited, this, lightenIcon);
            input = edit;
        }
        else if (type == TConfigParam::TType::TCode) {
            TCodeEdit * edit = new TCodeEdit(this);
            edit->setPlainText(param.getValue());
            connect(edit, &TCodeEdit::textChanged, this, lightenIcon);
            input = edit;
        }
        else {
            QLineEdit * edit = new QLineEdit(param.getValue(), this);
            QValidator * validator = nullptr;
            if (type == TConfigParam::TType::TReal) {
                validator = new QDoubleValidator(edit);
                edit->setValidator(validator);
                edit->setText(edit->validator()->locale().toString(param.getValue().toDouble()));
            }
            else if (type == TConfigParam::TType::TInt || type == TConfigParam::TType::TLongLong || type == TConfigParam::TType::TShort || type == TConfigParam::TType::TUInt || type == TConfigParam::TType::TULongLong ||type == TConfigParam::TType::TUShort) {
                int digits = 0;
                bool isSigned = false;
                if (type == TConfigParam::TType::TUShort) {
                    digits = QString::number(std::numeric_limits<quint16>::max()).length();
                }
                else if (type == TConfigParam::TType::TShort) {
                    digits = QString::number(std::numeric_limits<qint16>::max()).length();
                    isSigned = true;
                }
                else if (type == TConfigParam::TType::TUInt) {
                    digits = QString::number(std::numeric_limits<quint32>::max()).length();
                }
                else if (type == TConfigParam::TType::TInt) {
                    digits = QString::number(std::numeric_limits<qint32>::max()).length();
                    isSigned = true;
                }
                else if (type == TConfigParam::TType::TULongLong) {
                    digits = QString::number(std::numeric_limits<quint64>::max()).length();
                }
                else if (type == TConfigParam::TType::TLongLong) {
                    digits = QString::number(std::numeric_limits<qint64>::max()).length();
                    isSigned = true;
                }
                validator = new QRegularExpressionValidator(QRegularExpression(QString("[+%1]{0,1}0*\\d{1,%2} *").arg(isSigned?"-":"", QString::number(digits))), edit);
                edit->setValidator(validator);
                edit->setText(param.getValue());
            }
            else {
                edit->setText(param.getValue());
            }
            connect(edit, &QLineEdit::textEdited, this, lightenIcon);
            input = edit;
        }
        input->setToolTip(param.getHint());
        input->setDisabled(param.isReadonly() || m_readOnly);
        setItemWidget(parent, 1, input);
    }
}

bool TConfigParamWidget::checkInput(TConfigParam & param, QTreeWidgetItem * parent)
{
    if (param.isReadonly()) {
        return true;
    }

    bool ok = true;

    TConfigParam::TType type = param.getType();
    if (
        type == TConfigParam::TType::TBool ||
        type == TConfigParam::TType::TEnum ||
        type == TConfigParam::TType::TTime ||
        type == TConfigParam::TType::TFileName ||
        type == TConfigParam::TType::TDirectoryName ||
        type == TConfigParam::TType::TString ||
        type == TConfigParam::TType::TByteArray ||
        type == TConfigParam::TType::TReal ||
        type == TConfigParam::TType::TInt ||
        type == TConfigParam::TType::TLongLong ||
        type == TConfigParam::TType::TShort ||
        type == TConfigParam::TType::TUInt ||
        type == TConfigParam::TType::TULongLong ||
        type == TConfigParam::TType::TUShort ||
        type == TConfigParam::TType::TCode
    ) {
        QWidget * input = itemWidget(parent, 1);
        QString message;
        if (type == TConfigParam::TType::TBool || type == TConfigParam::TType::TEnum) {
            QComboBox * comboBox = static_cast<QComboBox *>(input);
            if (type == TConfigParam::TType::TBool) {
                param.setBool(!comboBox->currentIndex());
            }
            else if (type == TConfigParam::TType::TEnum) {
                int index = comboBox->currentIndex();
                if(index > -1 && index < param.getEnumValues().count()) {
                    param.setValue(param.getEnumValues()[comboBox->currentIndex()]);
                }
                else {
                    message = tr("Selected value is invalid!");
                    ok = false;
                }
            }
        }
        else if (type == TConfigParam::TType::TTime) {
            TTimeEdit * timeEdit = static_cast<TTimeEdit *>(input);
            param.setValue(timeEdit->text(), &ok);
            message = tr("Time value expected!");
        }
        else if (type == TConfigParam::TType::TFileName || type == TConfigParam::TType::TDirectoryName) {
            TFileNameEdit * fileEdit = static_cast<TFileNameEdit *>(input);
            param.setValue(fileEdit->text(), &ok);
            message = tr("Name of file expected!");
        }
        else if (type == TConfigParam::TType::TCode) {
            TCodeEdit * codeEdit = static_cast<TCodeEdit *>(input);
            param.setValue(codeEdit->toPlainText(), &ok);
            message = tr("Huh?!");
        }
        else {
            QLineEdit * edit = static_cast<QLineEdit *>(input);
            if (type == TConfigParam::TType::TReal) {
                message = tr("Floating point value expected!");
                bool iOk;
                qreal value = edit->validator()->locale().toDouble(edit->text(), &iOk);
                param.setValue(QString::number(value), &ok);
                ok = ok && iOk;
            }
            else if (type == TConfigParam::TType::TInt || type == TConfigParam::TType::TLongLong || type == TConfigParam::TType::TShort || type == TConfigParam::TType::TUInt || type == TConfigParam::TType::TULongLong ||type == TConfigParam::TType::TUShort) {
                QString top;
                QString bottom;
                if (type == TConfigParam::TType::TUShort) {
                    top = QString::number(std::numeric_limits<quint16>::max());
                    bottom = QString::number(std::numeric_limits<quint16>::min());
                }
                else if (type == TConfigParam::TType::TShort) {
                    top = QString::number(std::numeric_limits<qint16>::max());
                    bottom = QString::number(std::numeric_limits<qint16>::min());
                }
                else if (type == TConfigParam::TType::TUInt) {
                    top = QString::number(std::numeric_limits<quint32>::max());
                    bottom = QString::number(std::numeric_limits<quint32>::min());
                }
                else if (type == TConfigParam::TType::TInt) {
                    top = QString::number(std::numeric_limits<qint32>::max());
                    bottom = QString::number(std::numeric_limits<qint32>::min());
                }
                else if (type == TConfigParam::TType::TULongLong) {
                    top = QString::number(std::numeric_limits<quint64>::max());
                    bottom = QString::number(std::numeric_limits<quint64>::min());
                }
                else if (type == TConfigParam::TType::TLongLong) {
                    top = QString::number(std::numeric_limits<qint64>::max());
                    bottom = QString::number(std::numeric_limits<qint64>::min());
                }
                message = tr("Integer value from %1 to %2 expected!").arg(bottom, top);
                param.setValue(edit->text(), &ok);
            }
            else {
                param.setValue(edit->text(), &ok);
            }
        }
        if (!ok) {
            param.setState(TConfigParam::TState::TError, message);
        }
        if (ok && param.getStateMessage() == message) {
            param.setState(TConfigParam::TState::TOk, message);
        }
        drawState(param, parent);
    }

    return ok;
}

QIcon TConfigParamWidget::stateIcon(TConfigParam::TState state, bool isLightened) {
    QIcon icon;
    if (state == TConfigParam::TState::TError) {
        icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    }
    else if (state == TConfigParam::TState::TWarning) {
        icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    }
    else if (state == TConfigParam::TState::TInfo) {
        icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    }

    if (isLightened) {
        QPixmap iconPixmap = icon.pixmap(icon.actualSize(QSize(128, 128)));
        QPixmap newIconPixmap(icon.actualSize(QSize(128, 128)));
        newIconPixmap.fill(Qt::transparent);
        QPainter painter(&newIconPixmap);
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, iconPixmap);
        painter.end();
        icon = newIconPixmap;
    }

    return icon;
}

QString TConfigParamWidget::stateColor(TConfigParam::TState state)
{
    if (state == TConfigParam::TState::TError) {
        return "DarkRed";
    }
    else if (state == TConfigParam::TState::TWarning) {
        return "DarkOrange";
    }
    else if (state == TConfigParam::TState::TInfo) {
        return "DarkBlue";
    }

    return QString();
}
