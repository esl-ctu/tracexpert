#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(QString & name, QString & info){

}


TnewaeDevice::~TnewaeDevice(){

}

QString TnewaeDevice::getIODeviceName() const{

}

QString TnewaeDevice::getIODeviceInfo() const{

}

TConfigParam TnewaeDevice::getPreInitParams() const{

}

TConfigParam TnewaeDevice::setPreInitParams(TConfigParam params){

}

void TnewaeDevice::init(bool *ok/* = nullptr*/){

}

void TnewaeDevice::deInit(bool *ok/* = nullptr*/){

}

TConfigParam TnewaeDevice::getPostInitParams() const{

}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){

}

size_t TnewaeDevice::writeData(const uint8_t * buffer, size_t len){

}

size_t TnewaeDevice::readData(uint8_t * buffer, size_t len){

}
