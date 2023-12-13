/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KWindowSystem>
#include <QAbstractListModel>
#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QIcon>
#include <QList>
#include <QLocale>
#include <QPainter>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QQuickWindow>
#include <QSessionManager>

#include "config-workspace.h"

class EngineWatcher : public QObject
{
public:
    EngineWatcher(QQmlApplicationEngine *engine)
        : QObject(engine)
    {
        connect(engine, &QQmlApplicationEngine::objectCreated, this, &EngineWatcher::integrateObject);
    }

    void integrateObject(QObject *object)
    {
        QWindow *window = qobject_cast<QWindow *>(object);

        auto conf = KSharedConfig::openConfig();
        KWindowConfig::restoreWindowSize(window, conf->group(QStringLiteral("Window")));

        object->installEventFilter(this);
    }

    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (event->type() == QEvent::Close) {
            QWindow *window = qobject_cast<QWindow *>(object);

            auto conf = KSharedConfig::openConfig();
            auto group = conf->group(QStringLiteral("Window"));
            KWindowConfig::saveWindowSize(window, group);
            group.sync();
        }
        return false;
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-emoticons")));
    KCrash::initialize();

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("org.kde.plasma.emojier"));

    KAboutData about(QStringLiteral("plasma.emojier"),
                     i18n("Emoji Selector"),
                     QStringLiteral(WORKSPACE_VERSION_STRING),
                     i18n("Emoji Selector"),
                     KAboutLicense::GPL,
                     i18n("(C) 2019 Aleix Pol i Gonzalez"));
    about.addAuthor(QStringLiteral("Aleix Pol i Gonzalez"), QString(), QStringLiteral("aleixpol@kde.org"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    //     about.setProductName("");
    about.setProgramLogo(app.windowIcon());
    KAboutData::setApplicationData(about);

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    KDBusService::StartupOptions startup = {};
    {
        QCommandLineParser parser;

        QCommandLineOption replaceOption({QStringLiteral("replace")}, i18n("Replace an existing instance"));
        parser.addOption(replaceOption);
        about.setupCommandLine(&parser);
        parser.process(app);
        about.processCommandLine(&parser);

        if (parser.isSet(replaceOption)) {
            startup |= KDBusService::Replace;
        }
    }

    KDBusService *service = new KDBusService(KDBusService::Unique | startup, &app);

    QQmlApplicationEngine engine;
    new EngineWatcher(&engine);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.loadFromModule("org.kde.plasma.emoji.app", "Emojier");

    QObject::connect(service, &KDBusService::activateRequested, &engine, [&engine](const QStringList & /*arguments*/, const QString & /*workingDirectory*/) {
        for (QObject *object : engine.rootObjects()) {
            auto w = qobject_cast<QQuickWindow *>(object);
            if (!w)
                continue;

            KWindowSystem::updateStartupId(w);
            KWindowSystem::activateWindow(w);

            w->setVisible(true);
            w->raise();
        }
    });

    return app.exec();
}
