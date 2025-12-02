/*
    SPDX-FileCopyrightText: 2012 Luís Gabriel Lima <lampih@gmail.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.draganddrop as DnD
import org.kde.plasma.private.pager
import org.kde.plasma.activityswitcher as ActivitySwitcher
import org.kde.kirigami as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

PlasmoidItem {
    id: root

    readonly property bool isActivityPager: Plasmoid.pluginName === "org.kde.plasma.activitypager"
    readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    readonly property real aspectRatio: (((pagerModel.pagerItemSize.width * pagerItemGrid.effectiveColumns)
        + ((pagerItemGrid.effectiveColumns * pagerItemGrid.spacing) - pagerItemGrid.spacing))
        / ((pagerModel.pagerItemSize.height * pagerItemGrid.effectiveRows)
        + ((pagerItemGrid.effectiveRows * pagerItemGrid.spacing) - pagerItemGrid.spacing)))

    Layout.minimumWidth: !root.vertical ? Math.floor(height * aspectRatio) : 1
    Layout.minimumHeight: root.vertical ? Math.floor(width / aspectRatio) : 1

    Layout.maximumWidth: !root.vertical ? Math.floor(height * aspectRatio) : Infinity
    Layout.maximumHeight: root.vertical ? Math.floor(width / aspectRatio) : Infinity

    Plasmoid.status: pagerModel.shouldShowPager ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

    Layout.fillWidth: root.vertical
    Layout.fillHeight: !root.vertical

    property bool dragging: false
    property string dragId

    property int dragSwitchDesktopIndex: -1

    property int wheelDelta: 0

    function colorWithAlpha(color: color, alpha: real): color {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    readonly property color windowActiveOnActiveDesktopColor: colorWithAlpha(Kirigami.Theme.textColor, 0.6)
    readonly property color windowInactiveOnActiveDesktopColor: colorWithAlpha(Kirigami.Theme.textColor, 0.35)
    readonly property color windowActiveColor: colorWithAlpha(Kirigami.Theme.textColor, 0.5)
    readonly property color windowActiveBorderColor: Kirigami.Theme.textColor
    readonly property color windowInactiveColor: colorWithAlpha(Kirigami.Theme.textColor, 0.17)
    readonly property color windowInactiveBorderColor: colorWithAlpha(Kirigami.Theme.textColor, 0.5)

    function sanitize(input: string): string {
        // Based on QQuickStyledTextPrivate::parseEntity
        const table = {
            '>': '&gt;',
            '<': '&lt;',
            '&': '&amp;',
            "'": '&apos;',
            '"': '&quot;',
            '\u00a0': '&nbsp;',
        };
        return input.replace(/[<>&'"\u00a0]/g, c => table[c]);
    }

    function generateWindowList(windows) {
        // if we have 5 windows, we would show "4 and another one" with the
        // hint that there's 1 more taking the same amount of space than just showing it
        const maximum = windows.length === 5 ? 5 : 4

        let text = "<ul><li>"
            + windows.slice(0, maximum).map(sanitize).join("</li><li>")
            + "</li></ul>";

        if (windows.length > maximum) {
            text += i18np("…and %1 other window", "…and %1 other windows", windows.length - maximum) // qmllint disable unqualified
        }

        return text
    }

    MouseArea {
        id: rootMouseArea
        anchors.fill: parent

        acceptedButtons: Qt.NoButton
        hoverEnabled: true

        onWheel: wheel => {
            // Magic number 120 for common "one click, see:
            // https://doc.qt.io/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
            root.wheelDelta += wheel.angleDelta.y || wheel.angleDelta.x;

            let increment = 0;

            while (root.wheelDelta >= 120) {
                root.wheelDelta -= 120;
                increment++;
            }

            while (root.wheelDelta <= -120) {
                root.wheelDelta += 120;
                increment--;
            }

            while (increment !== 0) {
                if (increment < 0) {
                    const nextPage = Plasmoid.configuration.wrapPage?
                        (pagerModel.currentPage + 1) % repeater.count :
                        Math.min(pagerModel.currentPage + 1, repeater.count - 1);
                    pagerModel.changePage(nextPage);
                } else {
                    const previousPage = Plasmoid.configuration.wrapPage ?
                        (repeater.count + pagerModel.currentPage - 1) % repeater.count :
                        Math.max(pagerModel.currentPage - 1, 0);
                    pagerModel.changePage(previousPage);
                }

                increment += (increment < 0) ? 1 : -1;
                root.wheelDelta = 0;
            }
        }
    }

    PagerModel {
        id: pagerModel

        enabled: root.visible

        showDesktop: (Plasmoid.configuration.currentDesktopSelected === 1)

        showOnlyCurrentScreen: Plasmoid.configuration.showOnlyCurrentScreen
        screenGeometry: Plasmoid.containment.screenGeometry

        pagerType: root.isActivityPager ? PagerModel.Activities : PagerModel.VirtualDesktops
    }

    Connections {
        target: Plasmoid.configuration

        function onShowWindowIconsChanged() {
            // Causes the model to reset; Component.onCompleted in the
            // window delegate now gets a chance to create the icon item,
            // which it otherwise will not do.
            pagerModel.refresh();
        }

        function onDisplayedTextChanged() {
            // Causes the model to reset; Component.onCompleted in the
            // desktop delegate now gets a chance to create the label item,
            // which it otherwise will not do.
            pagerModel.refresh();
        }
    }

    Component {
        id: desktopLabelComponent

        PlasmaComponents3.Label {
            required property int index
            required property string display
            required property KSvg.FrameSvgItem desktopFrame

            anchors {
                fill: parent
                topMargin: desktopFrame.margins.top
                leftMargin: desktopFrame.margins.left
                rightMargin: desktopFrame.margins.right
                bottomMargin: desktopFrame.margins.bottom
            }

            text: Plasmoid.configuration.displayedText ? display : index + 1
            textFormat: Text.PlainText

            wrapMode: Text.NoWrap
            elide: Text.ElideRight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.pixelSize: Math.min(height, Kirigami.Theme.defaultFont.pixelSize)

            z: 9999 // The label goes above everything
        }
    }

    Component {
        id: windowIconComponent

        Kirigami.Icon {
            anchors.centerIn: parent

            height: Math.min(Kirigami.Units.iconSizes.small,
                             parent.height,
                             Math.max(parent.height - (Kirigami.Units.smallSpacing * 2),
                                      Kirigami.Units.smallSpacing * 2))
            width: Math.min(Kirigami.Units.iconSizes.small,
                            parent.width,
                            Math.max(parent.width - (Kirigami.Units.smallSpacing * 2),
                                     Kirigami.Units.smallSpacing * 2))

            required property var decoration//: null

            source: decoration ?? undefined
            roundToIconSize: false
            animated: false
        }
    }

    Timer {
        id: dragTimer
        interval: 1000
        onTriggered: {
            if (root.dragSwitchDesktopIndex !== -1 && root.dragSwitchDesktopIndex !== pagerModel.currentPage) {
                pagerModel.changePage(root.dragSwitchDesktopIndex);
            }
        }
    }
    onDragSwitchDesktopIndexChanged: if (root.dragSwitchDesktopIndex === -1) {
        dragTimer.stop();
    } else {
        dragTimer.restart();
    }

    Grid {
        id: pagerItemGrid

        anchors.centerIn: parent
        spacing: 1
        rows: effectiveRows
        columns: effectiveColumns

        z: 1

        readonly property int effectiveRows: {
            if (!pagerModel.count) {
                return 1;
            }

            let rows = 1;

            if (root.isActivityPager && Plasmoid.configuration.pagerLayout !== 0 /*No Default*/) {
                if (Plasmoid.configuration.pagerLayout === 1 /*Horizontal*/) {
                    rows = 1;
                } else if (Plasmoid.configuration.pagerLayout === 2 /*Vertical*/) {
                    rows = pagerModel.count;
                }
            } else {
                let columns = Math.floor(pagerModel.count / pagerModel.layoutRows);

                if (pagerModel.count % pagerModel.layoutRows > 0) {
                    columns += 1;
                }

                rows = Math.floor(pagerModel.count / columns);

                if (pagerModel.count % columns > 0) {
                    rows += 1;
                }
            }

            return rows;
        }

        readonly property int effectiveColumns: {
            if (!pagerModel.count) {
                return 1;
            }

            return Math.ceil(pagerModel.count / effectiveRows);
        }

        readonly property real pagerItemSizeRatio: pagerModel.pagerItemSize.width / pagerModel.pagerItemSize.height
        readonly property real widthScaleFactor: columnWidth / pagerModel.pagerItemSize.width
        readonly property real heightScaleFactor: rowHeight / pagerModel.pagerItemSize.height

        states: [
            State {
                name: "vertical"
                when: root.vertical
                PropertyChanges {
                    pagerItemGrid.innerSpacing: pagerItemGrid.effectiveColumns
                    pagerItemGrid.rowHeight: Math.floor(pagerItemGrid.columnWidth / pagerItemGrid.pagerItemSizeRatio)
                    pagerItemGrid.columnWidth: Math.floor((root.width - pagerItemGrid.innerSpacing) / pagerItemGrid.effectiveColumns)
                }
            }
        ]

        property int innerSpacing: (effectiveRows - 1) * spacing
        property int rowHeight: Math.floor((root.height - innerSpacing) / effectiveRows)
        property int columnWidth: Math.floor(rowHeight * pagerItemSizeRatio)

        Repeater {
            id: repeater

            model: pagerModel

            component DesktopDelegate: PlasmaCore.ToolTipArea {
                required property string display
                required property int index
                required property var model

                readonly property /*WindowModel*/ var tasksModel: model.TasksModel
                readonly property string desktopId: root.isActivityPager ? tasksModel.activity : tasksModel.virtualDesktop
                readonly property bool active: (index === pagerModel.currentPage)
            }

            delegate: DesktopDelegate {
                id: desktop

                mainText: display
                // our ToolTip has maximumLineCount of 8 which doesn't fit but QML doesn't
                // respect that in RichText so we effectively can put in as much as we like :)
                // it also gives us more flexibility when it comes to styling the <li>
                textFormat: Text.RichText

                function updateSubTextIfNeeded() {
                    if (!containsMouse) {
                        return;
                    }

                    let text = ""
                    let visibleWindows = []
                    let minimizedWindows = []

                    for (let i = 0, length = windowRectRepeater.count; i < length; ++i) {
                        const window = windowRectRepeater.itemAt(i) as WindowDelegate
                        if (window) {
                            if (window.minimized) {
                                minimizedWindows.push(window.display)
                            } else {
                                visibleWindows.push(window.display)
                            }
                        }
                    }

                    if (visibleWindows.length === 1) {
                        text += visibleWindows[0]
                    } else if (visibleWindows.length > 1) {
                        text += i18np("%1 Window:", "%1 Windows:", visibleWindows.length) // qmllint disable unqualified
                            + root.generateWindowList(visibleWindows)
                    }

                    if (visibleWindows.length && minimizedWindows.length) {
                        if (visibleWindows.length === 1) {
                            text += "<br>"
                        }
                        text += "<br>"
                    }

                    if (minimizedWindows.length > 0) {
                        text += i18np("%1 Minimized Window:", "%1 Minimized Windows:", minimizedWindows.length) // qmllint disable unqualified
                            + root.generateWindowList(minimizedWindows)
                    }

                    if (text.length) {
                        // Get rid of the spacing <ul> would cause
                        text = "<style>ul { margin: 0; }</style>" + text
                    }

                    subText = text
                }

                width: pagerItemGrid.columnWidth
                height: pagerItemGrid.rowHeight

                // These states match the set of SVG prefixes for the "widgets/pager" below.
                state: {
                    if ((desktopMouseArea.enabled && (desktopMouseArea.containsMouse || desktopMouseArea.activeFocus))
                            || (root.dragging && root.dragId === desktopId)) {
                        return "hover";
                    } else if (active) {
                        return "active";
                    } else {
                        return "normal";
                    }
                }

                component PagerFrame : KSvg.FrameSvgItem {
                    anchors.fill: parent
                    imagePath: "widgets/pager"
                    opacity: desktop.state === usedPrefix ? 1 : 0
                }

                PagerFrame {
                    id: desktopFrame
                    z: 2 // Above window outlines, but below label
                    prefix: "hover"
                }
                PagerFrame {
                    z: 3
                    prefix: "active"
                }
                PagerFrame {
                    z: 4
                    prefix: "normal"
                }

                DnD.DropArea {
                    id: droparea
                    anchors.fill: parent
                    preventStealing: true

                    onDragEnter: event => {
                        root.dragSwitchDesktopIndex = desktop.index;
                    }
                    onDragLeave: event => {
                        // new onDragEnter may happen before an old onDragLeave
                        if (root.dragSwitchDesktopIndex === desktop.index) {
                            root.dragSwitchDesktopIndex = -1;
                        }
                    }
                    onDrop: event => {
                        pagerModel.drop(event.mimeData, event.modifiers, desktop.desktopId);
                        root.dragSwitchDesktopIndex = -1;
                    }
                }

                MouseArea {
                    id: desktopMouseArea

                    anchors.fill: parent
                    hoverEnabled: true
                    activeFocusOnTab: true
                    onClicked: mouse => {
                        pagerModel.changePage(desktop.index);
                    }
                    Accessible.name: Plasmoid.configuration.displayedText ? desktop.display : i18n("Desktop %1", (desktop.index + 1)) // qmllint disable unqualified
                    Accessible.description: Plasmoid.configuration.displayedText ? i18nc("@info:tooltip %1 is the name of a virtual desktop or an activity", "Switch to %1", desktop.display) : i18nc("@info:tooltip %1 is the name of a virtual desktop or an activity", "Switch to %1", (desktop.index + 1)) // qmllint disable unqualified
                    Accessible.role: Accessible.Button
                    Keys.onPressed: event => {
                        switch (event.key) {
                        case Qt.Key_Space:
                        case Qt.Key_Enter:
                        case Qt.Key_Return:
                        case Qt.Key_Select:
                            pagerModel.changePage(desktop.index);
                            break;
                        }
                    }
                }

                Item {
                    id: clipRect

                    x: 1
                    y: 1
                    z: 1 // Below FrameSvg
                    width: desktop.width - 2 * x
                    height: desktop.height - 2 * y

                    Repeater {
                        id: windowRectRepeater

                        model: desktop.tasksModel

                        onCountChanged: desktop.updateSubTextIfNeeded()

                        component WindowDelegate: Rectangle {
                            required property string display
                            required property var model
                            required property var decoration
                            required property int index

                            // These can't be required due to their role names
                            readonly property rect geometry: model.Geometry
                            readonly property bool minimized: model.IsMinimized
                            readonly property bool isActive: model.IsActive
                            readonly property int stackingOrder: model.StackingOrder
                        }

                        delegate: WindowDelegate {
                            id: windowRect

                            onMinimizedChanged: desktop.updateSubTextIfNeeded()
                            onDisplayChanged: desktop.updateSubTextIfNeeded()

                            z: 1 + stackingOrder
                            /* since we move clipRect with 1, move it back */
                            x: Math.round(geometry.x * pagerItemGrid.widthScaleFactor) - 1
                            y: Math.round(geometry.y * pagerItemGrid.heightScaleFactor) - 1
                            width: Math.round(geometry.width * pagerItemGrid.widthScaleFactor)
                            height: Math.round(geometry.height * pagerItemGrid.heightScaleFactor)
                            visible: Plasmoid.configuration.showWindowOutlines && !minimized
                            color: {
                                if (desktop.active) {
                                    if (isActive) {
                                        return root.windowActiveOnActiveDesktopColor;
                                    } else {
                                        return root.windowInactiveOnActiveDesktopColor;
                                    }
                                } else {
                                    if (isActive) {
                                        return root.windowActiveColor;
                                    } else {
                                        return root.windowInactiveColor;
                                    }
                                }
                            }

                            border.width: 1
                            border.color: isActive
                                ? root.windowActiveBorderColor
                                : root.windowInactiveBorderColor

                            MouseArea {
                                id: windowMouseArea
                                anchors.fill: parent

                                drag.target: windowRect
                                drag.threshold: 1
                                drag.axis: Drag.XAndYAxis
                                drag.minimumX: 0
                                drag.maximumX: root.width - windowRect.width
                                drag.minimumY: 0
                                drag.maximumY: root.height - windowRect.height

                                drag.onActiveChanged: {
                                    root.dragging = drag.active;
                                    root.dragId = desktop.desktopId;
                                    desktopMouseArea.enabled = !drag.active;

                                    if (drag.active) {
                                        // Reparent to allow drags outside of this desktop.
                                        const value = root.mapFromItem(clipRect, windowRect.x, windowRect.y);
                                        windowRect.parent = root;
                                        windowRect.x = value.x;
                                        windowRect.y = value.y;
                                    }
                                }

                                onReleased: mouse => {
                                    if (root.dragging) {
                                        windowRect.visible = false;
                                        const windowCenter = Qt.point(windowRect.x + windowRect.width / 2, windowRect.y + windowRect.height / 2);
                                        const pagerItem = pagerItemGrid.childAt(windowCenter.x, windowCenter.y) as DesktopDelegate;

                                        if (pagerItem) {
                                            const relativeTopLeft = root.mapToItem(pagerItem, windowRect.x, windowRect.y);

                                            const modelIndex = windowRectRepeater.model.index(windowRect.index, 0)
                                            pagerModel.moveWindow(modelIndex, relativeTopLeft.x, relativeTopLeft.y,
                                                pagerItem.desktopId, root.dragId,
                                                pagerItemGrid.widthScaleFactor, pagerItemGrid.heightScaleFactor);
                                        }

                                        // Will reset the model, destroying the reparented drag delegate that
                                        // is no longer bound to model.Geometry.
                                        root.dragging = false;
                                    } else {
                                        // When there is no dragging (just a click), the event is passed
                                        // to the desktop MouseArea.
                                        desktopMouseArea.clicked(mouse);
                                    }
                                    pagerModel.refresh();
                                }
                            }

                            Component.onCompleted: {
                                if (Plasmoid.configuration.showWindowIcons) {
                                    windowIconComponent.createObject(windowRect, { decoration: windowRect.decoration });
                                }
                            }
                        }
                    }
                }

                Component.onCompleted: {
                    if (Plasmoid.configuration.displayedText < 2) {
                        desktopLabelComponent.createObject(desktop, { index, display: desktop.display, desktopFrame });
                    }
                }

                onContainsMouseChanged: updateSubTextIfNeeded()
            }
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action:inmenu widget context menu", "Show Activity Manager") // qmllint disable unqualified
            icon.name: "activities"
            visible: root.isActivityPager
            onTriggered: ActivitySwitcher.Backend.toggleActivityManager()
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu widget context menu", "Add Virtual Desktop") // qmllint disable unqualified
            icon.name: "list-add"
            visible: !root.isActivityPager && KConfig.KAuthorized.authorize("kcm_kwin_virtualdesktops")
            onTriggered: pagerModel.addDesktop()
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu widget context menu", "Remove Virtual Desktop") // qmllint disable unqualified
            icon.name: "list-remove"
            visible: !root.isActivityPager && KConfig.KAuthorized.authorize("kcm_kwin_virtualdesktops")
            enabled: repeater.count > 1
            onTriggered: pagerModel.removeDesktop()
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu widget context menu", "&Configure Activities…") // qmllint disable unqualified
            visible: root.isActivityPager && KConfig.KAuthorized.authorize("kcm_activities")
            onTriggered: KCM.KCMLauncher.openSystemSettings("kcm_activities")
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu widget context menu", "Configure Virtual Desktops…") // qmllint disable unqualified
            visible: !root.isActivityPager && KConfig.KAuthorized.authorize("kcm_kwin_virtualdesktops")
            onTriggered: {
                if (Qt.platform.pluginName.includes("wayland"))
                    KCM.KCMLauncher.openSystemSettings("kcm_kwin_virtualdesktops")
                else
                    KCM.KCMLauncher.openSystemSettings("kcm_kwin_virtualdesktops_x11")
            }
        }
    ]
}
