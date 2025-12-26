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

#ifndef TSENDER_H
#define TSENDER_H

#include <QObject>

class TSender : public QObject
{
    Q_OBJECT

public:
    explicit TSender(QObject * parent = nullptr);

    virtual QString name();
    virtual QString info();

    bool isBusy();

public slots:
    void sendData(QByteArray data);

protected:
    virtual size_t writeData(const uint8_t * buffer, size_t len) = 0;

signals:
    void dataSent(QByteArray data);
    void sendFailed();

private:
    bool m_isBusy = false;
};

#endif // TSENDER_H
