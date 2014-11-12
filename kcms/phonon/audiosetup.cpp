/*  This file is part of the KDE project
    Copyright (C) 2010 Colin Guthrie <cguthrie@mandriva.org>
    Copyright (C) 2011 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "audiosetup.h"

#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QTimer>
#include <QLabel>

#include <KLocalizedString>
#include <KIconLoader>
#include <KUser>

#include <pulse/glib-mainloop.h>

#include "testspeakerwidget.h"

#define SS_DEFAULT_ICON "audio-card"

#define THAT(userdata) Q_ASSERT(userdata); AudioSetup *ss = static_cast<AudioSetup *>(userdata)

static pa_glib_mainloop *s_mainloop = NULL;
static pa_context *s_context = NULL;

QMap<quint32, cardInfo> s_Cards;
QMap<quint32, deviceInfo> s_Sinks;
QMap<quint32, deviceInfo> s_Sources;

static void card_cb(pa_context *c, const pa_card_info *i, int eol, void *userdata) {
    Q_ASSERT(c);
    THAT(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        qDebug() << "Card callback failure";
        return;
    }

    if (eol > 0) {
        ss->updateFromPulse();
        return;
    }

    Q_ASSERT(i);
    ss->updateCard(i);
}

static void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    Q_ASSERT(c);
    THAT(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;
        qDebug() << "Sink callback failure";
        return;
    }

    if (eol > 0) {
        ss->updateIndependantDevices();
        ss->updateFromPulse();
        return;
    }

    Q_ASSERT(i);
    ss->updateSink(i);
}

static void source_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    Q_ASSERT(c);
    THAT(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        qDebug() << "Source callback failure";
        return;
    }

    if (eol > 0) {
        ss->updateIndependantDevices();
        ss->updateFromPulse();
        return;
    }

    Q_ASSERT(i);
    ss->updateSource(i);
}

static void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    Q_ASSERT(c);
    THAT(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_CARD:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            ss->removeCard(index);
        } else {
            pa_operation *operation =
                    pa_context_get_card_info_by_index(c, index, card_cb, ss);
            if (!operation) {
                qDebug() << "pa_context_get_card_info_by_index() failed";
                return;
            }
            pa_operation_unref(operation);
        }
        break;

    case PA_SUBSCRIPTION_EVENT_SINK:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            ss->removeSink(index);
        } else {
            pa_operation *operation =
                    pa_context_get_sink_info_by_index(c, index, sink_cb, ss);
            if (!operation) {
                qDebug() << "pa_context_get_sink_info_by_index() failed";
                return;
            }
            pa_operation_unref(operation);
        }
        break;

    case PA_SUBSCRIPTION_EVENT_SOURCE:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            ss->removeSource(index);
        } else {
            pa_operation *o;
            if (!(o = pa_context_get_source_info_by_index(c, index, source_cb, ss))) {
                qDebug() << "pa_context_get_source_info_by_index() failed";
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
}

static void context_state_callback(pa_context *c, void *userdata)
{
    Q_ASSERT(c);
    THAT(userdata);

    qDebug() << "context_state_callback" << pa_context_get_state(c);
    pa_context_state_t state = pa_context_get_state(c);
    if (state == PA_CONTEXT_READY) {
        // Attempt to load things up
        pa_operation *o;

        pa_context_set_subscribe_callback(c, subscribe_cb, ss);

        if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                       (PA_SUBSCRIPTION_MASK_CARD|
                                        PA_SUBSCRIPTION_MASK_SINK|
                                        PA_SUBSCRIPTION_MASK_SOURCE), NULL, NULL))) {
            qDebug() << "pa_context_subscribe() failed";
            return;
        }
        pa_operation_unref(o);

        if (!(o = pa_context_get_card_info_list(c, card_cb, ss))) {
            qDebug() << "pa_context_get_card_info_list() failed";
            return;
        }
        pa_operation_unref(o);

        if (!(o = pa_context_get_sink_info_list(c, sink_cb, ss))) {
            qDebug() << "pa_context_get_sink_info_list() failed";
            return;
        }
        pa_operation_unref(o);

        if (!(o = pa_context_get_source_info_list(c, source_cb, ss))) {
            qDebug() << "pa_context_get_source_info_list() failed";
            return;
        }
        pa_operation_unref(o);

        ss->load();

    } else if (!PA_CONTEXT_IS_GOOD(state)) {
        // If this is our probe phase, exit our context immediately
        if (s_context != c)
            pa_context_disconnect(c);
        else {
            qWarning() << "PulseAudio context lost. Scheduling reconnect in eventloop.";
            pa_context_unref(s_context);
            s_context = 0;
            QMetaObject::invokeMethod(ss, "connectToDaemon", Qt::QueuedConnection);
        }
    }
}

static void suspended_callback(pa_stream *s, void *userdata) {
    THAT(userdata);

    if (pa_stream_is_suspended(s))
        ss->updateVUMeter(-1);
}

static void read_callback(pa_stream *s, size_t length, void *userdata) {
    THAT(userdata);

    const void *data;
    int v;
    if (pa_stream_peek(s, &data, &length) < 0) {
        qDebug() << "Failed to read data from stream";
        return;
    }

    Q_ASSERT(length > 0);
    Q_ASSERT(length % sizeof(float) == 0);

    v = ((const float*) data)[length / sizeof(float) -1] * 100;

    pa_stream_drop(s);

    if (v < 0)
        v = 0;
    if (v > 100)
        v = 100;

    ss->updateVUMeter(v);
}


AudioSetup::AudioSetup(QWidget *parent)
    : QWidget(parent)
    , m_OutstandingRequests(3)
    , m_Canberra(0)
    , m_VUStream(0)
    , m_VURealValue(0)
{
    setupUi(this);

    cardLabel->setEnabled(false);
    cardBox->setEnabled(false);
    profileLabel->setVisible(false);
    profileBox->setVisible(false);

    deviceLabel->setEnabled(false);
    deviceBox->setEnabled(false);
    portLabel->setVisible(false);
    portBox->setVisible(false);

    for (int i = 0; i < 5; ++i)
        placementGrid->setColumnStretch(i, 1);
    for (int i = 0; i < 3; ++i)
        placementGrid->setRowStretch(i, 1);

    m_icon = new QLabel(this);
    m_icon->setPixmap(QPixmap(KUser().faceIconPath()));
    if (m_icon->pixmap()->isNull())
        m_icon->setPixmap(QIcon::fromTheme("system-users").pixmap(KIconLoader::SizeHuge, KIconLoader::SizeHuge));
    placementGrid->addWidget(m_icon, 1, 2, Qt::AlignCenter);

    update();
    connect(cardBox, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &AudioSetup::cardChanged);
    connect(profileBox, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &AudioSetup::profileChanged);
    connect(deviceBox, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &AudioSetup::deviceChanged);
    connect(portBox, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &AudioSetup::portChanged);

    m_VUTimer = new QTimer(this);
    m_VUTimer->setInterval(10);
    connect(m_VUTimer, &QTimer::timeout, this, &AudioSetup::reallyUpdateVUMeter);

    // We require a glib event loop
    const QByteArray eventDispatcher(
                QAbstractEventDispatcher::instance()->metaObject()->className());
    if (!eventDispatcher.contains("EventDispatcherGlib")) {
        qDebug() << "Disabling PulseAudio integration for lack of GLib event loop.";
        return;
    }

    int ret = ca_context_create(&m_Canberra);
    if (ret < 0) {
        qDebug() << "Disabling PulseAudio integration. Canberra context failed.";
        return;
    }

    s_mainloop = pa_glib_mainloop_new(NULL);
    if (!s_mainloop) {
        qDebug() << "Disabling PulseAudio integration for lack of working GLib event loop.";
        ca_context_destroy(m_Canberra);
        m_Canberra = 0;
        return;
    }

    connectToDaemon();
}

AudioSetup::~AudioSetup()
{
    if (m_Canberra)
        ca_context_destroy(m_Canberra);
    if (s_context) {
        pa_context_unref(s_context);
        s_context = 0;
    }
    if (s_mainloop) {
        pa_glib_mainloop_free(s_mainloop);
        s_mainloop = 0;
    }
    s_Cards.clear();
    s_Sinks.clear();
    s_Sources.clear();
}

void AudioSetup::load()
{
}

void AudioSetup::save()
{
}

void AudioSetup::defaults()
{
}

void AudioSetup::updateCard(const pa_card_info *pInfo)
{
    cardInfo info;
    info.index = pInfo->index;

    const char *description = pa_proplist_gets(pInfo->proplist, PA_PROP_DEVICE_DESCRIPTION);
    if(description)
        info.name = QString::fromUtf8(description);
    else
        info.name = QString::fromUtf8(pInfo->name);

    const char *icon = pa_proplist_gets(pInfo->proplist, PA_PROP_DEVICE_ICON_NAME);
    if (icon)
        info.icon = QString::fromUtf8(icon);
    else
        info.icon = QString::fromUtf8(SS_DEFAULT_ICON);

    for (quint32 i = 0; i < pInfo->n_profiles; ++i) {
        const pa_card_profile_info *profile = &(pInfo->profiles[i]);
        const quint32 priority = profile->priority;
        const QPair<QString, QString> name(profile->name ? QString::fromUtf8(profile->name) : QString(),
                                           profile->description ? QString::fromUtf8(profile->description) : QString());
        info.profiles.insert(priority, name);
    }

    if (pInfo->active_profile)
        info.activeProfile = pInfo->active_profile->name;

    cardBox->blockSignals(true);
    if (s_Cards.contains(pInfo->index)) {
        int idx = cardBox->findData(pInfo->index);
        if (idx >= 0) {
            cardBox->setItemIcon(idx, QIcon::fromTheme(info.icon));
            cardBox->setItemText(idx, info.name);
        }
    } else {
        cardBox->addItem(QIcon::fromTheme(info.icon), info.name, pInfo->index);
    }
    cardBox->blockSignals(false);

    s_Cards[pInfo->index] = info;

    cardChanged();
    qDebug() << "Got info about card" << info.name;
}

void AudioSetup::removeCard(uint32_t index)
{
    s_Cards.remove(index);
    updateFromPulse();
    const int idx = cardBox->findData(index);
    if (idx >= 0)
        cardBox->removeItem(idx);
}

void AudioSetup::updateSink(const pa_sink_info* i)
{
    deviceInfo info;
    info.index = i->index;
    info.cardIndex = i->card;
    info.name = QString::fromUtf8(i->description);

    const char *icon = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_ICON_NAME);
    info.icon = icon ? icon : SS_DEFAULT_ICON;

    info.channelMap = i->channel_map;

    for (uint32_t j = 0; j < i->n_ports; ++j)
        info.ports[i->ports[j]->priority] = QPair<QString,QString>(i->ports[j]->name, QString::fromUtf8(i->ports[j]->description));
    if (i->active_port)
        info.activePort = i->active_port->name;

    s_Sinks[i->index] = info;

    // Need to update the currently displayed port if this sink is the currently displayed one.
    if (info.ports.size()) {
        int idx = deviceBox->currentIndex();
        if (idx >= 0) {
            qint64 index = deviceBox->itemData(idx).toInt();
            if (index >= 0 && index == i->index) {
                portBox->blockSignals(true);
                portBox->setCurrentIndex(portBox->findData(info.activePort));
                portBox->blockSignals(false);
            }
        }
    }

    qDebug() << "Got info about sink" << info.name;
}

void AudioSetup::removeSink(uint32_t index)
{
    s_Sinks.remove(index);
    updateIndependantDevices();
    updateFromPulse();
    int idx = deviceBox->findData(index);
    if (idx >= 0)
        deviceBox->removeItem(idx);
}

void AudioSetup::updateSource(const pa_source_info* i)
{
    if (i->monitor_of_sink != PA_INVALID_INDEX)
        return;

    deviceInfo info;
    info.index = i->index;
    info.cardIndex = i->card;
    info.name = QString::fromUtf8(i->description);

    const char* icon = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_ICON_NAME);
    info.icon = icon ? icon : SS_DEFAULT_ICON;

    info.channelMap = i->channel_map;

    for (uint32_t j = 0; j < i->n_ports; ++j)
        info.ports[i->ports[j]->priority] = QPair<QString,QString>(i->ports[j]->name, QString::fromUtf8(i->ports[j]->description));
    if (i->active_port)
        info.activePort = i->active_port->name;

    s_Sources[i->index] = info;

    // Need to update the currently displayed port if this source is the currently displayed one.
    if (false && info.ports.size()) {
        int idx = deviceBox->currentIndex();
        if (idx >= 0) {
            qint64 index = deviceBox->itemData(idx).toInt();
            if (index < 0 && ((-1*index) - 1) == i->index) {
                portBox->blockSignals(true);
                portBox->setCurrentIndex(portBox->findData(info.activePort));
                portBox->blockSignals(false);
            }
        }
    }

    qDebug() << "Got info about source" << info.name;
}

void AudioSetup::removeSource(uint32_t index)
{
    s_Sources.remove(index);
    updateIndependantDevices();
    updateFromPulse();
    int idx = deviceBox->findData(index);
    if (false && idx >= 0)
        deviceBox->removeItem(idx);
}

void AudioSetup::updateFromPulse()
{
    bool setupReady = false;
    if (m_OutstandingRequests > 0) {
        if (0 == --m_OutstandingRequests) {
            // Work out which seclector to pick by default (we want to choose a real Card if possible)
            if (s_Cards.size() != cardBox->count())
                cardBox->setCurrentIndex(1);
            setupReady = true;
        }
    }

    if (!m_OutstandingRequests) {
        if (!s_Cards.size() && !s_Sinks.size()) {
            cardLabel->setEnabled(false);
            cardBox->setEnabled(false);
            profileLabel->setVisible(false);
            profileBox->setVisible(false);

            deviceLabel->setEnabled(false);
            deviceBox->setEnabled(false);
            portLabel->setVisible(false);
            portBox->setVisible(false);
        }
        if (s_Cards.size() && !cardBox->isEnabled()) {
            cardLabel->setEnabled(true);
            cardBox->setEnabled(true);
            cardChanged();
        }
        if (s_Sinks.size() && !deviceBox->isEnabled()) {
            deviceLabel->setEnabled(true);
            deviceBox->setEnabled(true);
            deviceChanged();
        }

        if (setupReady) {
            emit ready();
        }
    }
}

void AudioSetup::cardChanged()
{
    int idx = cardBox->currentIndex();
    if (idx < 0) {
        profileLabel->setVisible(false);
        profileBox->setVisible(false);
        return;
    }

    uint32_t card_index = cardBox->itemData(idx).toUInt();
    Q_ASSERT(PA_INVALID_INDEX == card_index || s_Cards.contains(card_index));
    bool show_profiles = (PA_INVALID_INDEX != card_index && s_Cards[card_index].profiles.size());
    if (show_profiles) {
        cardInfo &card_info = s_Cards[card_index];
        profileBox->blockSignals(true);
        profileBox->clear();
        QMap<quint32, QPair<QString, QString> >::const_iterator it;
        for (it = card_info.profiles.constBegin(); it != card_info.profiles.constEnd(); ++it)
            profileBox->insertItem(0, it.value().second, it.value().first);
        profileBox->setCurrentIndex(profileBox->findData(card_info.activeProfile));
        profileBox->blockSignals(false);
    }
    profileLabel->setVisible(show_profiles);
    profileBox->setVisible(show_profiles);

    deviceBox->blockSignals(true);
    deviceBox->clear();
    QMap<quint32, deviceInfo>::const_iterator it;
    for (it = s_Sinks.constBegin(); it != s_Sinks.constEnd(); ++it) {
        if (it->cardIndex == card_index)
            deviceBox->addItem(QIcon::fromTheme(it->icon), i18n("Playback (%1)", it->name), it->index);
    }
    for (it = s_Sources.constBegin(); it != s_Sources.constEnd(); ++it) {
        if (it->cardIndex == card_index)
            deviceBox->addItem(QIcon::fromTheme(it->icon), i18n("Recording (%1)", it->name), ((-1*it->index) - 1));
    }
    deviceBox->blockSignals(false);

    deviceGroupBox->setEnabled(!!deviceBox->count());

    deviceChanged();

    qDebug() << "Doing update" << cardBox->currentIndex();

    emit changed();
}

void AudioSetup::profileChanged()
{
    quint32 card_index = cardBox->itemData(cardBox->currentIndex()).toUInt();
    Q_ASSERT(PA_INVALID_INDEX != card_index);

    QString profile = profileBox->itemData(profileBox->currentIndex()).toString();
    qDebug() << "Changing profile to" << profile;

    Q_ASSERT(s_Cards[card_index].profiles.size());

    pa_operation *operation =
            pa_context_set_card_profile_by_index(s_context,
                                                 card_index,
                                                 qPrintable(profile),
                                                 NULL,
                                                 NULL);
    if (!operation)
        qDebug() << "pa_context_set_card_profile_by_name() failed";
    else
        pa_operation_unref(operation);

    emit changed();
}

void AudioSetup::updateIndependantDevices()
{
    // Should we display the "Independent Devices" drop down?
    // Count all the sinks without cards
    bool showID = false;
    QMap<quint32, deviceInfo>::const_iterator it;
    for (it = s_Sinks.constBegin(); it != s_Sinks.constEnd(); ++it) {
        if (PA_INVALID_INDEX == it->cardIndex) {
            showID = true;
            break;
        }
    }

    bool haveID = (PA_INVALID_INDEX == cardBox->itemData(0).toUInt());

    qDebug() << QString("Want ID: %1; Have ID: %2").arg(showID?"Yes":"No").arg(haveID?"Yes":"No");

    cardBox->blockSignals(true);
    if (haveID && !showID)
        cardBox->removeItem(0);
    else if (!haveID && showID)
        cardBox->insertItem(0, QIcon::fromTheme(SS_DEFAULT_ICON), i18n("Independent Devices"), PA_INVALID_INDEX);
    cardBox->blockSignals(false);
}

void AudioSetup::updateVUMeter(int vol)
{
    if (vol < 0) {
        inputLevels->setEnabled(false);
        inputLevels->setValue(0);
        m_VURealValue = 0;
    } else {
        inputLevels->setEnabled(true);
        if (vol > inputLevels->value())
            inputLevels->setValue(vol);
        m_VURealValue = vol;
    }
}

void AudioSetup::reallyUpdateVUMeter()
{
    int val = inputLevels->value();
    if (val > m_VURealValue)
        inputLevels->setValue(val-1);
}

bool AudioSetup::connectToDaemon()
{
    pa_mainloop_api *api = pa_glib_mainloop_get_api(s_mainloop);

    s_context = pa_context_new(api, i18n("KDE Audio Hardware Setup").toUtf8().constData());
    if (pa_context_connect(s_context, NULL, PA_CONTEXT_NOFAIL, 0) < 0) {
        qDebug() << "Disabling PulseAudio integration. Context connection failed: " << pa_strerror(pa_context_errno(s_context));
        pa_context_unref(s_context);
        s_context = 0;
        pa_glib_mainloop_free(s_mainloop);
        s_mainloop = 0;
        ca_context_destroy(m_Canberra);
        m_Canberra = 0;
        setEnabled(false);
        return false;
    }

    pa_context_set_state_callback(s_context, &context_state_callback, this);
    setEnabled(true);
    return true;
}

static deviceInfo &getDeviceInfo(qint64 index)
{
    if (index >= 0) {
        Q_ASSERT(s_Sinks.contains(index));
        return s_Sinks[index];
    }

    index = (-1 * index) - 1;
    Q_ASSERT(s_Sources.contains(index));
    return s_Sources[index];
}

void AudioSetup::deviceChanged()
{
    int idx = deviceBox->currentIndex();
    if (idx < 0) {
        portLabel->setVisible(false);
        portBox->setVisible(false);
        _updatePlacementTester();
        return;
    }
    qint64 index = deviceBox->itemData(idx).toInt();
    deviceInfo &device_info = getDeviceInfo(index);

    qDebug() << QString("Updating ports for device '%1' (%2 ports available)")
                .arg(device_info.name)
                .arg(device_info.ports.size());

    bool showPorts = !!device_info.ports.size();
    if (showPorts) {
        portBox->blockSignals(true);
        portBox->clear();
        QMap<quint32, QPair<QString, QString> >::const_iterator it;
        for (it = device_info.ports.constBegin(); it != device_info.ports.constEnd(); ++it)
            portBox->insertItem(0, it.value().second, it.value().first);
        portBox->setCurrentIndex(portBox->findData(device_info.activePort));
        portBox->blockSignals(false);
    }
    portLabel->setVisible(showPorts);
    portBox->setVisible(showPorts);

    if (deviceBox->currentIndex() >= 0) {
        if (index < 0)
            _createMonitorStreamForSource((-1*index) - 1);
        else if (m_VUStream) {
            pa_stream_disconnect(m_VUStream);
            m_VUStream = NULL;
        }

        _updatePlacementTester();
    }

    emit changed();
}

void AudioSetup::portChanged()
{
    qint64 index = deviceBox->itemData(deviceBox->currentIndex()).toInt();

    QString port = portBox->itemData(portBox->currentIndex()).toString();
    qDebug() << "Changing port to" << port;

#ifndef QT_NO_DEBUG
    deviceInfo &device_info = getDeviceInfo(index);
    Q_ASSERT(device_info.ports.size());
#endif /* QT_NO_DEBUG */

    pa_operation *o;
    if (index >= 0) {
        if (!(o = pa_context_set_sink_port_by_index(s_context, (uint32_t)index, port.toAscii().constData(), NULL, NULL)))
            qDebug() << "pa_context_set_sink_port_by_index() failed";
        else
            pa_operation_unref(o);
    } else {
        if (!(o = pa_context_set_source_port_by_index(s_context, (uint32_t)((-1*index) - 1), port.toAscii().constData(), NULL, NULL)))
            qDebug() << "pa_context_set_source_port_by_index() failed";
        else
            pa_operation_unref(o);
    }

    emit changed();
}

