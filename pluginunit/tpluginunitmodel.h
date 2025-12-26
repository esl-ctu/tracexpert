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

#ifndef TPLUGINUNITMODEL_H
#define TPLUGINUNITMODEL_H

#include <QObject>

#include "tcommon.h"
#include "tconfigparam.h"
#include "../project/tprojectitem.h"

class TPluginUnitContainer;

class TPluginUnitModel : public QObject, public virtual TProjectItem
{
    Q_OBJECT

public:
    explicit TPluginUnitModel(TCommon * unit, QObject * parent = nullptr, bool manual = false);
    virtual ~TPluginUnitModel();

    QString name() const override;
    QString info() const;

    virtual bool init();
    virtual bool deInit();

    virtual bool remove();

    bool isInit() const;
    bool initWhenAvailable() const;
    bool isAvailable() const;
    bool isManual() const;

    TConfigParam preInitParams() const;
    TConfigParam postInitParams() const;
    virtual TConfigParam setPreInitParams(const TConfigParam & param);
    virtual TConfigParam setPostInitParams(const TConfigParam & param);

    virtual bool toBeSaved() const override;
    virtual QDomElement save(QDomDocument & document) const override;
    virtual void load(QDomElement * element);

    virtual void bind(TCommon * unit);
    virtual void release();

    Status status() const override;

protected:
    TCommon * m_unit;

    QString m_name;
    QString m_info;

    TConfigParam m_preInitParam;
    TConfigParam m_postInitParam;

    bool m_isInit;
    bool m_wasInit;
    bool m_initWhenAvailable;
    bool m_isManual;

private:
    QByteArray saveParam(const TConfigParam & param) const;
    TConfigParam loadParam(const QByteArray & array) const;
};

#endif // TPLUGINUNITMODEL_H
