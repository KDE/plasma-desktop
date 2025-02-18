/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.pragma library

//written as a library to share knowledge of when a key was pressed
//between the multiple views, so pressing a key on one cancels all timers

var callbacks = [];

function addCancelAutoTriggerCallback(callback) {
    callbacks.push(callback);
}

function cancelAutoTrigger() {
    callbacks.forEach(function(c) {
        if (!c) {
            return;
        }
        c();
    });
}

