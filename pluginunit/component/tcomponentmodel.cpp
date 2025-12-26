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

#include "tcomponentmodel.h"

#include "tcomponentcontainer.h"

TComponentModel::TComponentModel(TPlugin * plugin, TComponentContainer * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(plugin, parent, false), m_plugin(plugin)
{
    m_typeName = "component";

    m_IOdevices = new TIODeviceContainer(this);
    m_scopes = new TScopeContainer(this);
    m_analDevices = new TAnalDeviceContainer(this);
}

TComponentModel::~TComponentModel()
{
    if (isInit())
        TComponentModel::deInit();

    delete m_unit;
}

bool TComponentModel::init()
{
    if (isInit() || !TPluginUnitModel::init()) {
        return false;
    }

    m_isInit = true;

    QList<TIODevice *> IODevices = m_plugin->getIODevices();
    for (int i = 0; i < m_IOdevices->count(); i++) {
        bool found = false;
        TIODeviceModel * existingDevice = m_IOdevices->at(i);

        for (int j = 0; j < IODevices.count(); j++) {
            if (IODevices.at(j)->getName() == existingDevice->name()) {
                existingDevice->bind(IODevices.at(j));
                IODevices.removeAt(j);
                found = true;
                break;
            }
        }

        if (!found)
            existingDevice->bind(m_plugin->addIODevice(existingDevice->name(), existingDevice->info()));

        if (existingDevice->initWhenAvailable())
            existingDevice->init();
    }

    for (int i = 0; i < IODevices.length(); i++) {
        appendIODevice(IODevices[i]);
    }

    QList<TScope *> scopes = m_plugin->getScopes();
    for (int i = 0; i < m_scopes->count(); i++) {
        bool found = false;
        TScopeModel * existingDevice = m_scopes->at(i);

        for (int j = 0; j < scopes.count(); j++) {
            if (scopes.at(j)->getName() == existingDevice->name()) {
                existingDevice->bind(scopes.at(j));
                scopes.removeAt(j);
                found = true;
                break;
            }
        }

        if (!found)
            existingDevice->bind(m_plugin->addScope(existingDevice->name(), existingDevice->info()));

        if (existingDevice->initWhenAvailable())
            existingDevice->init();
    }

    for (int i = 0; i < scopes.length(); i++) {
        appendScope(scopes[i]);
    }

    QList<TAnalDevice *> analDevices = m_plugin->getAnalDevices();
    for (int i = 0; i < m_analDevices->count(); i++) {
        bool found = false;
        TAnalDeviceModel * existingDevice = m_analDevices->at(i);

        for (int j = 0; j < analDevices.count(); j++) {
            if (analDevices.at(j)->getName() == existingDevice->name()) {
                existingDevice->bind(analDevices.at(j));
                analDevices.removeAt(j);
                found = true;
                break;
            }
        }

        if (!found)
            existingDevice->bind(m_plugin->addAnalDevice(existingDevice->name(), existingDevice->info()));

        if (existingDevice->initWhenAvailable())
            existingDevice->init();
    }

    for (int i = 0; i < analDevices.length(); i++) {
        appendAnalDevice(analDevices[i]);
    }

    itemDataChanged();

    return true;
}

bool TComponentModel::deInit()
{
    if (!isInit()) {
        return false;
    }

    QList<TIODeviceModel *> removedIODevices;

    for (int i = 0; i < m_IOdevices->count(); i++) {
        TIODeviceModel * IODevice = m_IOdevices->at(i);

        if (IODevice->isInit() && !IODevice->deInit())
            return false;

        if (IODevice->toBeSaved())
            IODevice->release();
        else
            removedIODevices.append(IODevice);
    }

    for (int i = 0; i < removedIODevices.count(); i++) {
        TIODeviceModel * IODevice = removedIODevices.at(i);
        m_IOdevices->remove(IODevice);
        delete IODevice;
    }

    QList<TScopeModel *> removedScopes;

    for (int i = 0; i < m_scopes->count(); i++) {
        TScopeModel * scope = m_scopes->at(i);

        if (scope->isInit() && !scope->deInit())
            return false;

        if (scope->toBeSaved())
            scope->release();
        else
            removedScopes.append(scope);
    }

    for (int i = 0; i < removedScopes.count(); i++) {
        TScopeModel * scope = removedScopes.at(i);
        m_scopes->remove(scope);
        delete scope;
    }

    QList<TAnalDeviceModel *> removedAnalDevices;

    for (int i = 0; i < m_analDevices->count(); i++) {
        TAnalDeviceModel * analDevice = m_analDevices->at(i);

        if (analDevice->isInit() && !analDevice->deInit())
            return false;

        if (analDevice->toBeSaved())
            analDevice->release();
        else
            removedAnalDevices.append(analDevice);
    }

    for (int i = 0; i < removedAnalDevices.count(); i++) {
        TAnalDeviceModel * analDevice = removedAnalDevices.at(i);
        m_analDevices->remove(analDevice);
        delete analDevice;
    }

    if (!TPluginUnitModel::deInit()) {
        return false;
    }

    m_isInit = false;

    itemDataChanged();

    return true;
}

int TComponentModel::IODeviceCount() const
{
    return m_IOdevices->count();
}

int TComponentModel::scopeCount() const
{
    return m_scopes->count();
}

int TComponentModel::analDeviceCount() const
{
    return m_analDevices->count();
}

TIODeviceModel * TComponentModel::IODevice(int index) const
{
    return m_IOdevices->at(index);
}

TScopeModel * TComponentModel::scope(int index) const
{
    return m_scopes->at(index);
}

TAnalDeviceModel * TComponentModel::analDevice(int index) const
{
    return m_analDevices->at(index);
}

bool TComponentModel::canAddIODevice() const
{
    return m_plugin->canAddIODevice();
}

bool TComponentModel::canAddScope() const
{
    return m_plugin->canAddScope();
}

bool TComponentModel::canAddAnalDevice() const
{
    return m_plugin->canAddAnalDevice();
}

bool TComponentModel::addIODevice(QString name, QString info)
{
    if (m_plugin->canAddIODevice()) {
        bool ok;
        TIODevice * IODevice = m_plugin->addIODevice(name, info, &ok);

        if (ok) {
            appendIODevice(IODevice, true);
            return true;
        }
    }

    return false;
}

bool TComponentModel::addScope(QString name, QString info)
{
    if (m_plugin->canAddScope()) {
        bool ok;
        TScope * scope = m_plugin->addScope(name, info, &ok);

        if (ok) {
            appendScope(scope, true);
            return true;
        }
    }

    return false;
}

bool TComponentModel::addAnalDevice(QString name, QString info)
{
    if (m_plugin->canAddAnalDevice()) {
        bool ok;
        TAnalDevice * analDevice = m_plugin->addAnalDevice(name, info, &ok);

        if (ok) {
            appendAnalDevice(analDevice, true);
            return true;
        }
    }

    return false;
}

bool TComponentModel::removeIODevice(TIODeviceModel * IODevice)
{
    if (IODevice->isInit() && !IODevice->deInit())
        return false;

    if (!IODevice->isManual() && IODevice->isAvailable())
        return false;

    bool hideContainer = false;

    if (m_IOdevices->count() == 1) {
        hideContainer = true;
        beginRemoveChild(0);
    }

    if (!m_IOdevices->remove(IODevice))
        return false;

    if (hideContainer)
        endRemoveChild();

    delete IODevice;
    return true;
}

bool TComponentModel::removeScope(TScopeModel * scope)
{
    if (scope->isInit() && !scope->deInit())
        return false;

    if (!scope->isManual() && scope->isAvailable())
        return false;

    bool hideContainer = false;

    if (m_scopes->count() == 1) {
        hideContainer = true;
        beginRemoveChild(m_IOdevices->count() > 0);
    }

    if (!m_scopes->remove(scope))
        return false;

    if (hideContainer)
        endRemoveChild();

    delete scope;
    return true;
}

bool TComponentModel::removeAnalDevice(TAnalDeviceModel * analDevice)
{
    if (analDevice->isInit() && !analDevice->deInit())
        return false;

    if (!analDevice->isManual() && analDevice->isAvailable())
        return false;

    bool hideContainer = false;

    if (m_analDevices->count() == 1) {
        hideContainer = true;
        beginRemoveChild((m_IOdevices->count() > 0) + (m_scopes->count() > 0));
    }

    if (!m_analDevices->remove(analDevice))
        return false;

    if (hideContainer)
        endRemoveChild();

    delete analDevice;
    return true;
}

TIODeviceContainer * TComponentModel::IODeviceContainer() const
{
    return m_IOdevices;
}

TScopeContainer * TComponentModel::scopeContainer() const
{
    return m_scopes;
}

TAnalDeviceContainer * TComponentModel::analDeviceContainer() const
{
    return m_analDevices;
}

int TComponentModel::childrenCount() const
{
    int count = 0;

    if (m_IOdevices && m_IOdevices->count()) {
        count++;
    }
    if (m_scopes && m_scopes->count()) {
        count++;
    }
    if (m_analDevices && m_analDevices->count()) {
        count++;
    }

    return count;
}

TProjectItem * TComponentModel::child(int row) const
{
    if (m_IOdevices && m_IOdevices->count()) {
        if (row == 0) {
            return m_IOdevices;
        }
        else {
            row--;
        }
    }

    if (m_scopes && m_scopes->count()) {
        if (row == 0) {
            return m_scopes;
        }
        else {
            row--;
        }
    }

    if (m_analDevices && m_analDevices->count()) {
        if (row == 0) {
            return m_analDevices;
        }
        else {
            row--;
        }
    }

    return nullptr;
}

void TComponentModel::load(QDomElement * element)
{
    TPluginUnitModel::load(element);

    if (m_unit && !m_preInitParam.isEmpty())
        m_unit->setPreInitParams(m_preInitParam);

    QDomNodeList children = element->childNodes();

    for (int i = 0; i < children.count(); i++) {
        QDomElement child = children.at(i).toElement();
        if (child.isNull())
            throw tr("Unexpected node type");

        if (child.tagName() == "iodevices")
            loadIODevices(&child);

        if (child.tagName() == "scopes")
            loadScopes(&child);

        if (child.tagName() == "analdevices")
            loadAnalDevices(&child);
    }
}

void TComponentModel::appendIODevice(TIODevice * IODevice, bool manual, QDomElement * element)
{
    TIODeviceModel * IODeviceModel = new TIODeviceModel(IODevice, m_IOdevices, manual);
    connect(IODeviceModel, &TIODeviceModel::initialized, this, &TComponentModel::IODeviceInitialized);
    connect(IODeviceModel, &TIODeviceModel::deinitialized, this, &TComponentModel::IODeviceDeinitialized);

    if (element)
        IODeviceModel->load(element);

    bool showContainer = false;

    if (!m_IOdevices->count()) {
        showContainer = true;
        beginInsertChild(0);
    }

    m_IOdevices->add(IODeviceModel);

    if (showContainer)
        endInsertChild();
}

void TComponentModel::appendScope(TScope * scope, bool manual, QDomElement * element)
{
    TScopeModel * scopeModel = new TScopeModel(scope, m_scopes, manual);
    connect(scopeModel, &TScopeModel::initialized, this, &TComponentModel::scopeInitialized);
    connect(scopeModel, &TScopeModel::deinitialized, this, &TComponentModel::scopeDeinitialized);

    if (element)
        scopeModel->load(element);

    bool showContainer = false;

    if (!m_scopes->count()) {
        showContainer = true;
        beginInsertChild(m_IOdevices->count() > 0);
    }

    m_scopes->add(scopeModel);

    if (showContainer)
        endInsertChild();
}

void TComponentModel::appendAnalDevice(TAnalDevice * analDevice, bool manual, QDomElement * element)
{
    TAnalDeviceModel * analDeviceModel = new TAnalDeviceModel(analDevice, m_analDevices, manual);
    connect(analDeviceModel, &TAnalDeviceModel::initialized, this, &TComponentModel::analDeviceInitialized);
    connect(analDeviceModel, &TAnalDeviceModel::deinitialized, this, &TComponentModel::analDeviceDeinitialized);

    if (element)
        analDeviceModel->load(element);

    bool showContainer = false;

    if (!m_analDevices->count()) {
        showContainer = true;
        beginInsertChild((m_IOdevices->count() > 0) + (m_scopes->count() > 0));
    }

    m_analDevices->add(analDeviceModel);

    if (showContainer)
        endInsertChild();
}

void TComponentModel::loadIODevices(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != "iodevices")
        throw tr("Unexpected node name");

    QDomNodeList children = element->childNodes();

    for (int i = 0; i < children.count(); i++) {
        QDomElement child = children.at(i).toElement();
        if (child.isNull())
            throw tr("Unexpected node type");

        if (child.tagName() == "iodevice")
            loadIODevice(&child);
        else
            throw tr("Unexpected node name");
    }
}

