/* This file is part of the KDE Project
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>

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
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <QStandardPaths>
#include <QProcess>
#include <QQuickView>

#include <QVBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QQmlContext>
#include <QDir>

#include <KLocalizedString>
#include <Plasma/PluginLoader>

#include <KNewStuff3/KNS3/DownloadDialog>

K_PLUGIN_FACTORY_WITH_JSON(KCMSplashScreenFactory, "kcm_splashscreen.json", registerPlugin<KCMSplashScreen>();)

KCMSplashScreen::KCMSplashScreen(QObject* parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_config(QStringLiteral("ksplashrc"))
    , m_configGroup(m_config.group("KSplash"))
{
    qmlRegisterType<QStandardItemModel>();
    KAboutData* about = new KAboutData(QStringLiteral("kcm_splashscreen"), i18n("Splash Screen"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Marco Martin"), QString(), QStringLiteral("mart@kde.org"));
    setAboutData(about);
    setButtons(Help | Apply | Default);

    m_model = new QStandardItemModel(this);
    QHash<int, QByteArray> roles = m_model->roleNames();
    roles[PluginNameRole] = "pluginName";
    roles[ScreenhotRole] = "screenshot";
    roles[DescriptionRole] = "description";
    m_model->setItemRoleNames(roles);
    loadModel();
}

QList<Plasma::Package> KCMSplashScreen::availablePackages(const QString &component)
{
    QList<Plasma::Package> packages;
    QStringList paths;
    const QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (const QString &path : dataPaths) {
        QDir dir(path + QStringLiteral("/plasma/look-and-feel"));
        paths << dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    for (const QString &path : paths) {
        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
        pkg.setPath(path);
        pkg.setFallbackPackage(Plasma::Package());
        if (component.isEmpty() || !pkg.filePath(component.toUtf8()).isEmpty()) {
            packages << pkg;
        }
    }

    return packages;
}

QStandardItemModel *KCMSplashScreen::splashModel()
{
    return m_model;
}

QString KCMSplashScreen::selectedPlugin() const
{
    return m_selectedPlugin;
}

void KCMSplashScreen::setSelectedPlugin(const QString &plugin)
{
    if (m_selectedPlugin == plugin) {
        return;
    }

    if (!m_selectedPlugin.isEmpty()) {
        setNeedsSave(true);
    }
    m_selectedPlugin = plugin;
    emit selectedPluginChanged();
    emit selectedPluginIndexChanged();
}

void KCMSplashScreen::getNewClicked()
{
    KNS3::DownloadDialog dialog("ksplash.knsrc", nullptr);
    if (dialog.exec()) {
        KNS3::Entry::List list = dialog.changedEntries();
        if (!list.isEmpty()) {
            loadModel();
        }
    }
}

void KCMSplashScreen::loadModel()
{
    m_model->clear();

    const QList<Plasma::Package> pkgs = availablePackages(QStringLiteral("splashmainscript"));
    for (const Plasma::Package &pkg : pkgs) {
        QStandardItem* row = new QStandardItem(pkg.metadata().name());
        row->setData(pkg.metadata().pluginName(), PluginNameRole);
        row->setData(pkg.filePath("previews", QStringLiteral("splash.png")), ScreenhotRole);
        row->setData(pkg.metadata().comment(), DescriptionRole);
        m_model->appendRow(row);
    }
    m_model->sort(0 /*column*/);

    QStandardItem* row = new QStandardItem(i18n("None"));
    row->setData("None", PluginNameRole);
    row->setData(i18n("No splash screen will be shown"), DescriptionRole);
    m_model->insertRow(0, row);

    emit selectedPluginIndexChanged();
}

void KCMSplashScreen::load()
{
    m_package = Plasma::PluginLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), "KDE");
    const QString packageName = cg.readEntry("LookAndFeelPackage", QString());
    if (!packageName.isEmpty()) {
        m_package.setPath(packageName);
    }

    QString currentPlugin = m_configGroup.readEntry("Theme", QString());
    if (currentPlugin.isEmpty()) {
        currentPlugin = m_package.metadata().pluginName();
    }
    setSelectedPlugin(currentPlugin);
    
    setNeedsSave(false);
}


void KCMSplashScreen::save()
{
    if (m_selectedPlugin.isEmpty()) {
        return;
    } else if (m_selectedPlugin == QLatin1String("None")) {
        m_configGroup.writeEntry("Theme", m_selectedPlugin);
        m_configGroup.writeEntry("Engine", "none");
    } else {
        m_configGroup.writeEntry("Theme", m_selectedPlugin);
        m_configGroup.writeEntry("Engine", "KSplashQML");
    }

    m_configGroup.sync();
    setNeedsSave(false);
}

void KCMSplashScreen::defaults()
{
    if (!m_package.metadata().isValid()) {
        return;
    }
    setSelectedPlugin(m_package.metadata().pluginName());
}

int KCMSplashScreen::selectedPluginIndex() const
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->data(m_model->index(i, 0), PluginNameRole).toString() == m_selectedPlugin) {
            return i;
        }
    }
    return -1;
}

bool KCMSplashScreen::testing() const
{
    return m_testProcess;
}

void KCMSplashScreen::test(const QString &plugin)
{
    if (plugin.isEmpty() || plugin == QLatin1String("None") || m_testProcess) {
        return;
    }

    m_testProcess = new QProcess(this);
    connect(m_testProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        Q_UNUSED(error);
        emit testingFailed();
    });
    connect(m_testProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);

        m_testProcess->deleteLater();
        m_testProcess = nullptr;
        emit testingChanged();
    });

    emit testingChanged();
    m_testProcess->start(QStringLiteral("ksplashqml"), {plugin, QStringLiteral("--test")});
}

#include "kcm.moc"
