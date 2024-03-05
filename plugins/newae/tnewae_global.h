
#ifndef NEWAE_GLOBAL_H
#define NEWAE_GLOBAL_H
#pragma once

#include <QtCore/qglobal.h>

#if defined(TNEWAE_LIBRARY)
#  define TNEWAE_EXPORT Q_DECL_EXPORT
#else
#  define TNEWAE_EXPORT Q_DECL_IMPORT
#endif

#include "tplugin.h"
#include "tscope.h"
#include "tnewae.h"
#include "tnewaescope.h"
#include "tnewaedevice.h"

#endif // NEWAE_GLOBAL_H
