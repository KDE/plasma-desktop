/*
    SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kimpanel.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QProcess>

class IBusPanelLauncher : public QCoreApplication
{
    Q_OBJECT
public:
    IBusPanelLauncher(int &argc, char *argv[])
        : QCoreApplication(argc, argv)
        , m_watcher(new QDBusServiceWatcher(this))
    {
        m_watcher->setConnection(QDBusConnection::sessionBus());
        QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    }

public Q_SLOTS:
    void init()
    {
        // already a launcher running
        if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.impanel.IBusPanelLauncher"))) {
            quit();
            return;
        }
        // Check if applet is still there.
        connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, &IBusPanelLauncher::serviceUnregistered);
        connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, &IBusPanelLauncher::serviceRegistered);
        m_watcher->addWatchedService(QStringLiteral("org.kde.impanel"));
        m_watcher->addWatchedService(QStringLiteral("org.freedesktop.IBus"));

        // check if panel is already created
        QDBusConnection::sessionBus().connect(QStringLiteral("org.kde.impanel"),
                                              QStringLiteral("/org/kde/impanel"),
                                              QStringLiteral("org.kde.impanel2"),
                                              QStringLiteral("PanelRegistered"),
                                              this,
                                              SLOT(quit()));
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.impanel"))) {
            quit();
            return;
        }
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.freedesktop.IBus"))) {
            serviceRegistered(QStringLiteral("org.freedesktop.IBus"));
        }
    }

    void serviceRegistered(const QString &service)
    {
        if (service == QLatin1String{"org.freedesktop.IBus"}) {
            launchIBusPanel();
        }
    }

    void serviceUnregistered(const QString &service)
    {
        if (service == QLatin1String{"org.kde.impanel"}) {
            quit();
        }
    }

private:
    void launchIBusPanel()
    {
        const QString panelPath = QStringLiteral(KIMPANEL_LIBEXEC_DIR "/kimpanel-ibus-panel");
        QProcess::startDetached(panelPath, QStringList());
        quit();
    }

private:
    QDBusServiceWatcher *m_watcher;
};

int main(int argc, char *argv[])
{
    IBusPanelLauncher app(argc, argv);

    return app.exec();
}

#include "launcher.moc"
