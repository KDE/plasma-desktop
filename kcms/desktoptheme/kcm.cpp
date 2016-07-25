/* This file is part of the KDE Project
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcm.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KDesktopFile>

#include <Plasma/Theme>
#include <Plasma/Svg>

#include <QDebug>
#include <QProcess>
#include <QQuickItem>
#include <QStandardPaths>
#include <QStandardItemModel>

#include <KNewStuff3/KNS3/DownloadDialog>

Q_LOGGING_CATEGORY(KCM_DESKTOP_THEME, "kcm_desktoptheme")

K_PLUGIN_FACTORY_WITH_JSON(KCMDesktopThemeFactory, "kcm_desktoptheme.json", registerPlugin<KCMDesktopTheme>();)

KCMDesktopTheme::KCMDesktopTheme(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_defaultTheme(new Plasma::Theme(this))
{
    //This flag seems to be needed in order for QQuickWidget to work
    //see https://bugreports.qt-project.org/browse/QTBUG-40765
    //also, it seems to work only if set in the kcm, not in the systemsettings' main
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    qmlRegisterType<QStandardItemModel>();

    KAboutData* about = new KAboutData(QStringLiteral("kcm_desktoptheme"), i18n("Configure Desktop Theme"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("David Rosca"), QString(), QStringLiteral("nowrep@gmail.com"));
    setAboutData(about);
    setButtons(Apply | Default | Help);

    m_model = new QStandardItemModel(this);
    QHash<int, QByteArray> roles = m_model->roleNames();
    roles[PluginNameRole] = QByteArrayLiteral("pluginName");
    roles[ThemeNameRole] = QByteArrayLiteral("themeName");
    roles[IsLocalRole] = QByteArrayLiteral("isLocal");
    m_model->setItemRoleNames(roles);
}

KCMDesktopTheme::~KCMDesktopTheme()
{
    delete m_defaultTheme;
}

QStandardItemModel *KCMDesktopTheme::desktopThemeModel() const
{
    return m_model;
}

QString KCMDesktopTheme::selectedPlugin() const
{
    return m_selectedPlugin;
}

void KCMDesktopTheme::setSelectedPlugin(const QString &plugin)
{
    if (m_selectedPlugin == plugin) {
        return;
    }
    m_selectedPlugin = plugin;
    Q_EMIT selectedPluginChanged(m_selectedPlugin);
    updateNeedsSave();
}

void KCMDesktopTheme::getNewThemes()
{
    KNS3::DownloadDialog *dialog = new KNS3::DownloadDialog(QStringLiteral("plasma-themes.knsrc"));
    dialog->open();

    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        if (!dialog->changedEntries().isEmpty()) {
            load();
            delete dialog;
        }
    });
}

void KCMDesktopTheme::installThemeFromFile(const QUrl &file)
{
    qCDebug(KCM_DESKTOP_THEME) << "Installing ... " << file;

    QString program = QStringLiteral("plasmapkg2");
    QStringList arguments;
    arguments << QStringLiteral("-t") << QStringLiteral("theme") << QStringLiteral("-i") << file.toLocalFile();

    qCDebug(KCM_DESKTOP_THEME) << program << arguments.join(QStringLiteral(" "));
    QProcess *myProcess = new QProcess(this);
    connect(myProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);
                if (exitCode == 0) {
                    qCDebug(KCM_DESKTOP_THEME) << "Theme installed successfully :)";
                    load();
                    Q_EMIT showInfoMessage(i18n("Theme installed successfully."));
                } else {
                    qCWarning(KCM_DESKTOP_THEME) << "Theme installation failed." << exitCode;
                    Q_EMIT showInfoMessage(i18n("Theme installation failed."));
                }
            });

    connect(myProcess, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this, [this](QProcess::ProcessError e) {
                qCWarning(KCM_DESKTOP_THEME) << "Theme installation failed: " << e;
                Q_EMIT showInfoMessage(i18n("Theme installation failed."));
            });

    myProcess->start(program, arguments);
}

void KCMDesktopTheme::removeTheme(const QString &name)
{
    Q_ASSERT(!m_pendingRemoval.contains(name));
    Q_ASSERT(!m_model->findItems(name).isEmpty());

    m_pendingRemoval.append(name);
    m_model->removeRow(m_model->findItems(name).at(0)->row());

    updateNeedsSave();
}

void KCMDesktopTheme::applyPlasmaTheme(QQuickItem *item, const QString &themeName)
{
    if (!item) {
        return;
    }

    Plasma::Theme *theme = m_themes[themeName];
    if (!theme) {
        theme = new Plasma::Theme(themeName, this);
        m_themes[themeName] = theme;
    }

    Q_FOREACH (Plasma::Svg *svg, item->findChildren<Plasma::Svg*>()) {
        svg->setTheme(theme);
        svg->setUsingRenderingCache(false);
    }
}

void KCMDesktopTheme::load()
{
    m_pendingRemoval.clear();

    // Get all desktop themes
    QStringList themes;
    const QStringList &packs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/desktoptheme"), QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &ppath, packs) {
        const QDir cd(ppath);
        const QStringList &entries = cd.entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &pack, entries) {
            const QString _metadata = ppath + QLatin1Char('/') + pack + QStringLiteral("/metadata.desktop");
            if (QFile::exists(_metadata)) {
                themes << _metadata;
            }
        }
    }

    m_model->clear();

    Q_FOREACH (const QString &theme, themes) {
        int themeSepIndex = theme.lastIndexOf('/', -1);
        const QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        const QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);

        KDesktopFile df(theme);

        if (df.noDisplay()) {
            continue;
        }

        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }
        const bool isLocal = QFileInfo(theme).isWritable();

        if (m_model->findItems(packageName).isEmpty()) {
            QStandardItem *item = new QStandardItem;
            item->setText(packageName);
            item->setData(packageName, PluginNameRole);
            item->setData(name, ThemeNameRole);
            item->setData(isLocal, IsLocalRole);
            m_model->appendRow(item);
        }
    }

    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("plasmarc")), "Theme");
    setSelectedPlugin(cg.readEntry("name", m_defaultTheme->themeName()));

    updateNeedsSave();
}

void KCMDesktopTheme::save()
{
    if (m_defaultTheme->themeName() == m_selectedPlugin) {
        return;
    }

    m_defaultTheme->setThemeName(m_selectedPlugin);
    removeThemes();
    updateNeedsSave();
}

void KCMDesktopTheme::defaults()
{
    setSelectedPlugin(QStringLiteral("default"));
}

void KCMDesktopTheme::updateNeedsSave()
{
    setNeedsSave(!m_pendingRemoval.isEmpty() || m_selectedPlugin != m_defaultTheme->themeName());
}

void KCMDesktopTheme::removeThemes()
{
    const QString program = QStringLiteral("plasmapkg2");

    Q_FOREACH (const QString &name, m_pendingRemoval) {
        const QStringList arguments = {QStringLiteral("-t"), QStringLiteral("theme"), QStringLiteral("-r"), name};
        qCDebug(KCM_DESKTOP_THEME) << program << arguments.join(QStringLiteral(" "));
        QProcess *process = new QProcess(this);
        connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                    Q_UNUSED(exitStatus);
                    if (exitCode == 0) {
                        qCDebug(KCM_DESKTOP_THEME) << "Theme removed successfully :)";
                        load();
                    } else {
                        qCWarning(KCM_DESKTOP_THEME) << "Theme removal failed." << exitCode;
                        Q_EMIT showInfoMessage(i18n("Theme removal failed."));
                    }
                    process->deleteLater();
                });

        connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
                this, [this, process](QProcess::ProcessError e) {
                    qCWarning(KCM_DESKTOP_THEME) << "Theme removal failed: " << e;
                    Q_EMIT showInfoMessage(i18n("Theme removal failed."));
                    process->deleteLater();
                });

        process->start(program, arguments);
    }
}

#include "kcm.moc"
