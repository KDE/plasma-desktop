/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.private.volume

QtObject {
    id: pulseAudio

    signal streamsChanged()

    // It's a JS object so we can do key lookup and don't need to take care of filtering duplicates.
    property var pidMatches: new Set()

    // TODO Evict cache at some point, preferably if all instances of an application closed.
    function registerPidMatch(appName: string) {
        if (!hasPidMatch(appName)) {
            pidMatches.add(appName);

            // In case this match is new, notify that streams might have changed.
            // This way we also catch the case when the non-playing instance
            // shows up first.
            // Only notify if we changed to avoid infinite recursion.
            streamsChanged();
        }
    }

    function hasPidMatch(appName: string): bool {
        return pidMatches.has(appName);
    }

    function findStreams(key: string, value: var): /*[QtObject]*/ var {
        return findStreamsFn(stream => stream[key] === value);
    }

    function findStreamsFn(fn: var): var {
        const streams = [];
        for (let i = 0, count = instantiator.count; i < count; ++i) {
            const stream = instantiator.objectAt(i);
            if (fn(stream)) {
                streams.push(stream);
            }
        }
        return streams;
    }

    function streamsForAppId(appId: string): /*[QtObject]*/ var {
        return findStreams("portalAppId", appId);
    }

    function streamsForAppName(appName: string): /*[QtObject]*/ var {
        return findStreams("appName", appName);
    }

    function streamsForPid(pid: int): /*[QtObject]*/ var {
        // skip stream that has portalAppId
        // app using portal may have a sandbox pid
        const streams = findStreamsFn(stream => stream.pid === pid && !stream.portalAppId);

        if (streams.length === 0) {
            for (let i = 0, length = instantiator.count; i < length; ++i) {
                const stream = instantiator.objectAt(i);

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
    readonly property Instantiator instantiator: Instantiator {
        model: PulseObjectFilterModel {
            filters: [ { role: "VirtualStream", value: false } ]
            sourceModel: SinkInputModel {}
        }

        delegate: QtObject {
            id: delegate
            required property var model
            readonly property int pid: model.Client?.properties["application.process.id"] ?? 0
            // Determined on demand.
            property int parentPid: -1
            readonly property string appName: model.Client?.properties["application.name"] ?? ""
            readonly property string portalAppId: model.Client?.properties["pipewire.access.portal.app_id"] ?? ""
            readonly property bool muted: model.Muted
            // whether there is nothing actually going on on that stream
            readonly property bool corked: model.Corked
            readonly property int volume: model.Volume

            function mute(): void {
                model.Muted = true;
            }
            function unmute(): void {
                model.Muted = false;
            }
        }

        onObjectAdded: (index, object) => pulseAudio.streamsChanged()
        onObjectRemoved: (index, object) => pulseAudio.streamsChanged()
    }

    readonly property int minimalVolume: PulseAudio.MinimalVolume
    readonly property int normalVolume: PulseAudio.NormalVolume
}
