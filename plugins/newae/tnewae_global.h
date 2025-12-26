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
// Tomáš Přeučil (initial author)

#ifndef NEWAE_GLOBAL_H
#define NEWAE_GLOBAL_H
#pragma once

#include <QtCore/qglobal.h>

#if defined(TNEWAE_LIBRARY)
#  define TNEWAE_EXPORT Q_DECL_EXPORT
#else
#  define TNEWAE_EXPORT Q_DECL_IMPORT
#endif

typedef enum {
    TARGET_NORMAL,
    TARGET_CW305,
    TARGET_CW310
} targetType;

#include "tplugin.h"
#include "tscope.h"
#include "tnewae.h"
#include "tnewaescope.h"
#include "tnewaedevice.h"

#endif // NEWAE_GLOBAL_H
