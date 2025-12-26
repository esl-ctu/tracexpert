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
// Vojtěch Miškovský (initial author)

#ifndef TMESSAGEFORMWIDGET_H
#define TMESSAGEFORMWIDGET_H


#include <QWidget>
#include <QFormLayout>
#include <QFrame>
#include <QLineEdit>
#include "tmessage.h"

class TMessageFormManager : public QObject
{
    Q_OBJECT

public:
    explicit TMessageFormManager(QFormLayout * formLayout, int insertOffset);
    ~TMessageFormManager();

    TMessage getMessage();
    void setMessage(const TMessage & message, bool * ok = nullptr);
    void clearRows();
    bool assignInputValues();

public slots:
    void validateInputValues();

private:
    QWidget * createInputField(const TMessagePart & messagePart, bool isLengthDeterminingMessagePart);

    TMessage m_message;
    QMap<int, QWidget*> m_inputs;
    QList<int> m_lengthDeterminingMessagePartIndexes;
    QFrame * m_separatorLine = nullptr;

    QFormLayout * m_formLayout;
    int m_insertOffset;
};
#endif // TMESSAGEFORMWIDGET_H
