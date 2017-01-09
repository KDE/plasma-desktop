/***************************************************************************
 *   Copyright (C) 2017 by Kai Uwe Broulik <kde@privat.broulik.de>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2

import org.kde.plasma.private.volume 0.1

QtObject {
    id: pulseAudio

    signal streamsChanged

    function streamsForPid(pid) {
        var streams = []
        for (var i = 0, length = instantiator.count; i < length; ++i) {
            var stream = instantiator.objectAt(i)
            if (stream.pid == pid) {
                streams.push(stream)
            }
        }
        return streams
    }

    // QtObject has no default property, hence adding the Instantiator to one explicitly.
    property var instantiator: Instantiator {
        model: PulseObjectFilterModel {
            filters: [ { role: "VirtualStream", value: false } ]
            sourceModel: SinkInputModel {}
        }

        delegate: QtObject {
            readonly property int pid: Client ? Client.properties["application.process.id"] : 0
            readonly property bool muted: Muted
            // whether there is nothing actually going on on that stream
            readonly property bool corked: Corked

            function mute() {
                Muted = true
            }
            function unmute() {
                Muted = false
            }
        }

        onObjectAdded: pulseAudio.streamsChanged()
        onObjectRemoved: pulseAudio.streamsChanged()
    }
}
