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

#ifndef PHONON_TESTSPEAKERWIDGET_H
#define PHONON_TESTSPEAKERWIDGET_H

#include <QPushButton>

#include <canberra.h>
#include <pulse/pulseaudio.h>

class AudioSetup;

class TestSpeakerWidget: public QPushButton
{
    Q_OBJECT
public:
    TestSpeakerWidget(const pa_channel_position_t pos, ca_context *canberra, AudioSetup* ss);
    ~TestSpeakerWidget();

public slots:
    void onFinish();

private slots:
    void toggled(bool);

private:
    QString _positionName();
    const char* _positionAsString();
    const char* _positionSoundName();

    AudioSetup* m_Ss;
    pa_channel_position_t m_Pos;
    ca_context* m_Canberra;
};

#endif // PHONON_TESTSPEAKERWIDGET_H
