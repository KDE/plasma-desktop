/*
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>
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

#include "colors.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>

#include <KAboutData>
#include <KColorUtils>
#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KWindowSystem>

#include <KIO/FileCopyJob>
#include <KIO/DeleteJob>
#include <KIO/JobUiDelegate>

#include <knewstuffcore_version.h>
#if KNEWSTUFFCORE_VERSION_MAJOR==5 && KNEWSTUFFCORE_VERSION_MINOR>=68
#include <KNSCore/EntryWrapper>
#endif

#include <algorithm>

#include "../krdb/krdb.h"

#include "colorsmodel.h"
#include "filterproxymodel.h"
#include "colorssettings.h"
#include "colorsdata.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMColorsFactory, "kcm_colors.json", registerPlugin<KCMColors>();registerPlugin<ColorsData>();)

KCMColors::KCMColors(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_model(new ColorsModel(this))
    , m_filteredModel(new FilterProxyModel(this))
    , m_data(new ColorsData(this))
    , m_config(KSharedConfig::openConfig(QStringLiteral("kdeglobals")))
{
    qmlRegisterUncreatableType<KCMColors>("org.kde.private.kcms.colors", 1, 0, "KCM", QStringLiteral("Cannot create instances of KCM"));
    qmlRegisterType<ColorsModel>();
    qmlRegisterType<FilterProxyModel>();
    qmlRegisterType<ColorsSettings>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_colors"), i18n("Colors"),
                                       QStringLiteral("2.0"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@privat.broulik.de"));
    setAboutData(about);

    connect(m_model, &ColorsModel::pendingDeletionsChanged, this, &KCMColors::settingsChanged);

    connect(m_model, &ColorsModel::selectedSchemeChanged, this, [this](const QString &scheme) {
        m_selectedSchemeDirty = true;
        colorsSettings()->setColorScheme(scheme);
    });

    connect(colorsSettings(), &ColorsSettings::colorSchemeChanged, this, [this] {
        m_model->setSelectedScheme(colorsSettings()->colorScheme());
    });

    connect(m_model, &ColorsModel::selectedSchemeChanged, m_filteredModel, &FilterProxyModel::setSelectedScheme);
    m_filteredModel->setSourceModel(m_model);
}

KCMColors::~KCMColors()
{
    m_config->markAsClean();
}

ColorsModel *KCMColors::model() const
{
    return m_model;
}

FilterProxyModel *KCMColors::filteredModel() const
{
    return m_filteredModel;
}

ColorsSettings *KCMColors::colorsSettings() const
{
    return m_data->settings();
}

bool KCMColors::downloadingFile() const
{
    return m_tempCopyJob;
}

void KCMColors::reloadModel(const QQmlListReference &changedEntries)
{
    m_model->load();

#if KNEWSTUFFCORE_VERSION_MAJOR==5 && KNEWSTUFFCORE_VERSION_MINOR>=68
    // If a new theme was installed, select the first color file in it
    if (changedEntries.count() > 0) {
        QStringList installedThemes;

        const QString suffix = QStringLiteral(".colors");

        for (int i = 0; i < changedEntries.count(); ++i) {
            KNSCore::EntryWrapper* entry = qobject_cast<KNSCore::EntryWrapper*>(changedEntries.at(i));
            if (entry && entry->entry().status() == KNS3::Entry::Installed) {
                for (const QString &path : entry->entry().installedFiles()) {
                    const QString fileName = path.section(QLatin1Char('/'), -1, -1);

                    const int suffixPos = fileName.indexOf(suffix);
                    if (suffixPos != fileName.length() - suffix.length()) {
                        continue;
                    }

                    installedThemes.append(fileName.left(suffixPos));
                }

                if (!installedThemes.isEmpty()) {
                    // The list is sorted by (potentially translated) name
                    // but that would require us parse every file, so this should be close enough
                    std::sort(installedThemes.begin(), installedThemes.end());

                    m_model->setSelectedScheme(installedThemes.constFirst());
                }
                // Only do this for the first newly installed theme we find
                break;
            }
        }
    }
#endif
}

void KCMColors::installSchemeFromFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        installSchemeFile(url.toLocalFile());
        return;
    }

    if (m_tempCopyJob) {
        return;
    }

    m_tempInstallFile.reset(new QTemporaryFile());
    if (!m_tempInstallFile->open()) {
        emit showErrorMessage(i18n("Unable to create a temporary file."));
        m_tempInstallFile.reset();
        return;
    }

    // Ideally we copied the file into the proper location right away but
    // (for some reason) we determine the file name from the "Name" inside the file
    m_tempCopyJob = KIO::file_copy(url, QUrl::fromLocalFile(m_tempInstallFile->fileName()),
                                    -1, KIO::Overwrite);
    m_tempCopyJob->uiDelegate()->setAutoErrorHandlingEnabled(true);
    emit downloadingFileChanged();

    connect(m_tempCopyJob, &KIO::FileCopyJob::result, this, [this, url](KJob *job) {
        if (job->error() != KJob::NoError) {
            emit showErrorMessage(i18n("Unable to download the color scheme: %1", job->errorText()));
            return;
        }

        installSchemeFile(m_tempInstallFile->fileName());
        m_tempInstallFile.reset();
    });
    connect(m_tempCopyJob, &QObject::destroyed, this, &KCMColors::downloadingFileChanged);
}

void KCMColors::installSchemeFile(const QString &path)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);

    KConfigGroup group(config, "General");
    const QString name = group.readEntry("Name");

    if (name.isEmpty()) {
        emit showErrorMessage(i18n("This file is not a color scheme file."));
        return;
    }

    // Do not overwrite another scheme
    int increment = 0;
    QString newName = name;
    QString testpath;
    do {
        if (increment) {
            newName = name + QString::number(increment);
        }
        testpath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            QStringLiteral("color-schemes/%1.colors").arg(newName));
        increment++;
    } while (!testpath.isEmpty());

    QString newPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/color-schemes/");

    if (!QDir().mkpath(newPath)) {
        emit showErrorMessage(i18n("Failed to create 'color-scheme' data folder."));
        return;
    }

    newPath += newName + QLatin1String(".colors");

    if (!QFile::copy(path, newPath)) {
        emit showErrorMessage(i18n("Failed to copy color scheme into 'color-scheme' data folder."));
        return;
    }

    // Update name
    KSharedConfigPtr config2 = KSharedConfig::openConfig(newPath, KConfig::SimpleConfig);
    KConfigGroup group2(config2, "General");
    group2.writeEntry("Name", newName);
    config2->sync();

    m_model->load();

    const auto results = m_model->match(m_model->index(0, 0), ColorsModel::SchemeNameRole, newName);
    if (!results.isEmpty()) {
        m_model->setSelectedScheme(newName);
    }

    emit showSuccessMessage(i18n("Color scheme installed successfully."));
}

void KCMColors::editScheme(const QString &schemeName, QQuickItem *ctx)
{
    if (m_editDialogProcess) {
        return;
    }

    QModelIndex idx = m_model->index(m_model->indexOfScheme(schemeName), 0);

    m_editDialogProcess = new QProcess(this);
    connect(m_editDialogProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);

        const auto savedThemes = QString::fromUtf8(m_editDialogProcess->readAllStandardOutput()).split(QLatin1Char('\n'), QString::SkipEmptyParts);

        if (!savedThemes.isEmpty()) {
            m_model->load(); // would be cool to just reload/add the changed/new ones

            // If the currently active scheme was edited, consider settings dirty even if the scheme itself didn't change
            if (savedThemes.contains(colorsSettings()->colorScheme())) {
                m_activeSchemeEdited = true;
                settingsChanged();
            }

            m_model->setSelectedScheme(savedThemes.last());
        }

        m_editDialogProcess->deleteLater();
        m_editDialogProcess = nullptr;
    });

    QStringList args;
    args << idx.data(ColorsModel::SchemeNameRole).toString();
    if (idx.data(ColorsModel::RemovableRole).toBool()) {
        args << QStringLiteral("--overwrite");
    }

    if (ctx && ctx->window()) {
        // QQuickWidget, used for embedding QML KCMs, renders everything into an offscreen window
        // Qt is able to resolve this on its own when setting transient parents in-process.
        // However, since we pass the ID to an external process which has no idea of this
        // we need to resolve the actual window we end up showing in.
        if (QWindow *actualWindow = QQuickRenderControl::renderWindowFor(ctx->window())) {
            if (KWindowSystem::isPlatformX11()) {
                // TODO wayland: once we have foreign surface support
                args << QStringLiteral("--attach") << (QStringLiteral("x11:") + QString::number(actualWindow->winId()));
            }
        }
    }

    m_editDialogProcess->start(QStringLiteral("kcolorschemeeditor"), args);
}

bool KCMColors::isSaveNeeded() const
{
    return m_activeSchemeEdited || !m_model->match(m_model->index(0, 0), ColorsModel::PendingDeletionRole, true).isEmpty();
}


void KCMColors::load()
{
    ManagedConfigModule::load();
    m_model->load();

    m_config->markAsClean();
    m_config->reparseConfiguration();

    const QString schemeName = colorsSettings()->colorScheme();

    // If the scheme named in kdeglobals doesn't exist, show a warning and use default scheme
    if (m_model->indexOfScheme(schemeName) == -1) {
        m_model->setSelectedScheme(colorsSettings()->defaultColorSchemeValue());
        // These are normally synced but initially the model doesn't emit a change to avoid the
        // Apply button from being enabled without any user interaction. Sync manually here.
        m_filteredModel->setSelectedScheme(colorsSettings()->defaultColorSchemeValue());
        emit showSchemeNotInstalledWarning(schemeName);
    } else {
        m_model->setSelectedScheme(schemeName);
        m_filteredModel->setSelectedScheme(schemeName);
    }

    {
        KConfig cfg(QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals);
        KConfigGroup group(m_config, "General");
        group = KConfigGroup(&cfg, "X11");
        m_applyToAlien = group.readEntry("exportKDEColors", true);
    }

    // If need save is true at the end of load() function, it will stay disabled forever.
    // setSelectedScheme() call due to unexisting scheme name in kdeglobals will trigger a need to save.
    // this following call ensure the apply button will work properly.
    setNeedsSave(false);
}

void KCMColors::save()
{
    // We need to save the colors change first, to avoid a situation,
    // when we announced that the color scheme has changed, but
    // the colors themselves in the color scheme have not yet
    if (m_selectedSchemeDirty || m_activeSchemeEdited) {
        saveColors();
    }
    ManagedConfigModule::save();
    m_activeSchemeEdited = false;

    processPendingDeletions();
}

void KCMColors::saveColors()
{
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
        QStringLiteral("color-schemes/%1.colors").arg(m_model->selectedScheme()));

    // Using KConfig::SimpleConfig because otherwise Header colors won't be
    // rewritten when a new color scheme is loaded.
    KSharedConfigPtr config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);

    const QStringList colorSetGroupList{
        QStringLiteral("Colors:View"),
        QStringLiteral("Colors:Window"),
        QStringLiteral("Colors:Button"),
        QStringLiteral("Colors:Selection"),
        QStringLiteral("Colors:Tooltip"),
        QStringLiteral("Colors:Complementary"),
        QStringLiteral("Colors:Header")
    };

    const QList<KColorScheme> colorSchemes{
        KColorScheme(QPalette::Active, KColorScheme::View, config),
        KColorScheme(QPalette::Active, KColorScheme::Window, config),
        KColorScheme(QPalette::Active, KColorScheme::Button, config),
        KColorScheme(QPalette::Active, KColorScheme::Selection, config),
        KColorScheme(QPalette::Active, KColorScheme::Tooltip, config),
        KColorScheme(QPalette::Active, KColorScheme::Complementary, config),
        KColorScheme(QPalette::Active, KColorScheme::Header, config)
    };

    for (int i = 0; i < colorSchemes.length(); ++i) {
        KConfigGroup group(m_config, colorSetGroupList.value(i));
        group.writeEntry("BackgroundNormal", colorSchemes[i].background(KColorScheme::NormalBackground).color());
        group.writeEntry("BackgroundAlternate", colorSchemes[i].background(KColorScheme::AlternateBackground).color());
        group.writeEntry("ForegroundNormal", colorSchemes[i].foreground(KColorScheme::NormalText).color());
        group.writeEntry("ForegroundInactive", colorSchemes[i].foreground(KColorScheme::InactiveText).color());
        group.writeEntry("ForegroundActive", colorSchemes[i].foreground(KColorScheme::ActiveText).color());
        group.writeEntry("ForegroundLink", colorSchemes[i].foreground(KColorScheme::LinkText).color());
        group.writeEntry("ForegroundVisited", colorSchemes[i].foreground(KColorScheme::VisitedText).color());
        group.writeEntry("ForegroundNegative", colorSchemes[i].foreground(KColorScheme::NegativeText).color());
        group.writeEntry("ForegroundNeutral", colorSchemes[i].foreground(KColorScheme::NeutralText).color());
        group.writeEntry("ForegroundPositive", colorSchemes[i].foreground(KColorScheme::PositiveText).color());
        group.writeEntry("DecorationFocus", colorSchemes[i].decoration(KColorScheme::FocusColor).color());
        group.writeEntry("DecorationHover", colorSchemes[i].decoration(KColorScheme::HoverColor).color());
    }

    KConfigGroup groupWMTheme(config, "WM");
    KConfigGroup groupWMOut(m_config, "WM");
    KColorScheme inactiveHeaderColorScheme(QPalette::Inactive, KColorScheme::Header, config);

    const QStringList colorItemListWM{
        QStringLiteral("activeBackground"),
        QStringLiteral("activeForeground"),
        QStringLiteral("inactiveBackground"),
        QStringLiteral("inactiveForeground"),
        QStringLiteral("activeBlend"),
        QStringLiteral("inactiveBlend")
    };

    const QVector<QColor> defaultWMColors{
        colorSchemes[KColorScheme::Header].background().color(),
        colorSchemes[KColorScheme::Header].foreground().color(),
        inactiveHeaderColorScheme.background().color(),
        inactiveHeaderColorScheme.foreground().color(),
        colorSchemes[KColorScheme::Header].background().color(),
        inactiveHeaderColorScheme.background().color()
    };

    int i = 0;
    for (const QString &coloritem : colorItemListWM) {
        groupWMOut.writeEntry(coloritem, groupWMTheme.readEntry(coloritem, defaultWMColors.value(i)));
        ++i;
    }

    const QStringList groupNameList{
        QStringLiteral("ColorEffects:Inactive"),
        QStringLiteral("ColorEffects:Disabled")
    };

    const QStringList effectList{
        QStringLiteral("Enable"),
        QStringLiteral("ChangeSelectionColor"),
        QStringLiteral("IntensityEffect"),
        QStringLiteral("IntensityAmount"),
        QStringLiteral("ColorEffect"),
        QStringLiteral("ColorAmount"),
        QStringLiteral("Color"),
        QStringLiteral("ContrastEffect"),
        QStringLiteral("ContrastAmount")
    };

    for (const QString &groupName : groupNameList) {
        KConfigGroup groupEffectOut(m_config, groupName);
        KConfigGroup groupEffectTheme(config, groupName);

        for (const QString &effect : effectList) {
            groupEffectOut.writeEntry(effect, groupEffectTheme.readEntry(effect));
        }
    }

    m_config->sync();

    runRdb(KRdbExportQtColors | KRdbExportGtkTheme | (m_applyToAlien ? KRdbExportColors : 0));

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"),
                                                      QStringLiteral("org.kde.KGlobalSettings"),
                                                      QStringLiteral("notifyChange"));
    message.setArguments({
        0, //previous KGlobalSettings::PaletteChanged. This is now private API in khintsettings
        0  //unused in palette changed but needed for the DBus signature
    });
    QDBusConnection::sessionBus().send(message);

    m_selectedSchemeDirty = false;
}

void KCMColors::processPendingDeletions()
{
    const QStringList pendingDeletions = m_model->pendingDeletions();

    for (const QString &schemeName : pendingDeletions) {
        Q_ASSERT(schemeName != m_model->selectedScheme());

        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            QStringLiteral("color-schemes/%1.colors").arg(schemeName));

        auto *job = KIO::del(QUrl::fromLocalFile(path), KIO::HideProgressInfo);
        // needs to block for it to work on "OK" where the dialog (kcmshell) closes
        job->exec();
    }

    m_model->removeItemsPendingDeletion();
}

#include "colors.moc"
