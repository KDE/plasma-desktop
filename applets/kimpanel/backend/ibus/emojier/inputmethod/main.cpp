/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "inputbracket.h"
#include "inputmethod.h"
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

using namespace KWayland::Client;

class EmojiIM : public QObject
{
public:
    EmojiIM()
        : m_engine(new QQmlApplicationEngine(QStringLiteral("qrc:/keyboard.qml"), this))
    {
        ConnectionThread *connection = ConnectionThread::fromApplication(this);
        if (!connection) {
            qDebug() << "failed to get the Connection from Qt, Wayland remote input will not work";
            return;
        }
        Registry *registry = new Registry(this);
        registry->create(connection);
        connect(registry, &Registry::interfaceAnnounced, this, [this, registry](const QByteArray &iface, quint32 name, quint32 version) {
            if (iface == "zwp_input_method_v1") {
                m_inputMethod = new InputMethod(*registry, name, version);
                m_name = name;
                for (auto obj : m_engine->rootObjects()) {
                    bool b = obj->setProperty("inputMethod", QVariant::fromValue<QObject *>(m_inputMethod));
                    Q_ASSERT(b);
                }
            }
        });
        connect(registry, &Registry::interfaceRemoved, this, [this](uint32_t name) {
            if (name == m_name) {
                delete m_inputMethod;
                m_inputMethod = nullptr;
                m_name = 0;
            }
        });
        registry->setup();
    }

private:
    QQmlApplicationEngine *const m_engine;
    InputMethod *m_inputMethod = nullptr;
    uint32_t m_name = 0;
};

int main(int argc, char *argv[])
{
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "inputpanel-shell");
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    qmlRegisterAnonymousType<InputMethod>("org.kde.plasma.emoji.im", 1);
    qmlRegisterType<InputBracket>("org.kde.plasma.emoji.im", 1, 0, "InputBracket");
    EmojiIM service;

    return app.exec();
}
