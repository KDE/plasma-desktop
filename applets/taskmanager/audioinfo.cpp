/*
 *  SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "audioinfo.h"

#include <QDebug>
#include <QVariant>

#include <PulseAudioQt/Client>
#include <PulseAudioQt/Context>
#include <PulseAudioQt/SinkInput>

#include <processcore/processes.h>

using namespace Qt::Literals;

QSet<QString> AudioInfo::s_pidMatches;

AudioInfo::AudioInfo()
    : QObject()
{
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkInputAdded, this, [this](PulseAudioQt::SinkInput *stream) {
        connect(stream, &PulseAudioQt::SinkInput::mutedChanged, this, &AudioInfo::update);
        connect(stream, &PulseAudioQt::SinkInput::corkedChanged, this, &AudioInfo::update);
        connect(stream, &PulseAudioQt::SinkInput::volumeChanged, this, &AudioInfo::volumeChanged);
    });
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkInputAdded, this, &AudioInfo::update);
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkInputRemoved, this, &AudioInfo::update);
}

QString AudioInfo::appId() const
{
    return m_appId;
}

void AudioInfo::setAppId(const QString &appId)
{
    m_appId = appId;

    update();
}

QString AudioInfo::appName() const
{
    return m_appName;
}

void AudioInfo::setAppName(const QString &appName)
{
    m_appName = appName;

    update();
}

qint64 AudioInfo::pid() const
{
    return m_pid;
}

void AudioInfo::setPid(qint64 pid)
{
    m_pid = pid;

    update();
}

bool AudioInfo::hasAudioStream() const
{
    return !m_streams.isEmpty();
}

bool AudioInfo::playingAudio() const
{
    return std::any_of(m_streams.cbegin(), m_streams.cend(), [](const auto *stream) {
        return !stream->isCorked();
    });
}

bool AudioInfo::muted() const
{
    return std::all_of(m_streams.cbegin(), m_streams.cend(), [](const auto *stream) {
        return stream->isMuted();
    });
}

void AudioInfo::setMuted(bool muted)
{
    for (auto *stream : std::as_const(m_streams)) {
        stream->setMuted(muted);
    }
}

qint64 AudioInfo::volume() const
{
    const qint64 maxVolume = std::accumulate(m_streams.cbegin(), m_streams.cend(), 0, [](qint64 volume, PulseAudioQt::SinkInput *stream) {
        return std::max(volume, stream->volume());
    });

    return ((float)maxVolume / PulseAudioQt::normalVolume()) * 100;
}

void AudioInfo::setVolume(qint64 v)
{
    const qint64 targetVolume = std::clamp<qint64>(v, 0, 100) * PulseAudioQt::normalVolume() / 100;

    const qint64 maxVolume = std::accumulate(m_streams.cbegin(), m_streams.cend(), 0, [](qint64 volume, PulseAudioQt::SinkInput *stream) {
        return std::max(volume, stream->volume());
    });

    for (auto *stream : std::as_const(m_streams)) {
        qint64 streamVolume = targetVolume;
        if (targetVolume > 0 && maxVolume > 0) { // prevent divide by 0
            // adjust volume relative to the loudest stream
            streamVolume = stream->volume() / maxVolume * targetVolume;
        }
        stream->setVolume(streamVolume);
    }
}

void AudioInfo::update()
{
    m_streams.clear();

    m_streams = streamsForAppId(m_appId);

    if (m_streams.isEmpty()) {
        m_streams = streamsForPid(m_pid);

        if (m_streams.isEmpty()) {
            // We only want to fall back to appName matching if we never managed to map
            // a PID to an audio stream window. Otherwise if you have two instances of
            // an application, one playing and the other not, it will look up appName
            // for the non-playing instance and erroneously show an indicator on both.
            if (!s_pidMatches.contains(m_appName)) {
                m_streams = streamsForAppName(m_appName);
            }
        } else {
            s_pidMatches << m_appName;
        }
    }

    Q_EMIT updated();
    Q_EMIT volumeChanged();
}

QList<PulseAudioQt::SinkInput *> AudioInfo::streamsForAppId(const QString &appId)
{
    QList<PulseAudioQt::SinkInput *> ret;
    const auto streams = PulseAudioQt::Context::instance()->sinkInputs();
    for (auto *stream : streams) {
        if (stream->client()->properties().value(u"pipewire.access.portal.app_id"_s) == appId) {
            ret << stream;
        }
    }

    return ret;
}

QList<PulseAudioQt::SinkInput *> AudioInfo::streamsForPid(qint64 pid)
{
    QList<PulseAudioQt::SinkInput *> ret;

    const auto streams = PulseAudioQt::Context::instance()->sinkInputs();
    for (auto *stream : streams) {
        // skip stream that has portalAppId
        // app using portal may have a sandbox pid
        if (!stream->client()->properties().value(u"pipewire.access.portal.app_id"_s).toString().isEmpty()) {
            continue;
        }

        const int streamPid = stream->client()->properties()[u"application.process.id"_s].toInt();

        if (streamPid == pid) {
            ret << stream;
            continue;
        }

        if (parentPid(streamPid) == pid) {
            ret << stream;
            continue;
        }
    }

    return ret;
}

QList<PulseAudioQt::SinkInput *> AudioInfo::streamsForAppName(const QString &appName)
{
    QList<PulseAudioQt::SinkInput *> ret;
    const auto streams = PulseAudioQt::Context::instance()->sinkInputs();
    for (auto *stream : streams) {
        if (stream->client()->properties().value(u"application.name"_s) == appName) {
            ret << stream;
        }
    }
    return ret;
}

qint64 AudioInfo::parentPid(qint64 pid) const
{
    KSysGuard::Processes procs;
    procs.updateOrAddProcess(pid);

    KSysGuard::Process *proc = procs.getProcess(pid);
    if (!proc) {
        return -1;
    }

    int parentPid = proc->parentPid();
    if (parentPid != -1) {
        procs.updateOrAddProcess(parentPid);

        KSysGuard::Process *parentProc = procs.getProcess(parentPid);
        if (!parentProc) {
            return -1;
        }

        if (!proc->cGroup().isEmpty() && parentProc->cGroup() == proc->cGroup()) {
            return parentProc->pid();
        }
    }

    return -1;
}
