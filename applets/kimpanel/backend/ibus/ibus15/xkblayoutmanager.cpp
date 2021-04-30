/*
 *  Copyright (C) 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "xkblayoutmanager.h"
#include <QDir>
#include <QProcess>
#include <QStringList>

const char *XKB_COMMAND = "setxkbmap";
const char *XKB_QUERY_ARG = "-query";
const char *XKB_LAYOUT_ARG = "-layout";
const char *XMODMAP_COMMAND = "xmodmap";
const char *XMODMAP_KNOWN_FILES[] = {".xmodmap", ".xmodmaprc", ".Xmodmap", ".Xmodmaprc"};

XkbLayoutManager::XkbLayoutManager()
    : m_useXkbModmap(false)
{
}

void XkbLayoutManager::setLatinLayouts(const gchar **variants, gsize length)
{
    m_latinLayouts.clear();
    for (gsize i = 0; i < length; i++) {
        m_latinLayouts.insert(QString::fromUtf8(variants[i]));
    }
}

void XkbLayoutManager::getLayout()
{
    QProcess process;
    process.start(QString::fromLatin1(XKB_COMMAND), QStringList(QString::fromLatin1(XKB_QUERY_ARG)));
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QList<QByteArray> lines = output.split('\n');
    Q_FOREACH (const QByteArray &line, lines) {
        QByteArray element("layout:");
        if (line.startsWith(element)) {
            m_defaultLayout = QString::fromLatin1(line.mid(element.length(), line.length())).trimmed();
        }

        element = "variant:";
        if (line.startsWith(element)) {
            m_defaultVariant = QString::fromLatin1(line.mid(element.length(), line.length())).trimmed();
        }

        element = "options:";
        if (line.startsWith(element)) {
            m_defaultOption = QString::fromLatin1(line.mid(element.length(), line.length())).trimmed();
        }
    }
}

void XkbLayoutManager::setLayout(IBusEngineDesc *desc)
{
    const gchar *clayout = ibus_engine_desc_get_layout(desc);
    const gchar *cvariant = ibus_engine_desc_get_layout_variant(desc);
    const gchar *coption = ibus_engine_desc_get_layout_option(desc);

    QString layout = QString::fromUtf8(clayout ? clayout : "");
    QString variant = QString::fromUtf8(cvariant ? cvariant : "");
    QString option = QString::fromUtf8(coption ? coption : "");

    if (layout == QLatin1String{"default"} && (variant.isEmpty() || variant == QLatin1String{"default"})
        && (option.isEmpty() && option == QLatin1String{"default"})) {
        return;
    }

    bool need_us_layout = false;
    if (variant != QLatin1String{"eng"}) {
        need_us_layout = m_latinLayouts.contains(layout);
    }
    if (!variant.isEmpty() && !need_us_layout) {
        need_us_layout = m_latinLayouts.contains(QLatin1String("%1(%2)").arg(layout, variant));
    }

    if (m_defaultLayout.isEmpty()) {
        getLayout();
    }

    if (layout.isEmpty() || layout == QLatin1String{"default"}) {
        layout = m_defaultLayout;
        variant = m_defaultVariant;
    }

    if (layout.isEmpty()) {
        g_warning("Could not get the correct layout");
        return;
    }

    if (option.isEmpty() || option == QLatin1String{"default"}) {
        option = m_defaultOption;
    } else {
        if (!(m_defaultOption.split(QLatin1Char{','}).contains(option))) {
            option = QLatin1String("%1,%2").arg(m_defaultOption, option);
        } else {
            option = m_defaultOption;
        }
    }

    if (need_us_layout) {
        layout += QLatin1String{",us"};
        if (!variant.isEmpty()) {
            variant += QLatin1Char{','};
        }
    }

    QStringList args;
    args << QString::fromLatin1(XKB_LAYOUT_ARG);
    args << layout;
    if (!variant.isEmpty() && variant != QLatin1String("default")) {
        args << QStringLiteral("-variant");
        args << variant;
    }
    if (!option.isEmpty() && option != QLatin1String("default")) {
        /*TODO: Need to get the session XKB options */
        args << QStringLiteral("-option");
        args << option;
    }

    int exit_status = QProcess::execute(QString::fromLatin1(XKB_COMMAND), args);

    if (exit_status != 0) {
        g_warning("Execute setxkbmap failed.");
    }

    runXmodmap();
}

void XkbLayoutManager::runXmodmap()
{
    if (!m_useXkbModmap) {
        return;
    }

    QDir home = QDir::home();
    for (size_t i = 0; i < sizeof(XMODMAP_KNOWN_FILES) / sizeof(XMODMAP_KNOWN_FILES[0]); i++) {
        if (home.exists(QString::fromLatin1(XMODMAP_KNOWN_FILES[i]))) {
            QProcess::startDetached(QString::fromLatin1(XMODMAP_COMMAND), QStringList{home.filePath(QString::fromLatin1(XMODMAP_KNOWN_FILES[i]))});
            break;
        }
    }
}

void XkbLayoutManager::setUseXkbModmap(bool use)
{
    m_useXkbModmap = use;
}
