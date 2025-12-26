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
// Petr Socha (initial author)

#ifndef TPREDICTPLUGIN_GLOBAL_H
#define TPREDICTPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TPREDICTPLUGIN_LIBRARY)
#  define TPREDICTPLUGIN_EXPORT Q_DECL_EXPORT
#else
#  define TPREDICTPLUGIN_EXPORT Q_DECL_IMPORT
#endif

#endif // TPREDICTPLUGIN_GLOBAL_H

