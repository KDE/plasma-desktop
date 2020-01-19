/*
 * Copyright 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDir>
#include <QUrl>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>

#include <KTar>
#include <KLocalizedString>
#include <KNS3/DownloadDialog>

#include "gtkpage.h"
#include "gtkthemesmodel.h"

GtkPage::GtkPage(QObject *parent)
    : QObject(parent)
    , m_gtk2ThemesModel(new GtkThemesModel(this))
    , m_gtk3ThemesModel(new GtkThemesModel(this))
    , gtkConfigInterface(
        QStringLiteral("org.kde.GtkConfig"),
        QStringLiteral("/GtkConfig"),
        QStringLiteral("org.kde.GtkConfig")
    )
{
    connect(m_gtk2ThemesModel, &GtkThemesModel::themeRemoved, this, &GtkPage::onThemeRemoved);
    connect(m_gtk3ThemesModel, &GtkThemesModel::themeRemoved, this, &GtkPage::onThemeRemoved);

    connect(m_gtk2ThemesModel, &GtkThemesModel::selectedThemeChanged, this, [this](){
        Q_EMIT gtkThemeSettingsChanged();
    });
    connect(m_gtk3ThemesModel, &GtkThemesModel::selectedThemeChanged, this, [this](){
        Q_EMIT gtkThemeSettingsChanged();
    });
}

GtkPage::~GtkPage()
{
    delete m_gtk2ThemesModel;
    delete m_gtk3ThemesModel;
}

QString GtkPage::gtk2ThemeFromConfig()
{
    QDBusReply<QString> dbusReply = gtkConfigInterface.call(QStringLiteral("gtk2Theme"));
    return dbusReply.value();
}

QString GtkPage::gtk3ThemeFromConfig()
{
    QDBusReply<QString> dbusReply = gtkConfigInterface.call(QStringLiteral("gtk3Theme"));
    return dbusReply.value();
}

bool GtkPage::gtk2PreviewAvailable()
{
    return !QStandardPaths::findExecutable(QStringLiteral("gtk_preview"), {CMAKE_INSTALL_FULL_LIBEXECDIR}).isEmpty();
}

bool GtkPage::gtk3PreviewAvailable()
{
    return !QStandardPaths::findExecutable(QStringLiteral("gtk3_preview"), {CMAKE_INSTALL_FULL_LIBEXECDIR}).isEmpty();
}

void GtkPage::showGtk2Preview()
{
    gtkConfigInterface.call(QStringLiteral("showGtk2ThemePreview"), m_gtk2ThemesModel->selectedTheme());
}

void GtkPage::showGtk3Preview()
{
    gtkConfigInterface.call(QStringLiteral("showGtk3ThemePreview"), m_gtk3ThemesModel->selectedTheme());
}

void GtkPage::onThemeRemoved()
{
    load();
    defaults();
    save();
}

void GtkPage::onGhnsEntriesChanged(const QQmlListReference &changedEnties)
{
    if (changedEnties.count() == 0) {
        return;
    }

    load();
}

void GtkPage::installGtkThemeFromFile(const QUrl &fileUrl)
{
    QString themesInstallDirectoryPath(QDir::homePath() + QStringLiteral("/.themes"));
    QDir::home().mkpath(themesInstallDirectoryPath);
    KTar themeArchive(fileUrl.path());
    themeArchive.open(QIODevice::ReadOnly);

    auto showError = [this, fileUrl]() {
        Q_EMIT showErrorMessage(i18n("%1 is not a valid GTK Theme archive.", fileUrl.fileName()));
    };

    QString firstEntryName = themeArchive.directory()->entries().first();
    const KArchiveEntry *possibleThemeDirectory = themeArchive.directory()->entry(firstEntryName);
    if (possibleThemeDirectory->isDirectory()) {
        const KArchiveDirectory *themeDirectory = static_cast<const KArchiveDirectory *>(possibleThemeDirectory);
        QStringList archiveSubitems = themeDirectory->entries();

        if (!archiveSubitems.contains(QStringLiteral("gtk-2.0")) && archiveSubitems.indexOf(QRegExp("gtk-3.*")) == -1) {
            showError();
            return;
        }
    } else {
        showError();
        return;
    }

    themeArchive.directory()->copyTo(themesInstallDirectoryPath);

    load();
}

void GtkPage::save()
{
    gtkConfigInterface.call(QStringLiteral("setGtk2Theme"), m_gtk2ThemesModel->selectedTheme());
    gtkConfigInterface.call(QStringLiteral("setGtk3Theme"), m_gtk3ThemesModel->selectedTheme());
}

void GtkPage::defaults()
{
    Q_EMIT selectGtk2ThemeInCombobox(QStringLiteral("Breeze"));
    Q_EMIT selectGtk3ThemeInCombobox(QStringLiteral("Breeze"));
}

void GtkPage::load()
{
    m_gtk2ThemesModel->loadGtk2();
    m_gtk3ThemesModel->loadGtk3();
    Q_EMIT selectGtk2ThemeInCombobox(gtk2ThemeFromConfig());
    Q_EMIT selectGtk3ThemeInCombobox(gtk3ThemeFromConfig());
}
