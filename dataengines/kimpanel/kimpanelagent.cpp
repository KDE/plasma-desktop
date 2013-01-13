/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kimpanelagent.h"
#include "impaneladaptor.h"

// Qt
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QDBusServiceWatcher>

PanelAgent::PanelAgent(QObject *parent)
    : QObject(parent)
    ,adaptor(new ImpanelAdaptor(this))
    ,adaptor2(new Impanel2Adaptor(this))
    ,watcher(new QDBusServiceWatcher(this))
{
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    QDBusConnection::connectToBus(QDBusConnection::SessionBus, "kimpanel_bus").registerObject("/org/kde/impanel", this);
    QDBusConnection::connectToBus(QDBusConnection::SessionBus, "kimpanel_bus").registerService("org.kde.impanel");

    // directly connect to corresponding signal
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "Enable", this, SIGNAL(enable(bool)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "ShowPreedit", this, SIGNAL(showPreedit(bool)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "ShowAux", this, SIGNAL(showAux(bool)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "ShowLookupTable", this, SIGNAL(showLookupTable(bool)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdateLookupTableCursor", this, SIGNAL(updateLookupTableCursor(int)));

    // do some serialization
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdateLookupTable", this, SLOT(UpdateLookupTable(QStringList,
                                                    QStringList, QStringList, bool, bool)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdatePreeditCaret", this, SIGNAL(updatePreeditCaret(int)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdatePreeditText", this, SLOT(UpdatePreeditText(QString, QString)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdateAux", this, SLOT(UpdateAux(QString, QString)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdateSpotLocation", this, SIGNAL(updateSpotLocation(int, int)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdateScreen", this, SLOT(UpdateScreen(int)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "UpdateProperty", this, SLOT(UpdateProperty(QString)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "RegisterProperties", this, SLOT(RegisterProperties(QStringList)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "ExecDialog", this, SLOT(ExecDialog(QString)));
    QDBusConnection("kimpanel_bus").connect("", "", "org.kde.kimpanel.inputmethod",
                                            "ExecMenu", this, SLOT(ExecMenu(QStringList)));

    connect(watcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(serviceUnregistered(QString)));
}

PanelAgent::~PanelAgent()
{
    // destructor
    QDBusConnection::disconnectFromBus("kimpanel_bus");
}

void PanelAgent::serviceUnregistered(const QString& service)
{
    if (service == m_currentService) {
        watcher->setWatchedServices(QStringList());
        cached_props.clear();
        m_currentService = QString();
        emit showAux(false);
        emit showPreedit(false);
        emit showLookupTable(false);
        emit registerProperties(QList<KimpanelProperty>());
    }
}

void PanelAgent::configure()
{
    emit Configure();
}

void PanelAgent::lookupTablePageDown()
{
    emit LookupTablePageDown();
}

void PanelAgent::lookupTablePageUp()
{
    emit LookupTablePageUp();
}

void PanelAgent::movePreeditCaret(int pos)
{
    emit MovePreeditCaret(pos);
}

void PanelAgent::triggerProperty(const QString& key)
{
    emit TriggerProperty(key);
}

void PanelAgent::selectCandidate(int idx)
{
    emit SelectCandidate(idx);
}


static QList<TextAttribute> String2AttrList(const QString &str)
{
    QList<TextAttribute> result;
    if (str.isEmpty()) {
        return result;
    }
    foreach(const QString & s, str.split(';')) {
        TextAttribute attr;
        QStringList list = s.split(':');
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

    QStringList list = str.split(':');

    if (list.size() < 4)
        return result;

    result.key = list.at(0);
    result.label = list.at(1);
    result.icon = list.at(2);
    result.tip = list.at(3);
//X     {
//X         result.state = KimpanelProperty::None;
//X         int n = list.at(4).toInt();
//X         if (n & KimpanelProperty::Active)
//X             result.state &= KimpanelProperty::Active;
//X         if (n & KimpanelProperty::Visible)
//X             result.state &= KimpanelProperty::Visible;
//X     }

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
    emit PanelCreated();
    emit PanelCreated2();
}

void PanelAgent::exit()
{
    emit Exit();
}

void PanelAgent::reloadConfig()
{
    emit ReloadConfig();
}

void PanelAgent::UpdateLookupTable(const QStringList &labels,
                                   const QStringList &candis,
                                   const QStringList &attrlists,
                                   bool has_prev, bool has_next)
{
    emit updateLookupTable(Args2LookupTable(labels, candis, attrlists, has_prev, has_next));
}

void PanelAgent::UpdatePreeditText(const QString &text, const QString &attr)
{
    emit updatePreeditText(text, String2AttrList(attr));
}

void PanelAgent::UpdateAux(const QString &text, const QString &attr)
{
    emit updateAux(text, String2AttrList(attr));
}

void PanelAgent::UpdateScreen(int screen_id)
{
    Q_UNUSED(screen_id);
}

void PanelAgent::UpdateProperty(const QString &prop)
{
    emit updateProperty(String2Property(prop));
}

void PanelAgent::RegisterProperties(const QStringList &props)
{
    const QDBusMessage& msg = message();
    if (msg.service() != m_currentService) {
        watcher->removeWatchedService(m_currentService);
        m_currentService = msg.service();
        watcher->addWatchedService(m_currentService);
    }
    if (cached_props != props) {
        cached_props = props;
        QList<KimpanelProperty> list;
        foreach(const QString & prop, props) {
            list << String2Property(prop);
        }

        emit registerProperties(list);
    }
}

void PanelAgent::ExecDialog(const QString &prop)
{
    emit execDialog(String2Property(prop));
}

void PanelAgent::ExecMenu(const QStringList &entries)
{
    QList<KimpanelProperty> list;
    foreach(const QString & entry, entries) {
        list << String2Property(entry);
    }

    emit execMenu(list);
}

void PanelAgent::SetSpotRect(int x, int y, int w, int h)
{
    emit updateSpotRect(x, y, w, h);
}

void PanelAgent::SetLookupTable(const QStringList& labels, const QStringList& candis, const QStringList& attrlists, bool hasPrev, bool hasNext, int cursor, int layout)
{
    emit updateLookupTableFull(Args2LookupTable(labels, candis, attrlists, hasPrev, hasNext), cursor, layout);
}
