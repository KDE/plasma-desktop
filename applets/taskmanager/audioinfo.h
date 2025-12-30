/*
 *  SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "qqmlregistration.h"
#include <QObject>
#include <QQmlParserStatus>

#include <PulseAudioQt/SinkInput>

class AudioInfo : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    // input
    Q_PROPERTY(QString appId READ appId WRITE setAppId NOTIFY appIdChanged FINAL)
    Q_PROPERTY(qint64 pid READ pid WRITE setPid NOTIFY pidChanged FINAL)
    Q_PROPERTY(QString appName READ appName WRITE setAppName NOTIFY appNameChanged FINAL)

    // output
    Q_PROPERTY(bool hasAudioStream READ hasAudioStream NOTIFY updated FINAL)
    Q_PROPERTY(bool playingAudio READ playingAudio NOTIFY updated FINAL)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged FINAL)
    Q_PROPERTY(qint64 volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL) // normalized to 0-100

public:
    AudioInfo();

    QString appId() const;
    void setAppId(const QString &appId);
    Q_SIGNAL void appIdChanged();

    QString appName() const;
    void setAppName(const QString &appName);
    Q_SIGNAL void appNameChanged();

    qint64 pid() const;
    void setPid(qint64 pid);
    Q_SIGNAL void pidChanged();

    bool hasAudioStream() const;

    bool playingAudio() const;

    bool muted() const;
    void setMuted(bool muted);
    Q_SIGNAL void mutedChanged();

    qint64 volume() const;
    void setVolume(qint64 volume);
    Q_SIGNAL void volumeChanged();

    Q_SIGNAL void updated();

    void classBegin() override;
    void componentComplete() override;

private:
    void update();
    QList<PulseAudioQt::SinkInput *> streamsForAppId(const QString &appId) const;
    QList<PulseAudioQt::SinkInput *> streamsForPid(qint64 pid) const;
    QList<PulseAudioQt::SinkInput *> streamsForAppName(const QString &appName) const;
    qint64 parentPid(qint64 pid) const;

    QString m_appId;
    QString m_appName;
    qint64 m_pid;
    bool m_complete = false;

    QList<PulseAudioQt::SinkInput *> m_streams;

    static QSet<QString> s_pidMatches;
};
