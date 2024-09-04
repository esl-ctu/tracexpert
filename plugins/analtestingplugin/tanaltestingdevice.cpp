#include "tanaltestingdevice.h"

TAnalTestingDevice::TAnalTestingDevice() {
    
}

TAnalTestingDevice::~TAnalTestingDevice() {

}

QString TAnalTestingDevice::getName() const {
    return QString("Anal testing device");
}

QString TAnalTestingDevice::getInfo() const {
    return QString("Default device for testing analytical device-related features");
}


TConfigParam TAnalTestingDevice::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TAnalTestingDevice::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TAnalTestingDevice::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TAnalTestingDevice::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TAnalTestingDevice::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TAnalTestingDevice::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

QList<TAnalAction *> TAnalTestingDevice::getActions() const
{
    return QList<TAnalAction *>();
}

QList<TAnalInputStream *> TAnalTestingDevice::getInputDataStreams() const
{
    return QList<TAnalInputStream *>();
}

QList<TAnalOutputStream *> TAnalTestingDevice::getOutputDataStreams() const
{
    return QList<TAnalOutputStream *>();
}

bool TAnalTestingDevice::isBusy() const
{
    return false;
}
