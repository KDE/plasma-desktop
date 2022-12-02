/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.private.volume 0.1

QtObject {
    id: pulseAudio

    signal streamsChanged

    // It's a JS object so we can do key lookup and don't need to take care of filtering duplicates.
    property var pidMatches: ({})

    // TODO Evict cache at some point, preferably if all instances of an application closed.
    function registerPidMatch(appName) {
        if (!hasPidMatch(appName)) {
            pidMatches[appName] = true;

            // In case this match is new, notify that streams might have changed.
            // This way we also catch the case when the non-playing instance
            // shows up first.
            // Only notify if we changed to avoid infinite recursion.
            streamsChanged();
        }
    }

    function hasPidMatch(appName) {
        return pidMatches[appName] === true;
    }

    function findStreams(key, value) {
        return findStreamsFn(stream => stream[key] === value);
    }

    function findStreamsFn(fn) {
        var streams = []
        for (var i = 0, length = instantiator.count; i < length; ++i) {
            var stream = instantiator.objectAt(i);
            if (fn(stream)) {
                streams.push(stream);
            }
        }
        return streams;
    }

    function streamsForAppId(appId) {
        return findStreams("portalAppId", appId);
    }

    function streamsForAppName(appName) {
        return findStreams("appName", appName);
    }

    function streamsForPid(pid) {
        // skip stream that has portalAppId
        // app using portal may have a sandbox pid
        var streams = findStreamsFn(stream => stream.pid === pid && !stream.portalAppId);

        if (streams.length === 0) {
            for (var i = 0, length = instantiator.count; i < length; ++i) {
                var stream = instantiator.objectAt(i);

                if (stream.parentPid === -1) {
                    stream.parentPid = backend.parentPid(stream.pid);
                }

                if (stream.parentPid === pid) {
                    streams.push(stream);
                }
            }
        }

        return streams;
    }

    // QtObject has no default property, hence adding the Instantiator to one explicitly.
    property var instantiator: Instantiator {
        model: PulseObjectFilterModel {
            filters: [ { role: "VirtualStream", value: false } ]
            sourceModel: SinkInputModel {}
        }

        delegate: QtObject {
            id: delegate
            required property var model
            readonly property int pid: model.Client ? model.Client.properties["application.process.id"] : 0
            // Determined on demand.
            property int parentPid: -1
            readonly property string appName: model.Client && model.Client.properties["application.name"] || ""
            readonly property string portalAppId: model.Client && model.Client.properties["pipewire.access.portal.app_id"] || ""
            readonly property bool muted: model.Muted
            // whether there is nothing actually going on on that stream
            readonly property bool corked: model.Corked
            readonly property int volume: model.Volume

            function mute() {
                model.Muted = true
            }
            function unmute() {
                model.Muted = false
            }
        }

        onObjectAdded: pulseAudio.streamsChanged()
        onObjectRemoved: pulseAudio.streamsChanged()
    }

    readonly property int minimalVolume: PulseAudio.MinimalVolume
    readonly property int normalVolume: PulseAudio.NormalVolume
}
