/*
    SPDX-FileCopyrightText: 2009 Wang Hoi <zealot.hoi@gmail.com>
    SPDX-FileCopyrightText: 2011 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimpanelagent.h"
#include "impaneladaptor.h"

// Qt
#include <QByteArray>
#include <QDBusServiceWatcher>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

int PanelAgent::m_connectionIndex = 0;

PanelAgent::PanelAgent(QObject *parent)
    : QObject(parent)
    , m_adaptor(new ImpanelAdaptor(this))
    , m_adaptor2(new Impanel2Adaptor(this))
    , m_watcher(new QDBusServiceWatcher(this))
    , m_connection(QDBusConnection::connectToBus(QDBusConnection::SessionBus, QStringLiteral("kimpanel_bus_%0").arg(++m_connectionIndex)))
{
    m_watcher->setConnection(QDBusConnection::sessionBus());
    m_watcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    m_connection.registerObject(QStringLiteral("/org/kde/impanel"), this);
    m_connection.registerService(QStringLiteral("org.kde.impanel"));

    // directly connect to corresponding signal
    m_connection.connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("Enable"), this, SIGNAL(enable(bool)));
    m_connection.connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("ShowPreedit"), this, SIGNAL(showPreedit(bool)));
    m_connection.connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("ShowAux"), this, SIGNAL(showAux(bool)));
    m_connection
        .connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("ShowLookupTable"), this, SIGNAL(showLookupTable(bool)));
    m_connection.connect(QString(),
                         QString(),
                         QStringLiteral("org.kde.kimpanel.inputmethod"),
                         QStringLiteral("UpdateLookupTableCursor"),
                         this,
                         SIGNAL(updateLookupTableCursor(int)));

    // do some serialization
    m_connection.connect(QString(),
                         QString(),
                         QStringLiteral("org.kde.kimpanel.inputmethod"),
                         QStringLiteral("UpdateLookupTable"),
                         this,
                         SLOT(UpdateLookupTable(QStringList, QStringList, QStringList, bool, bool)));
    m_connection.connect(QString(),
                         QString(),
                         QStringLiteral("org.kde.kimpanel.inputmethod"),
                         QStringLiteral("UpdatePreeditCaret"),
                         this,
                         SIGNAL(updatePreeditCaret(int)));
    m_connection.connect(QString(),
                         QString(),
                         QStringLiteral("org.kde.kimpanel.inputmethod"),
                         QStringLiteral("UpdatePreeditText"),
                         this,
                         SLOT(UpdatePreeditText(QString, QString)));
    m_connection
        .connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("UpdateAux"), this, SLOT(UpdateAux(QString, QString)));
    m_connection.connect(QString(),
                         QString(),
                         QStringLiteral("org.kde.kimpanel.inputmethod"),
                         QStringLiteral("UpdateSpotLocation"),
                         this,
                         SIGNAL(updateSpotLocation(int, int)));
    m_connection.connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("UpdateScreen"), this, SLOT(UpdateScreen(int)));
    m_connection
        .connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("UpdateProperty"), this, SLOT(UpdateProperty(QString)));
    m_connection.connect(QString(),
                         QString(),
                         QStringLiteral("org.kde.kimpanel.inputmethod"),
                         QStringLiteral("RegisterProperties"),
                         this,
                         SLOT(RegisterProperties(QStringList)));
    m_connection.connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("ExecDialog"), this, SLOT(ExecDialog(QString)));
    m_connection.connect(QString(), QString(), QStringLiteral("org.kde.kimpanel.inputmethod"), QStringLiteral("ExecMenu"), this, SLOT(ExecMenu(QStringList)));

    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, &PanelAgent::serviceUnregistered);
}

PanelAgent::~PanelAgent()
{
    // destructor
    QDBusConnection::disconnectFromBus(m_connection.name());
}

void PanelAgent::serviceUnregistered(const QString &service)
{
    if (service == m_currentService) {
        m_watcher->setWatchedServices(QStringList());
        m_cachedProps.clear();
        m_currentService = QString();
        Q_EMIT showAux(false);
        Q_EMIT showPreedit(false);
        Q_EMIT showLookupTable(false);
        Q_EMIT registerProperties(QList<KimpanelProperty>());
    }
}

void PanelAgent::configure()
{
    Q_EMIT Configure();
}

void PanelAgent::lookupTablePageDown()
{
    Q_EMIT LookupTablePageDown();
}

void PanelAgent::lookupTablePageUp()
{
    Q_EMIT LookupTablePageUp();
}

void PanelAgent::movePreeditCaret(int pos)
{
    Q_EMIT MovePreeditCaret(pos);
}

void PanelAgent::triggerProperty(const QString &key)
{
    Q_EMIT TriggerProperty(key);
}

void PanelAgent::selectCandidate(int idx)
{
    Q_EMIT SelectCandidate(idx);
}

static QList<TextAttribute> String2AttrList(const QString &str)
{
    QList<TextAttribute> result;
    if (str.isEmpty()) {
        return result;
    }
    foreach (const QString &s, str.split(QLatin1Char(';'))) {
        TextAttribute attr;
        QStringList list = s.split(QLatin1Char(':'));
        if (list.size() < 4)
            continue;
        switch (list.at(0).toInt()) {
        case 0:
            attr.type = TextAttribute::None;
            break;
        case 1:
            attr.type = TextAttribute::Decorate;
            break;
        case 2:
            attr.type = TextAttribute::Foreground;
            break;
        case 3:
            attr.type = TextAttribute::Background;
            break;
        default:
            attr.type = TextAttribute::None;
        }
        attr.start = list.at(1).toInt();
        attr.length = list.at(2).toInt();
        attr.value = list.at(3).toInt();
        result << attr;
    }
    return result;
}

static KimpanelProperty String2Property(const QString &str)
{
    KimpanelProperty result;

    QStringList list = str.split(QLatin1Char(':'));

    if (list.size() < 4)
        return result;

    result.key = list.at(0);
    result.label = list.at(1);
    result.icon = list.at(2);
    result.tip = list.at(3);
    result.hint = list.size() > 4 ? list.at(4) : QString();

    return result;
}

static KimpanelLookupTable Args2LookupTable(const QStringList &labels, const QStringList &candis, const QStringList &attrs, bool has_prev, bool has_next)
{
    Q_ASSERT(labels.size() == candis.size());
    Q_ASSERT(labels.size() == attrs.size());

    KimpanelLookupTable result;

    for (int i = 0; i < labels.size(); i++) {
        KimpanelLookupTable::Entry entry;

        entry.label = labels.at(i);
        entry.text = candis.at(i);
        entry.attr = String2AttrList(attrs.at(i));

        result.entries << entry;
    }

    result.has_prev = has_prev;
    result.has_next = has_next;
    return result;
}

void PanelAgent::created()
{
    Q_EMIT PanelCreated();
    Q_EMIT PanelCreated2();
}

void PanelAgent::exit()
{
    Q_EMIT Exit();
}

void PanelAgent::reloadConfig()
{
    Q_EMIT ReloadConfig();
}

void PanelAgent::UpdateLookupTable(const QStringList &labels, const QStringList &candis, const QStringList &attrlists, bool has_prev, bool has_next)
{
    Q_EMIT updateLookupTable(Args2LookupTable(labels, candis, attrlists, has_prev, has_next));
}

void PanelAgent::UpdatePreeditText(const QString &text, const QString &attr)
{
    Q_EMIT updatePreeditText(text, String2AttrList(attr));
}

void PanelAgent::UpdateAux(const QString &text, const QString &attr)
{
    Q_EMIT updateAux(text, String2AttrList(attr));
}

void PanelAgent::UpdateScreen(int screen_id)
{
    Q_UNUSED(screen_id);
}

void PanelAgent::UpdateProperty(const QString &prop)
{
    Q_EMIT updateProperty(String2Property(prop));
}

void PanelAgent::RegisterProperties(const QStringList &props)
{
    const QDBusMessage &msg = message();
    if (msg.service() != m_currentService) {
        m_watcher->removeWatchedService(m_currentService);
        if (m_currentService.isEmpty()) {
            Q_EMIT PanelRegistered();
        }
        m_currentService = msg.service();
        m_watcher->addWatchedService(m_currentService);
    }
    if (m_cachedProps != props) {
        m_cachedProps = props;
        QList<KimpanelProperty> list;
        foreach (const QString &prop, props) {
            list << String2Property(prop);
        }

        Q_EMIT registerProperties(list);
    }
}

void PanelAgent::ExecDialog(const QString &prop)
{
    Q_EMIT execDialog(String2Property(prop));
}

void PanelAgent::ExecMenu(const QStringList &entries)
{
    QList<KimpanelProperty> list;
    foreach (const QString &entry, entries) {
        list << String2Property(entry);
    }

    Q_EMIT execMenu(list);
}

void PanelAgent::SetSpotRect(int x, int y, int w, int h)
{
    Q_EMIT updateSpotRect(x, y, w, h);
}

void PanelAgent::SetLookupTable(const QStringList &labels,
                                const QStringList &candis,
                                const QStringList &attrlists,
                                bool hasPrev,
                                bool hasNext,
                                int cursor,
                                int layout)
{
    Q_EMIT updateLookupTableFull(Args2LookupTable(labels, candis, attrlists, hasPrev, hasNext), cursor, layout);
}
