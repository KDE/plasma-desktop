/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2022 Niccolò Venerandi <niccolo@venerandi.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid


Loader {
    required property Item applet
    required property int index
    property Item temporarySpacer
    property Item dragging
    property bool isAppletContainer: true
    property bool isMarginSeparator: ((applet.Plasmoid?.constraintHints & Plasmoid.MarginAreasSeparator) == Plasmoid.MarginAreasSeparator)
    property int appletIndex: index // To make sure it's always readable even inside other models
    property bool inThickArea: false
    visible: applet.Plasmoid?.status !== PlasmaCore.Types.HiddenStatus || (!Plasmoid.immutable && Plasmoid.userConfiguring) || Containment.corona.editMode;

    //when the applet moves caused by its resize, don't animate.
    //this is completely heuristic, but looks way less "jumpy"
    property bool movingForResize: false

    property bool wantsToFillWidth: applet?.Layout.fillWidth
    property bool wantsToFillHeight: applet?.Layout.fillHeight

    property int availWidth
    property int availHeight

}
