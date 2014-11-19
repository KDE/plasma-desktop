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
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtQml>
#include <QQmlContext>
#include <QDir>

#include <KLocalizedString>
#include <Plasma/PluginLoader>

K_PLUGIN_FACTORY(KCMSplashScreenFactory, registerPlugin<KCMSplashScreen>();)

KCMSplashScreen::KCMSplashScreen(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , m_config("ksplashrc")
    , m_configGroup(m_config.group("KSplash"))
{
    qmlRegisterType<QStandardItemModel>();
    KAboutData* about = new KAboutData("kcm_splashscreen", i18n("Configure Splash screen details"),
                                       "0.1", QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Marco Martin"), QString(), "mart@kde.org");
    setAboutData(about);
    setButtons(Help | Apply | Default);

    m_model = new QStandardItemModel(this);
    QHash<int, QByteArray> roles = m_model->roleNames();
    roles[PluginNameRole] = "pluginName";
    roles[ScreenhotRole] = "screenshot";
    m_model->setItemRoleNames(roles);
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_quickView = new QQuickView();
    QWidget *widget = QWidget::createWindowContainer(m_quickView, this);
    m_quickView->setResizeMode(QQuickView::SizeRootObjectToView);
    Plasma::Package package = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    package.setDefaultPackageRoot("plasma/kcms");
    package.setPath("kcm_splashscreen");
    m_quickView->rootContext()->setContextProperty("kcm", this);
    m_quickView->setSource(QUrl::fromLocalFile(package.filePath("mainscript")));
    setMinimumHeight(m_quickView->initialSize().height());

    layout->addWidget(widget);
}

QList<Plasma::Package> KCMSplashScreen::availablePackages(const QString &component)
{
    QList<Plasma::Package> packages;
    QStringList paths;
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (const QString &path : dataPaths) {
        QDir dir(path + "/plasma/look-and-feel");
        paths << dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    for (const QString &path : paths) {
        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
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

    m_selectedPlugin = plugin;
    emit selectedPluginChanged();
    changed();
}

void KCMSplashScreen::load()
{
    m_package = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
    KConfigGroup cg(KSharedConfig::openConfig("kdeglobals"), "KDE");
    const QString packageName = cg.readEntry("LookAndFeelPackage", QString());
    if (!packageName.isEmpty()) {
        m_package.setPath(packageName);
    }

    QString currentPlugin = m_configGroup.readEntry("Theme", QString());
    if (currentPlugin.isEmpty()) {
        currentPlugin = m_package.metadata().pluginName();
    }
    setSelectedPlugin(currentPlugin);

    m_model->clear();

    QStandardItem* row = new QStandardItem(i18n("None"));
    row->setData("None", PluginNameRole);
    m_model->appendRow(row);

    const QList<Plasma::Package> pkgs = availablePackages("splashmainscript");
    for (const Plasma::Package &pkg : pkgs) {
        QStandardItem* row = new QStandardItem(pkg.metadata().name());
        row->setData(pkg.metadata().pluginName(), PluginNameRole);
        row->setData(pkg.filePath("previews", "splash.png"), ScreenhotRole);
        m_model->appendRow(row);
    }
}


void KCMSplashScreen::save()
{
    if (m_selectedPlugin.isEmpty()) {
        return;
    } else if (m_selectedPlugin == "None") {
        m_configGroup.writeEntry("Theme", m_selectedPlugin);
        m_configGroup.writeEntry("Engine", "none");
    } else {
        m_configGroup.writeEntry("Theme", m_selectedPlugin);
        m_configGroup.writeEntry("Engine", "KSplashQML");
    }

    m_configGroup.sync();
}

void KCMSplashScreen::defaults()
{
    if (!m_package.metadata().isValid()) {
        return;
    }
    setSelectedPlugin(m_package.metadata().pluginName());
}

void KCMSplashScreen::test(const QString &plugin)
{
    if (plugin.isEmpty() || plugin == "None") {
        return;
    }

    QProcess proc;
    QStringList arguments;
    arguments << plugin << "--test";
    if (proc.execute("ksplashqml", arguments)) {
        QMessageBox::critical(this, i18n("Error"), i18n("Failed to successfully test the splash screen."));
    }
}

#include "kcm.moc"
