#ifndef __KFI_CONSTANTS_H__
#define __KFI_CONSTANTS_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QLatin1String>
#include <KLocalizedString>
#include <kio/udsentry.h>

#define KFI_NAME                     "kfontinst"
#define KFI_CATALOGUE                KFI_NAME

#define KFI_INSTALLER                QLatin1String("kfontinst")
#define KFI_VIEWER                   QLatin1String("kfontview")
#define KFI_PRINTER                  QLatin1String("kfontprint"), "libexec"

#define KFI_PRINT_GROUP              "Print"
#define KFI_KIO_FONTS_PROTOCOL       "fonts"
#define KFI_KIO_FONTS_USER           I18N_NOOP("Personal")
#define KFI_KIO_FONTS_SYS            I18N_NOOP("System")
#define KFI_KIO_FONTS_ALL            I18N_NOOP("All")
#define KFI_KIO_NO_CLEAR             "noclear"
#define KFI_KIO_TIMEOUT              "timeout"
#define KFI_KIO_FACE                 "face"

#define KFI_IFACE                    "org.kde.fontinst"

#define KFI_FILE_QUERY               "file"
#define KFI_NAME_QUERY               "name"
#define KFI_STYLE_QUERY              "style"
#define KFI_MIME_QUERY               "mime"

#define KFI_SYS_USER                 "root"

#define KFI_AUTHINF_USER             I18N_NOOP("Administrator")
#define KFI_UI_CFG_FILE              KFI_NAME"uirc"
#define KFI_ROOT_CFG_DIR             "/etc/fonts/"
#define KFI_DEFAULT_SYS_FONTS_FOLDER "/usr/local/share/fonts/"

#define KFI_NO_STYLE_INFO            0xFFFFFFFF
#define KFI_NAME_KEY                 "Name="
#define KFI_STYLE_KEY                "Style="
#define KFI_PATH_KEY                 "Path="
#define KFI_FACE_KEY                 "Face="

#define KFI_FONTS_PACKAGE            ".fonts.zip"
#define KFI_FONTS_PACKAGE_LEN        10
#define KFI_GROUPS_FILE              "fontgroups.xml"
#define KFI_TMP_DIR_PREFIX           "kfi"

#define KFI_NULL_SETTING             0xFF

namespace KFI
{

// KIO::special
enum ESpecial
{
    SPECIAL_RESCAN = 0,
    SPECIAL_CONFIGURE
};

// UDS_EXTRA entries...
enum EUdsExtraEntries
{
    UDS_EXTRA_FC_STYLE        = (((KIO::UDSEntry::UDS_EXTRA|KIO::UDSEntry::UDS_STRING)^KIO::UDSEntry::UDS_STRING)|KIO::UDSEntry::UDS_NUMBER)+1,
    UDS_EXTRA_FILE_NAME       = KIO::UDSEntry::UDS_EXTRA+2,
    UDS_EXTRA_FILE_FACE       = KIO::UDSEntry::UDS_EXTRA+3
};

}

// Font name...
#define KFI_WEIGHT_THIN              I18N_NOOP("Thin")
#define KFI_WEIGHT_EXTRALIGHT        I18N_NOOP("Extra Light")
#define KFI_WEIGHT_ULTRALIGHT        I18N_NOOP("Ultra Light")
#define KFI_WEIGHT_LIGHT             I18N_NOOP("Light")
#define KFI_WEIGHT_REGULAR           I18N_NOOP("Regular")
#define KFI_WEIGHT_NORMAL            I18N_NOOP("Normal")
#define KFI_WEIGHT_MEDIUM            I18N_NOOP("Medium")
#define KFI_WEIGHT_DEMIBOLD          I18N_NOOP("Demi Bold")
#define KFI_WEIGHT_SEMIBOLD          I18N_NOOP("Semi Bold")
#define KFI_WEIGHT_BOLD              I18N_NOOP("Bold")
#define KFI_WEIGHT_EXTRABOLD         I18N_NOOP("Extra Bold")
#define KFI_WEIGHT_ULTRABOLD         I18N_NOOP("Ultra Bold")
#define KFI_WEIGHT_BLACK             I18N_NOOP("Black")
#define KFI_WEIGHT_HEAVY             I18N_NOOP("Heavy")

#define KFI_SLANT_ROMAN              I18N_NOOP("Roman")
#define KFI_SLANT_ITALIC             I18N_NOOP("Italic")
#define KFI_SLANT_OBLIQUE            I18N_NOOP("Oblique")

#define KFI_WIDTH_ULTRACONDENSED     I18N_NOOP("Ultra Condensed")
#define KFI_WIDTH_EXTRACONDENSED     I18N_NOOP("Extra Condensed")
#define KFI_WIDTH_CONDENSED          I18N_NOOP("Condensed")
#define KFI_WIDTH_SEMICONDENSED      I18N_NOOP("Semi Condensed")
#define KFI_WIDTH_NORMAL             I18N_NOOP("Normal")
#define KFI_WIDTH_SEMIEXPANDED       I18N_NOOP("Semi Expanded")
#define KFI_WIDTH_EXPANDED           I18N_NOOP("Expanded")
#define KFI_WIDTH_EXTRAEXPANDED      I18N_NOOP("Extra Expanded")
#define KFI_WIDTH_ULTRAEXPANDED      I18N_NOOP("Ultra Expanded")

#define KFI_SPACING_MONO             I18N_NOOP("Monospaced")
#define KFI_SPACING_CHARCELL         I18N_NOOP("Charcell")
#define KFI_SPACING_PROPORTIONAL     I18N_NOOP("Proportional")

#define KFI_UNKNOWN_FOUNDRY          I18N_NOOP("Unknown")

#endif
