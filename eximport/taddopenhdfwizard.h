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
// Petr Socha (initial author)

#ifndef TADDOPENHDFWIZARD_H
#define TADDOPENHDFWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLineEdit>
#include <QRadioButton>
#include <QSharedPointer>
#include <QLabel>

class QCheckBox;

class THdfSession;
class THdfBrowserWidget;

// -------------------- Page 1 --------------------

class AddOpenFilePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit AddOpenFilePage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;

private:
    QRadioButton *m_radioCreateNew  = nullptr;
    QRadioButton *m_radioOpenExisting = nullptr;
    QLineEdit *m_fileEdit = nullptr;
};

// -------------------- Page 2 --------------------

class HdfBrowsePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit HdfBrowsePage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override { return -1; } // final page

private:
    THdfBrowserWidget *m_browser = nullptr;
    QLabel *m_fileNameLabel = nullptr;
    QLabel *m_filePathLabel = nullptr;
};

// -------------------- Wizard --------------------

class TAddOpenHdfWizard : public QWizard
{
    Q_OBJECT
public:
    explicit TAddOpenHdfWizard(QWidget *parent = nullptr);
    ~TAddOpenHdfWizard() override;

    QSharedPointer<THdfSession> hdfSession() const { return m_hdfSession; }

    QString filePath() const;

private:
    QSharedPointer<THdfSession> m_hdfSession;
};

#endif // TADDOPENHDFWIZARD_H
