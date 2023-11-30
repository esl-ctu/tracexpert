#ifndef RANDOM_GLOBAL_H
#define RANDOM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TRANDOM_LIBRARY)
#  define TRANDOM_EXPORT Q_DECL_EXPORT
#else
#  define TRANDOM_EXPORT Q_DECL_IMPORT
#endif

#endif // RANDOM_GLOBAL_H
