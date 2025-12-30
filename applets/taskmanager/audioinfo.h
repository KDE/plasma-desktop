/*
 *  SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "qqmlregistration.h"
#include <QObject>

#include <PulseAudioQt/SinkInput>

class AudioInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // input
    Q_PROPERTY(QString appId READ appId WRITE setAppId NOTIFY updated FINAL)
    Q_PROPERTY(qint64 pid READ pid WRITE setPid NOTIFY updated FINAL)
    Q_PROPERTY(QString appName READ appName WRITE setAppName NOTIFY updated FINAL)

    // output
    Q_PROPERTY(bool hasAudioStream READ hasAudioStream NOTIFY updated FINAL)
    Q_PROPERTY(bool playingAudio READ playingAudio NOTIFY updated FINAL)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY updated FINAL)
    Q_PROPERTY(qint64 volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL) // normalized to 0-100

public:
    AudioInfo();

    QString appId() const;
    void setAppId(const QString &appId);

    QString appName() const;
    void setAppName(const QString &appName);

    qint64 pid() const;
    void setPid(qint64 pid);

    bool hasAudioStream() const;

    bool playingAudio() const;

    bool muted() const;
    void setMuted(bool muted);

    qint64 volume() const;
    void setVolume(qint64 volume);
    Q_SIGNAL void volumeChanged();

    Q_SIGNAL void updated();

private:
    void update();
    QList<PulseAudioQt::SinkInput *> streamsForAppId(const QString &appId);
    QList<PulseAudioQt::SinkInput *> streamsForPid(qint64 pid);
    QList<PulseAudioQt::SinkInput *> streamsForAppName(const QString &appName);
    qint64 parentPid(qint64 pid) const;

    QString m_appId;
    QString m_appName;
    qint64 m_pid;

    QList<PulseAudioQt::SinkInput *> m_streams;

    static QSet<QString> s_pidMatches;
};
