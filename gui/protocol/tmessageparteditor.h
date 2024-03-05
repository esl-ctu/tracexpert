#ifndef TMESSAGEPARTEDITOR_H
#define TMESSAGEPARTEDITOR_H

#include <QWizard>
#include <QTableView>
#include <QListWidget>
#include <QLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QFormLayout>
#include <QHeaderView>
#include <QCheckBox>
#include <QComboBox>

#include "tmessagepart.h"

class TMessagePartEditorDetailsPage : public QWizardPage {
    Q_OBJECT

public:
    TMessagePartEditorDetailsPage(const TMessagePart & messagePart, const QList<TMessagePart> & messagePartList, QWidget * parent = nullptr);
    void updateDisplayedFields();
    bool validatePage() override;

private:
    QCheckBox * m_payloadCheckBox;
    QWidget   * m_radioWidget;
    QLineEdit * m_valueLineEdit;
    QRadioButton * m_hexRadioButton;
    QRadioButton * m_asciiRadioButton;
    QLineEdit * m_interpretedValueLineEdit;
    QCheckBox * m_staticLengthCheckBox;
    QLineEdit * m_lengthLineEdit;
    QComboBox * m_dynamicLengthComboBox;

    QFormLayout * m_formLayout;

    const QString m_originalName;
    const QList<TMessagePart> & m_messagePartList;
};

class TMessagePartEditor : public QWizard {
    Q_OBJECT

public:
    explicit TMessagePartEditor(const TMessagePart & messagePart, const QList<TMessagePart> & messagePartList, QWizard * parent = nullptr);
    TMessagePart messagePart();
};

#endif // TMESSAGEPARTEDITOR_H