void TComponentModel::loadIODevice(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != "iodevice")
        throw tr("Unexpected node name");

    QString name = element->attribute("name");

    if (name.isEmpty())
        throw tr("Missing component name");

    bool found = false;

    QString isManualString = element->attribute("manual");

    if (isManualString.isEmpty())
        throw tr("Missing component is manual attribute");

    bool ok;
    bool isManual = isManualString.toInt(&ok);

    if (!ok)
        throw tr("Unexpected value of io device is manual attribute");

    for (int i = 0; i < m_IOdevices->count(); i++) {
        TIODeviceModel * IODevice = m_IOdevices->at(i);

        if (name == IODevice->name()) {
            if (isManual)
                element->setAttribute("name", element->attribute("name")+" rename");
            else {
                IODevice->load(element);
                found = true;
            }

            break;
        }
    }

    if (!found) {
        appendIODevice(nullptr, false, element);
    }
}

void TComponentModel::loadScopes(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != "scopes")
        throw tr("Unexpected node name");

    QDomNodeList children = element->childNodes();

    for (int i = 0; i < children.count(); i++) {
        QDomElement child = children.at(i).toElement();
        if (child.isNull())
            throw tr("Unexpected node type");

        if (child.tagName() == "scope")
            loadScope(&child);
        else
            throw tr("Unexpected node name");
    }
}

