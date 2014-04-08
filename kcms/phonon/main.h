/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef MAIN_H
#define MAIN_H

#include <KCModule>

namespace Phonon {
class DevicePreference;
}
class BackendSelection;

#ifdef HAVE_PULSEAUDIO
class AudioSetup;
#endif
class QTabWidget;

class PhononKcm : public KCModule
{
    Q_OBJECT
public:
    PhononKcm(QWidget *parent, const QVariantList &);

    void load();
    void save();
    void defaults();

#ifdef HAVE_PULSEAUDIO
private Q_SLOTS:
    void speakerSetupReady();
#endif

private:
    QTabWidget* m_tabs;
    Phonon::DevicePreference *m_devicePreferenceWidget;
    BackendSelection *m_backendSelection;
#ifdef HAVE_PULSEAUDIO
    AudioSetup *m_speakerSetup;
#endif
};

#endif // MAIN_H
