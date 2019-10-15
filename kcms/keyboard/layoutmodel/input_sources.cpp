#include "input_sources.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QDebug>

InputSources InputSources::m_self{};

InputSources* InputSources::self()
{
    return &m_self;
}

int InputSources::currentSource() const
{
    if (m_isFcitxRegistered) {
        return Sources::FcitxSource;
    }
    else {
        return Sources::XkbSource;
    }
}

void InputSources::serviceRegistered(const QString &name)
{
    // FIXME: why is this called twice??
    qDebug() << name << "registered" << this;
    m_isFcitxRegistered = true;
    emit currentSourceChanged(Sources::FcitxSource);
}

void InputSources::serviceUnregistered(const QString &name)
{
    qDebug() << name << "unregistered" << this;
    m_isFcitxRegistered = false;
    emit currentSourceChanged(Sources::XkbSource);
}

InputSources::InputSources()
{
    // This can cause fcitx to "wake up"
    QDBusInterface interface("org.fcitx.Fcitx", "/", "");
    m_isFcitxRegistered = interface.isValid();

    m_fcitxWatcher = new QDBusServiceWatcher(
        "org.fcitx.Fcitx",
        QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
        this);

    QObject::connect(m_fcitxWatcher, &QDBusServiceWatcher::serviceRegistered,
        this, &InputSources::serviceRegistered);
    QObject::connect(m_fcitxWatcher, &QDBusServiceWatcher::serviceUnregistered,
        this, &InputSources::serviceUnregistered);
}