void AudioSetup::_updatePlacementTester()
{
    static const int position_table[] = {
        /* Position, X, Y */
        PA_CHANNEL_POSITION_FRONT_LEFT,            0, 0,
        PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER,  1, 0,
        PA_CHANNEL_POSITION_FRONT_CENTER,          2, 0,
        PA_CHANNEL_POSITION_MONO,                  2, 0,
        PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER, 3, 0,
        PA_CHANNEL_POSITION_FRONT_RIGHT,           4, 0,
        PA_CHANNEL_POSITION_SIDE_LEFT,             0, 1,
        PA_CHANNEL_POSITION_SIDE_RIGHT,            4, 1,
        PA_CHANNEL_POSITION_REAR_LEFT,             0, 2,
        PA_CHANNEL_POSITION_REAR_CENTER,           2, 2,
        PA_CHANNEL_POSITION_REAR_RIGHT,            4, 2,
        PA_CHANNEL_POSITION_LFE,                   3, 2
    };

    QLayoutItem* w;
    while ((w = placementGrid->takeAt(0))) {
        if (w->widget() != m_icon) {
            if (w->widget())
                delete w->widget();
            delete w;
        }
    }
    placementGrid->addWidget(m_icon, 1, 2, Qt::AlignCenter);
    int idx = deviceBox->currentIndex();
    if (idx < 0)
        return;

    qint64 index = deviceBox->itemData(idx).toInt();
    deviceInfo& sink_info = getDeviceInfo(index);

    if (index < 0) {
        playbackOrCaptureStack->setCurrentIndex(1);
        m_VUTimer->start();
        return;
    }

    playbackOrCaptureStack->setCurrentIndex(0);
    m_VUTimer->stop();

    for (int i = 0; i < 36; i += 3) {
        pa_channel_position_t pos = (pa_channel_position_t)position_table[i];
        // Check to see if we have this item in our current channel map.
        bool have = false;
        for (uint32_t j = 0; j < sink_info.channelMap.channels; ++j) {
            if (sink_info.channelMap.map[j] == pos) {
                have = true;
                break;
            }
        }
        if (!have) {
            continue;
        }

        QPushButton *btn = new TestSpeakerWidget(pos, m_Canberra, this);
        placementGrid->addWidget(btn, position_table[i+2], position_table[i+1], Qt::AlignCenter);
    }
}

