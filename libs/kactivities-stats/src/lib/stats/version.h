/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KACTIVITIESSTATS_VERSION_H
#define KACTIVITIESSTATS_VERSION_H

/** @file version.h <KActivities/Stats/Version> */

#include "kactivitiesstats_export.h"

/**
 * String version of libkactivitiesstats version, suitable for use in
 * file formats or network protocols
 */
#define KACTIVITIESSTATS_VERSION_STRING \
    "0.0.1"

/// @brief Major version of libkactivitiesstats, at compile time
#define KACTIVITIESSTATS_VERSION_MAJOR \
    0
/// @brief Minor version of libkactivitiesstats, at compile time
#define KACTIVITIESSTATS_VERSION_MINOR \
    0
/// @brief Release version of libkactivitiesstats, at compile time
#define KACTIVITIESSTATS_VERSION_RELEASE \
    1

#define KACTIVITIESSTATS_MAKE_VERSION(a, b, c) \
    (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile time macro for the version number of libkactivitiesstats
 */
#define KACTIVITIESSTATS_VERSION \
    KACTIVITIESSTATS_MAKE_VERSION(KACTIVITIESSTATS_VERSION_MAJOR, KACTIVITIESSTATS_VERSION_MINOR, KACTIVITIESSTATS_VERSION_RELEASE)

/**
 * Compile-time macro for checking the kactivities version. Not useful for
 * detecting the version of libkactivities at runtime.
 */
#define KACTIVITIESSTATS_IS_VERSION(a, b, c) \
    (KACTIVITIESSTATS_VERSION >= KACTIVITIESSTATS_MAKE_VERSION(a, b, c))

/**
 * Namespace for everything in libkactivitiesstats
 */
namespace KActivities {
namespace Stats {

/**
 * The runtime version of libkactivitiesstats
 */
KACTIVITIESSTATS_EXPORT unsigned int version();

/**
 * The runtime major version of libkactivitiesstats
 */
KACTIVITIESSTATS_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libkactivitiesstats
 */
KACTIVITIESSTATS_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libkactivitiesstats
 */
KACTIVITIESSTATS_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libkactivitiesstats
 */
KACTIVITIESSTATS_EXPORT const char *versionString();

} // Stats namespace
} // KActivities namespace

#endif // multiple inclusion guard
