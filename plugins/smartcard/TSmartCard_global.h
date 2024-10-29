#ifndef SMARTCARD_GLOBAL_H
#define SMARTCARD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TSMARTCARD_LIBRARY)
#  define TSMARTCARD_EXPORT Q_DECL_EXPORT
#else
#  define TSMARTCARD_EXPORT Q_DECL_IMPORT
#endif

#endif // SMARTCARD_GLOBAL_H
