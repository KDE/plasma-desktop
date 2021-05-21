/*
    SPDX-FileCopyrightText: 2020 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0

BaseView {
    id: placesView
    signal sectionTrigger(int index)
    objectName: "PlacesView"
    isExternalManagerMode: true
    model: placesViewModel
    focus: true
    Connections {
        target: placesViewModel
        function onTrigger(index) {
            placesView.listView.currentIndex = index
            placesView.sectionTrigger(index)
        }
    }
}
