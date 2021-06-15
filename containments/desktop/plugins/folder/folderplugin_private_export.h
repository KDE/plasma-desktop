/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FOLDERPLUGIN_PRIVATE_EXPORT_H
#define FOLDERPLUGIN_PRIVATE_EXPORT_H

#include "folderplugin_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef FOLDERPLUGIN_TESTS_EXPORT
#define FOLDERPLUGIN_TESTS_EXPORT FOLDERPLUGIN_EXPORT
#endif
#else /* not compiling tests */
#define FOLDERPLUGIN_TESTS_EXPORT
#endif

#endif // FOLDERPLUGIN_PRIVATE_EXPORT_H
