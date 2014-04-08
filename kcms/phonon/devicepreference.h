/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2011 Casian Andrei <skeletk13@gmail.com>

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

#ifndef DEVICEPREFERENCE_H_STUPID_UIC
#define DEVICEPREFERENCE_H_STUPID_UIC

#include "ui_devicepreference.h"

#include <QMap>
#include <QStandardItem>

#include <phonon/objectdescription.h>
#include <phonon/objectdescriptionmodel.h>

namespace Phonon {

class MediaObject;
class AudioOutput;
class VideoWidget;

class DevicePreference : public QWidget, private Ui::DevicePreference
{
    Q_OBJECT
public:
    explicit DevicePreference(QWidget *parent = 0);
    virtual ~DevicePreference();

    void load();
    void save();
    void defaults();
    void pulseAudioEnabled();

Q_SIGNALS:
    void changed();

protected:
    void changeEvent(QEvent *);

private Q_SLOTS:
    void on_preferButton_clicked();
    void on_deferButton_clicked();
    void on_showAdvancedDevicesCheckBox_toggled();
    void on_applyPreferencesButton_clicked();
    void on_testPlaybackButton_toggled(bool down);
    void updateButtonsEnabled();
    void updateDeviceList();
    void updateAudioOutputDevices();
    void updateAudioCaptureDevices();
    void updateVideoCaptureDevices();

private:
    enum DeviceType {dtInvalidDevice, dtAudioOutput, dtAudioCapture, dtVideoCapture};

private:
    template<ObjectDescriptionType T> void removeDevice(const ObjectDescription<T> &deviceToRemove,
                                                        QMap<int, ObjectDescriptionModel<T> *> *modelMap);
    void loadCategoryDevices();
    QList<AudioOutputDevice> availableAudioOutputDevices() const;
    QList<AudioCaptureDevice> availableAudioCaptureDevices() const;
    QList<VideoCaptureDevice> availableVideoCaptureDevices() const;
    DeviceType shownModelType() const;

private:
    QMap<int, AudioOutputDeviceModel *> m_audioOutputModel;
    QMap<int, AudioCaptureDeviceModel *> m_audioCaptureModel;
    QMap<int, VideoCaptureDeviceModel *> m_videoCaptureModel;
    QStandardItemModel m_categoryModel;
    QStandardItemModel m_headerModel;
    DeviceType m_testingType;

    MediaObject *m_media;
    AudioOutput *m_audioOutput;
    VideoWidget *m_videoWidget;
};

} // namespace Phonon

#endif // DEVICEPREFERENCE_H_STUPID_UIC