void TComponentModel::loadScope(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != "scope")
        throw tr("Unexpected node name");

    QString name = element->attribute("name");

    if (name.isEmpty())
        throw tr("Missing component name");

    bool found = false;

    QString isManualString = element->attribute("manual");

    if (isManualString.isEmpty())
        throw tr("Missing component is manual attribute");

    bool ok;
    bool isManual = isManualString.toInt(&ok);

    if (!ok)
        throw tr("Unexpected value of io device is manual attribute");

    for (int i = 0; i < m_scopes->count(); i++) {
        TScopeModel * scope = m_scopes->at(i);

        if (name == scope->name()) {
            if (isManual)
                element->setAttribute("name", element->attribute("name")+" rename");
            else {
                scope->load(element);
                found = true;
            }

            break;
        }
    }

    if (!found) {
        appendScope(nullptr, false, element);
    }
}

void TComponentModel::loadAnalDevices(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != "analdevices")
        throw tr("Unexpected node name");

    QDomNodeList children = element->childNodes();

    for (int i = 0; i < children.count(); i++) {
        QDomElement child = children.at(i).toElement();
        if (child.isNull())
            throw tr("Unexpected node type");

        if (child.tagName() == "analdevice")
            loadAnalDevice(&child);
        else
            throw tr("Unexpected node name");
    }
}

void TComponentModel::loadAnalDevice(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != "analdevice")
        throw tr("Unexpected node name");

    QString name = element->attribute("name");

    if (name.isEmpty())
        throw tr("Missing component name");

    bool found = false;

    QString isManualString = element->attribute("manual");

    if (isManualString.isEmpty())
        throw tr("Missing component is manual attribute");

    bool ok;
    bool isManual = isManualString.toInt(&ok);

    if (!ok)
        throw tr("Unexpected value of analytic device is manual attribute");

    for (int i = 0; i < m_analDevices->count(); i++) {
        TAnalDeviceModel * analDevice = m_analDevices->at(i);

        if (name == analDevice->name()) {
            if (isManual)
                element->setAttribute("name", element->attribute("name")+" rename");
            else {
                analDevice->load(element);
                found = true;
            }

            break;
        }
    }

    if (!found) {
        appendAnalDevice(nullptr, false, element);
    }
}