void AudioSetup::_createMonitorStreamForSource(uint32_t source_idx)
{
    if (m_VUStream) {
        pa_stream_disconnect(m_VUStream);
        m_VUStream = NULL;
    }

    char t[16];
    pa_buffer_attr attr;
    pa_sample_spec ss;

    ss.channels = 1;
    ss.format = PA_SAMPLE_FLOAT32;
    ss.rate = 25;

    memset(&attr, 0, sizeof(attr));
    attr.fragsize = sizeof(float);
    attr.maxlength = static_cast<quint32>(-1);

    snprintf(t, sizeof(t), "%u", source_idx);

    m_VUStream = pa_stream_new(s_context, "Peak detect", &ss, NULL);
    if (!m_VUStream) {
        qDebug() << "Failed to create monitoring stream";
        return;
    }

    pa_stream_set_read_callback(m_VUStream, read_callback, this);
    pa_stream_set_suspended_callback(m_VUStream, suspended_callback, this);

    if (pa_stream_connect_record(m_VUStream, t, &attr, (pa_stream_flags_t) (PA_STREAM_DONT_MOVE|PA_STREAM_PEAK_DETECT|PA_STREAM_ADJUST_LATENCY)) < 0) {
        qDebug() << "Failed to connect monitoring stream";
        pa_stream_unref(m_VUStream);
        m_VUStream = NULL;
    }
}

