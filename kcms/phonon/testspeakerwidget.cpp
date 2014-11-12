/*  This file is part of the KDE project
    Copyright (C) 2010 Colin Guthrie <cguthrie@mandriva.org>

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

#include "testspeakerwidget.h"

#include <KLocalizedString>

#include "audiosetup.h"

static uint32_t s_CurrentIndex = PA_INVALID_INDEX;
static TestSpeakerWidget *s_CurrentWidget = NULL;

static void finish_cb(ca_context *, uint32_t id, int, void *)
{
    Q_ASSERT(id == 0);
    Q_UNUSED(id); // Suppress compiler warning if QT_NO_DEBUG
    // Mustn't access QWidgets from a foreign thread, so queue a slot call.
    QMetaObject::invokeMethod(s_CurrentWidget, "onFinish", Qt::QueuedConnection);
}

TestSpeakerWidget::TestSpeakerWidget(const pa_channel_position_t pos, ca_context *canberra, AudioSetup* ss)
    : QPushButton(QIcon::fromTheme("preferences-desktop-sound"), "Test", ss)
    , m_Ss(ss)
    , m_Pos(pos)
    , m_Canberra(canberra)
{
    setCheckable(true);
    setText(_positionName());
    connect(this, &TestSpeakerWidget::toggled, this, &TestSpeakerWidget::toggled);
}

TestSpeakerWidget::~TestSpeakerWidget()
{
    if (this == s_CurrentWidget)
        s_CurrentWidget = NULL;
}

void TestSpeakerWidget::toggled(bool state)
{
    if (s_CurrentIndex != PA_INVALID_INDEX) {
        ca_context_cancel(m_Canberra, s_CurrentIndex);
        s_CurrentIndex = PA_INVALID_INDEX;
    }
    if (s_CurrentWidget) {
        if (this != s_CurrentWidget && state)
            s_CurrentWidget->setChecked(false);
        s_CurrentWidget = NULL;
    }
    if (!state)
        return;

    uint32_t sink_index = m_Ss->getCurrentSinkIndex();
    char dev[64];
    snprintf(dev, sizeof(dev), "%lu", (unsigned long) sink_index);
    ca_context_change_device(m_Canberra, dev);

    const char* sound_name = _positionSoundName();
    ca_proplist* proplist;
    ca_proplist_create(&proplist);
    
    ca_proplist_sets(proplist, CA_PROP_MEDIA_ROLE, "test");
    ca_proplist_sets(proplist, CA_PROP_MEDIA_NAME, _positionName().toAscii().constData());
    ca_proplist_sets(proplist, CA_PROP_CANBERRA_FORCE_CHANNEL, _positionAsString());
    ca_proplist_sets(proplist, CA_PROP_CANBERRA_ENABLE, "1");

    ca_proplist_sets(proplist, CA_PROP_EVENT_ID, sound_name);
    s_CurrentIndex = 0;
    s_CurrentWidget = this;
    if (ca_context_play_full(m_Canberra, s_CurrentIndex, proplist, finish_cb, NULL) < 0) {
        // Try a different sound name.
        ca_proplist_sets(proplist, CA_PROP_EVENT_ID, "audio-test-signal");
        if (ca_context_play_full(m_Canberra, s_CurrentIndex, proplist, finish_cb, NULL) < 0) {
            // Finaly try this... if this doesn't work, then stuff it.
            ca_proplist_sets(proplist, CA_PROP_EVENT_ID, "bell-window-system");
            if (ca_context_play_full(m_Canberra, s_CurrentIndex, proplist, finish_cb, NULL) < 0) {
                s_CurrentIndex = PA_INVALID_INDEX;
                s_CurrentWidget = NULL;
                setChecked(false);
            }
        }
    }

    ca_context_change_device(m_Canberra, NULL);
    ca_proplist_destroy(proplist);
}

void TestSpeakerWidget::onFinish()
{
    if (s_CurrentWidget && s_CurrentWidget->isChecked()) {
        s_CurrentIndex = PA_INVALID_INDEX;
        s_CurrentWidget->setChecked(false);
        s_CurrentWidget = NULL;
    }
}

const char* TestSpeakerWidget::_positionAsString()
{
    switch (m_Pos) {
    case PA_CHANNEL_POSITION_FRONT_LEFT:
        return "front-left";
    case PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER:
        return "front-left-of-center";
    case PA_CHANNEL_POSITION_FRONT_CENTER:
        return "front-center";
    case PA_CHANNEL_POSITION_MONO:
        return "mono";
    case PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER:
        return "front-right-of-center";
    case PA_CHANNEL_POSITION_FRONT_RIGHT:
        return "front-right";
    case PA_CHANNEL_POSITION_SIDE_LEFT:
        return "side-left";
    case PA_CHANNEL_POSITION_SIDE_RIGHT:
        return "side-right";
    case PA_CHANNEL_POSITION_REAR_LEFT:
        return "rear-left";
    case PA_CHANNEL_POSITION_REAR_CENTER:
        return "rear-center";
    case PA_CHANNEL_POSITION_REAR_RIGHT:
        return "rear-right";
    case PA_CHANNEL_POSITION_LFE:
        return "lfe";
    default:
        break;
    }
    return "invalid";
}

QString TestSpeakerWidget::_positionName()
{
    switch (m_Pos) {
    case PA_CHANNEL_POSITION_FRONT_LEFT:
        return i18n("Front Left");
    case PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER:
        return i18n("Front Left of Center");
    case PA_CHANNEL_POSITION_FRONT_CENTER:
        return i18n("Front Center");
    case PA_CHANNEL_POSITION_MONO:
        return i18n("Mono");
    case PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER:
        return i18n("Front Right of Center");
    case PA_CHANNEL_POSITION_FRONT_RIGHT:
        return i18n("Front Right");
    case PA_CHANNEL_POSITION_SIDE_LEFT:
        return i18n("Side Left");
    case PA_CHANNEL_POSITION_SIDE_RIGHT:
        return i18n("Side Right");
    case PA_CHANNEL_POSITION_REAR_LEFT:
        return i18n("Rear Left");
    case PA_CHANNEL_POSITION_REAR_CENTER:
        return i18n("Rear Center");
    case PA_CHANNEL_POSITION_REAR_RIGHT:
        return i18n("Rear Right");
    case PA_CHANNEL_POSITION_LFE:
        return i18n("Subwoofer");
    default:
        break;
    }
    return i18n("Unknown Channel");
}

const char* TestSpeakerWidget::_positionSoundName()
{
    switch (m_Pos) {
    case PA_CHANNEL_POSITION_FRONT_LEFT:
        return "audio-channel-front-left";
    case PA_CHANNEL_POSITION_FRONT_RIGHT:
        return "audio-channel-front-right";
    case PA_CHANNEL_POSITION_FRONT_CENTER:
        return "audio-channel-front-center";
    case PA_CHANNEL_POSITION_REAR_LEFT:
        return "audio-channel-rear-left";
    case PA_CHANNEL_POSITION_REAR_RIGHT:
        return "audio-channel-rear-right";
    case PA_CHANNEL_POSITION_REAR_CENTER:
        return "audio-channel-rear-center";
    case PA_CHANNEL_POSITION_LFE:
        return "audio-channel-lfe";
    case PA_CHANNEL_POSITION_SIDE_LEFT:
        return "audio-channel-side-left";
    case PA_CHANNEL_POSITION_SIDE_RIGHT:
        return "audio-channel-side-right";
    default:
        break;
    }
    return NULL;
}

#include "testspeakerwidget.moc"
