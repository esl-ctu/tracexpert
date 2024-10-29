#ifndef TPS6000_GLOBAL_H
#define TPS6000_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TPS6000a_LIBRARY)
#  define TPS6000a_EXPORT Q_DECL_EXPORT
#else
#  define TPS6000a_EXPORT Q_DECL_IMPORT
#endif

#endif // TPS6000_GLOBAL_H