quint32 AudioSetup::getCurrentSinkIndex()
{
    int idx = deviceBox->currentIndex();
    if (idx < 0)
        return PA_INVALID_INDEX;

    qint64 index = deviceBox->itemData(idx).toInt();
    if (index >= 0)
        return static_cast<quint32>(index);

    return PA_INVALID_INDEX;
}

QDebug operator<<(QDebug dbg, const pa_context_state_t &state)
{
    QString name;
    switch (state) {
    case PA_CONTEXT_UNCONNECTED:  name = QLatin1Literal("Unconnected");
    case PA_CONTEXT_CONNECTING:   name = QLatin1Literal("Connecting");
    case PA_CONTEXT_AUTHORIZING:  name = QLatin1Literal("Authorizing");
    case PA_CONTEXT_SETTING_NAME: name = QLatin1Literal("Setting Name");
    case PA_CONTEXT_READY:        name = QLatin1Literal("Ready");
    case PA_CONTEXT_FAILED:       name = QLatin1Literal("Failed");
    case PA_CONTEXT_TERMINATED:   name = QLatin1Literal("Terminated");
    }

    if (name.isEmpty())
        name = QString("Unknown state(%0)").arg(state);

    dbg.nospace() << name;
    return dbg;
}

#include "audiosetup.moc"
