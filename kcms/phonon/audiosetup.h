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

#ifndef PHONON_AUDIOSETUP_H
#define PHONON_AUDIOSETUP_H

#include <canberra.h>
#include <pulse/pulseaudio.h>

#include "ui_audiosetup.h"

class QTimer;

struct pa_glib_mainloop;

typedef struct {
    quint32 index;
    QString name;
    QString icon;
    QMap<quint32, QPair<QString, QString> > profiles;
    QString activeProfile;
} cardInfo;

typedef struct {
    quint32 index;
    quint32 cardIndex;
    QString name;
    QString icon;
    pa_channel_map channelMap;
    QMap<quint32, QPair<QString, QString> > ports;
    QString activePort;
} deviceInfo;

class AudioSetup : public QWidget, private Ui::AudioSetup
{
    Q_OBJECT

public:
    explicit AudioSetup(QWidget *parent = 0);
    ~AudioSetup();

    void load();
    void save();
    void defaults();
    uint32_t getCurrentSinkIndex();
    void updateCard(const pa_card_info*);
    void removeCard(uint32_t idx);
    void updateSink(const pa_sink_info*);
    void removeSink(uint32_t idx);
    void updateSource(const pa_source_info*);
    void removeSource(uint32_t idx);
    void updateFromPulse();
    void updateIndependantDevices();
    void updateVUMeter(int vol);

public Q_SLOTS:
    void cardChanged();
    void profileChanged();
    void deviceChanged();
    void portChanged();
    void reallyUpdateVUMeter();
    bool connectToDaemon();

Q_SIGNALS:
    void changed();
    void ready();

private:
    void _updatePlacementTester();
    void _createMonitorStreamForSource(uint32_t);

    QLabel *m_icon;
    int m_OutstandingRequests;
    ca_context* m_Canberra;
    pa_stream* m_VUStream;
    int m_VURealValue;
    QTimer* m_VUTimer;
};

QDebug operator<<(QDebug dbg, const pa_context_state_t &state);

#endif // PHONON_AUDIOSETUP_H
