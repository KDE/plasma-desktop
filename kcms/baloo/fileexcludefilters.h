/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008-2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QStringList>

namespace Baloo
{
/**
 * \return A list of default exclude filters to be used
 * in the filewatch service to ignore temporary files
 * and folders that change a lot and as a basis for the
 * user configurable exclude filters in the strigi service.
 */
QStringList defaultExcludeFilterList();

/**
 * \return The version of the default exclude filter list.
 * This is increased whenever the list changes.
 */
int defaultExcludeFilterListVersion();

QStringList defaultExcludeMimetypes();
int defaultExcludeMimetypesVersion();

QStringList sourceCodeMimeTypes();
}
