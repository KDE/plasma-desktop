/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008-2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "fileexcludefilters.h"

namespace
{
const char *const s_defaultFileExcludeFilters[] = {
    // tmp files
    "*~",
    "*.part",

    // temporary build files
    "*.o",
    "*.la",
    "*.lo",
    "*.loT",
    "*.moc",
    "moc_*.cpp",
    "qrc_*.cpp",
    "ui_*.h",
    "cmake_install.cmake",
    "CMakeCache.txt",
    "CTestTestfile.cmake",
    "libtool",
    "config.status",
    "confdefs.h",
    "autom4te",
    "conftest",
    "confstat",
    "Makefile.am",

    // misc
    "*.csproj",
    "*.m4",
    "*.rej",
    "*.gmo",
    "*.pc",
    "*.omf",
    "*.aux",
    "*.tmp",
    "*.po",
    "*.vm*",
    "*.nvram",
    "*.rcore",
    "*.swap",
    "lzo",
    "litmain.sh",
    "*.orig",
    ".histfile.*",
    ".xsession-errors*",

    // Compiled files
    "*.class", // Java
    "*.pyc", // Python
    "*.elc", // Emacs Lisp

    // end of list
    nullptr,
};

const int s_defaultFileExcludeFiltersVersion = 2;

const char *const s_defaultFolderExcludeFilters[] = {
    "po",

    // VCS
    "CVS",
    ".svn",
    ".git",
    "_darcs",
    ".bzr",
    ".hg",

    // development
    "CMakeFiles",
    "CMakeTmp",
    "CMakeTmpQmake",
    ".moc",
    ".obj",
    ".pch",
    ".uic",

    // misc
    "core-dumps",
    "lost+found",

    // end of list
    nullptr,
};

const int s_defaultFolderExcludeFiltersVersion = 1;

const char *const s_sourceCodeMimeTypes[] = {
    "text/css",
    "text/x-c++src",
    "text/x-c++hdr",
    "text/x-csrc",
    "text/x-chdr", // c header files
    "text/x-python",
    "text/x-assembly",
    "text/x-java",
    "text/x-objsrc",
    "text/x-ruby",
    "text/x-scheme",
    "text/x-pascal",
    "text/x-yacc",
    "text/x-sed",
    "text/x-haskell",
    "text/asp",
    "application/x-awk",
    "application/x-cgi",
    "application/x-csh",
    "application/x-java",
    "application/x-javascript",
    "application/x-perl",
    "application/x-php",
    "application/x-python",
    "application/x-sh",
    "application/x-tex",

    // end of list
    nullptr,
};
const int s_sourceCodeMimeTypesVersion = 1;
}

QStringList Baloo::defaultExcludeFilterList()
{
    QStringList l;
    for (int i = 0; s_defaultFileExcludeFilters[i]; ++i)
        l << QLatin1String(s_defaultFileExcludeFilters[i]);
    for (int i = 0; s_defaultFolderExcludeFilters[i]; ++i)
        l << QLatin1String(s_defaultFolderExcludeFilters[i]);
    return l;
}

int Baloo::defaultExcludeFilterListVersion()
{
    return qMax(s_defaultFileExcludeFiltersVersion, s_defaultFolderExcludeFiltersVersion);
}

QStringList Baloo::sourceCodeMimeTypes()
{
    QStringList l;
    for (int i = 0; s_sourceCodeMimeTypes[i]; ++i)
        l << QLatin1String(s_sourceCodeMimeTypes[i]);

    return l;
}

QStringList Baloo::defaultExcludeMimetypes()
{
    return sourceCodeMimeTypes();
}

int Baloo::defaultExcludeMimetypesVersion()
{
    // The +1 is the image, video and audio mimetypes
    return s_sourceCodeMimeTypesVersion + 1;
}
