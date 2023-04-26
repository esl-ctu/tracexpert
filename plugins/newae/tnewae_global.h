
#ifndef NEWAE_GLOBAL_H
#define NEWAE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TNEWAE_LIBRARY)
#  define TNEWAE_EXPORT Q_DECL_EXPORT
#else
#  define TNEWAE_EXPORT Q_DECL_IMPORT
#endif

#endif // NEWAE_GLOBAL_H
