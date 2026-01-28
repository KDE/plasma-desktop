/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid

import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PC3
import org.kde.draganddrop as DragDrop
import org.kde.kirigami as Kirigami

import "LayoutManager.js" as LayoutManager

ContainmentItem {
    id: root
    width: 640
    height: 48

//BEGIN properties
    Layout.preferredWidth: fixedWidth || currentLayout.implicitWidth + currentLayout.horizontalDisplacement
    Layout.preferredHeight: fixedHeight || currentLayout.implicitHeight + currentLayout.verticalDisplacement
    Layout.fillWidth: {
        return currentLayout.children
            .filter(child => child?.applet?.plasmoid?.pluginName === "org.kde.plasma.panelspacer")
            .some(child => child.applet.plasmoid.configuration.expanding)
    }
    Layout.fillHeight: Layout.fillWidth

    property Item toolBox
    property var layoutManager: LayoutManager

    property ConfigOverlay configOverlay

    property bool isHorizontal: Plasmoid.formFactor !== PlasmaCore.Types.Vertical
    property int fixedWidth: 0
    property int fixedHeight: 0
    property bool hasSpacer
    // True when e.g. the task manager is drag and dropping tasks.
    property bool appletRequestsInhibitDnD: false
    property bool reverse: Application.layoutDirection === Qt.RightToLeft

//END properties

//BEGIN functions
    function checkLastSpacer() {
        for (var i = 0; i < appletsModel.count; ++i) {
            const applet = appletsModel.get(i).applet;
            if (!applet || !applet.visible || !applet.Layout || applet.isPlaceholder) {
                continue;
            }
            if ((isHorizontal && applet.Layout.fillWidth) ||
                (!isHorizontal && applet.Layout.fillHeight)) {
                    hasSpacer = true;
                return;
            }
        }
        hasSpacer = false;
    }

    function plasmoidLocationString(): string {
        switch (Plasmoid.location) {
        case PlasmaCore.Types.LeftEdge:
            return "west";
        case PlasmaCore.Types.TopEdge:
            return "north";
        case PlasmaCore.Types.RightEdge:
            return "east";
        case PlasmaCore.Types.BottomEdge:
            return "south";
        }
        return "";
    }
//END functions

//BEGIN connections
    Containment.onAppletAdded: (applet, geometry) => {
        let pos = Qt.point(geometry.x, geometry.y)
        if ("positionBeforeDeletion" in applet) {
            pos = applet.positionBeforeDeletion
            delete applet.positionBeforeDeletion
        }

        LayoutManager.addApplet(applet, pos.x, pos.y);
        root.checkLastSpacer();

        // When a new preset panel is added, avoid calling save() multiple times
        Qt.callLater(LayoutManager.save);
    }

    Containment.onAppletRemoved: (applet) => {
        let plasmoidItem = root.itemFor(applet);

        if (plasmoidItem) {
            appletsModel.remove(plasmoidItem.parent.index);
            applet["positionBeforeDeletion"] = Qt.point(plasmoidItem.parent.x, plasmoidItem.parent.y)
        }
        checkLastSpacer();
        LayoutManager.save();
    }

    Plasmoid.onUserConfiguringChanged: {
        if (!Plasmoid.userConfiguring) {
            if (root.configOverlay) {
                if (root.configOverlay.dragAndDropping) {
                    root.configOverlay.finishDragOperation()
                }
                root.configOverlay.destroy();
                root.configOverlay = null;
            }
            return;
        }

        if (Plasmoid.immutable) {
            return;
        }

        Containment.applets.forEach(applet => applet.expanded = false);
        const component = Qt.createComponent("ConfigOverlay.qml");
        configOverlay = component.createObject(this, {
            "anchors.fill": dropArea,
            "anchors.rightMargin": Qt.binding(() => isHorizontal ? toolBox.height : 0),
            "anchors.bottomMargin": Qt.binding(() => !isHorizontal ? toolBox.height : 0),
            "layoutManager": Qt.binding(() => root.layoutManager),
            "appletsModel": appletsModel,
            "currentLayout": currentLayout,
            "appletContainerComponent": appletContainerComponent,
            "dropArea": dropArea,
            "isHorizontal": Qt.binding(() => root.isHorizontal),
            "rootWidth": Qt.binding(() => root.width),
            "reverse": Qt.binding(() => root.reverse),
            "lastSpacer": lastSpacer,
            "addWidgetsButton": addWidgetsButton
        });
        component.destroy();
    }
//END connections

    TapHandler {
        acceptedButtons: Qt.LeftButton
        acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus
        onLongPressed: Plasmoid.internalAction("configure").trigger()
    }

    DragDrop.DropArea {
        id: dropArea
        anchors.fill: parent

        // These are invisible and only used to read panel margins
        // Both will fallback to "standard" panel margins if the theme does not
        // define a normal or a thick margin.
        KSvg.FrameSvgItem {
            id: panelSvg
            visible: false
            imagePath: "widgets/panel-background"
            prefix: [root.plasmoidLocationString(), ""]
        }
        KSvg.FrameSvgItem {
            id: thickPanelSvg
            visible: false
            prefix: ['thick'].concat(panelSvg.prefix)
            imagePath: "widgets/panel-background"
        }
        property bool marginAreasEnabled: panelSvg.margins != thickPanelSvg.margins
        property var marginHighlightSvg: KSvg.Svg{imagePath: "widgets/margins-highlight"}
        //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.
        //the size a margin should be to force a panel to be the required size above
        readonly property real spacingAtMinSize: Math.floor(Math.max(1, (root.isHorizontal ? root.height : root.width) - Kirigami.Units.iconSizes.smallMedium)/2)

        Component.onCompleted: {
            LayoutManager.plasmoid = root.Plasmoid;
            LayoutManager.root = root;
            LayoutManager.layout = currentLayout;
            LayoutManager.marginHighlights = [];
            LayoutManager.appletsModel = appletsModel;
            LayoutManager.restore();

            root.Plasmoid.internalAction("configure").visible = Qt.binding(function() {
                return !root.Plasmoid.immutable;
            });
            root.Plasmoid.internalAction("configure").enabled = Qt.binding(function() {
                return !root.Plasmoid.immutable;
            });
        }

        onDragEnter: event => {
            if (Plasmoid.immutable || root.appletRequestsInhibitDnD) {
                event.ignore();
                return;
            }
            //during drag operations we disable panel auto resize
            root.fixedWidth = root.Layout.preferredWidth
            root.fixedHeight = root.Layout.preferredHeight
            appletsModel.insert(LayoutManager.indexAtCoordinates(event.x, event.y), {applet: dndSpacer})
        }

        onDragMove: event => {
            LayoutManager.move(dndSpacer.parent, LayoutManager.indexAtCoordinates(event.x, event.y));
        }

        onDragLeave: event => {
            /*
            * When reordering task items, dragLeave signal will be emitted directly
            * without dragEnter, and in this case parent.index is undefined, so also
            * check if dndSpacer is in appletsModel.
            */
            const spacerContainer = dndSpacer.parent as AppletContainer
            if (typeof(spacerContainer?.index) === "number" && spacerContainer.index > -1) {
                appletsModel.remove(spacerContainer.index);
                root.fixedWidth = root.fixedHeight = 0;
            }
        }

        onDrop: event => {
            appletsModel.remove((dndSpacer.parent as AppletContainer).index);
            root.processMimeData(event.mimeData, event.x, event.y);
            event.accept(event.proposedAction);
            root.fixedWidth = root.fixedHeight = 0;
        }

//BEGIN components


        Component {
            id: appletContainerComponent
            // This loader conditionally manages the BusyIndicator, it's not
            // loading the applet. The applet becomes a regular child item.
            AppletContainer {
                id: container

                function getMargins(side, returnAllMargins = false, overrideFillArea = null, overrideThickArea = null): real {
                    if (!applet || !applet.Plasmoid) {
                        return 0;
                    }
                    //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon + smallSpace) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.
                    let fillArea = overrideFillArea === null ? applet && (applet.Plasmoid.constraintHints & Plasmoid.CanFillArea) : overrideFillArea
                    let inThickArea = overrideThickArea === null ? container.inThickArea : overrideThickArea
                    var layout = {
                        top: root.isHorizontal, bottom: root.isHorizontal,
                        right: !root.isHorizontal, left: !root.isHorizontal
                    };
                    return ((layout[side] || returnAllMargins) && !fillArea) ? Math.round(Math.min(dropArea.spacingAtMinSize, (inThickArea ? thickPanelSvg.fixedMargins[side] : panelSvg.fixedMargins[side]))) : 0;
                }

                Layout.topMargin: getMargins('top')
                Layout.bottomMargin: getMargins('bottom')
                Layout.leftMargin: getMargins('left')
                Layout.rightMargin: getMargins('right')

                // Always fill width/height, in order to still shrink the applet when there is not enough space.
                // When the applet doesn't want to expand set a Layout.maximumWidth accordingly
                // https://bugs.kde.org/show_bug.cgi?id=473420
                Layout.fillWidth: true
                Layout.fillHeight: true

                onWantsToFillWidthChanged: root.checkLastSpacer()
                onWantsToFillHeightChanged: root.checkLastSpacer()

                property int availWidth: root.width - Layout.leftMargin - Layout.rightMargin
                property int availHeight: root.height - Layout.topMargin - Layout.bottomMargin
                function findPositive(first, second) {return first > 0 ? first : second}

    // BEGIN BUG 454095: do not combine these expressions to a function or the bindings won't work
                Layout.minimumWidth: root.isHorizontal ? findPositive(applet?.Layout.minimumWidth, availHeight) : availWidth
                Layout.minimumHeight: !root.isHorizontal ? findPositive(applet?.Layout.minimumHeight, availWidth) : availHeight

                Layout.preferredWidth: root.isHorizontal ? findPositive(applet?.Layout.preferredWidth, Layout.minimumWidth) : availWidth
                Layout.preferredHeight: !root.isHorizontal ? findPositive(applet?.Layout.preferredHeight, Layout.minimumHeight) : availHeight

                Layout.maximumWidth: root.isHorizontal ? (wantsToFillWidth ? findPositive(applet?.Layout.maximumWidth, root.width) : Math.min(applet?.Layout.maximumWidth, Layout.preferredWidth)) : availWidth
                Layout.maximumHeight: !root.isHorizontal ? (wantsToFillHeight ? findPositive(applet?.Layout.maximumHeight, root.height) : Math.min(applet?.Layout.maximumHeight, Layout.preferredHeight)) : availHeight
    // END BUG 454095

                Item {
                    id: marginHighlightElements
                    anchors.fill: parent
                    // index -1 is for floating applets, which do not need a margin highlight
                    opacity: Plasmoid.containment.corona.editMode && dropArea.marginAreasEnabled && !(root.configOverlay?.dragAndDropping ?? false) && container.index != -1 ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuad
                        }
                    }

                    component SideMargin: KSvg.SvgItem {
                        property string side; property bool fill: true
                        property int inset; property int padding
                        property var west: ({'left': 'top', 'top': 'left', 'right': 'top', 'bottom': 'left'})
                        property var mirror: ({'left': 'right', 'top': 'bottom', 'right': 'left', 'bottom': 'top'})
                        property var onComponentCompleted: {
                            let left = west[side]
                            let right = mirror[left]
                            let up = mirror[side]
                            anchors[up] = undefined
                            if (root.isHorizontal) {
                                height = padding;
                            } else {
                                width = padding;
                            }
                            anchors[left+'Margin'] = - currentLayout.rowSpacing/2 - (container.appletIndex == 0 ? dropArea.anchors[left + 'Margin'] + currentLayout.x : 0)
                            anchors[right+'Margin'] = - currentLayout.rowSpacing/2 - (container.appletIndex == appletsModel.count-1 ? dropArea.anchors[right + 'Margin'] + currentLayout.toolBoxSize : 0)
                            anchors[side+'Margin'] = - inset
                        }
                        elementId: fill ? 'fill' : (root.isHorizontal ? side + (container.inThickArea ? 'left' : 'right') : (container.inThickArea ? 'top' : 'bottom') + side)
                        svg: dropArea.marginHighlightSvg
                        anchors {top: parent?.top; left: parent?.left; right: parent?.right; bottom: parent?.bottom}
                    }
                    Repeater {
                        model: ['top', 'bottom', 'right', 'left']
                        SideMargin {
                            required property string modelData
                            side: modelData
                            inset: container.getMargins(side)
                            visible: (modelData === 'top' || modelData === 'bottom') === root.isHorizontal
                            padding: container.getMargins(side, false, false, container.isMarginSeparator ? false : container.inThickArea)
                        }
                    }
                    Repeater {
                        model: ['top', 'bottom', 'right', 'left']
                        SideMargin {
                            required property string modelData
                            side: modelData
                            inset: -container.getMargins(side, false, false, false)
                            padding: container.getMargins(side, false, false, true) + inset
                            visible: container.isMarginSeparator && (modelData === 'top' || modelData === 'bottom') === root.isHorizontal
                            fill: false
                        }
                    }
                }

                onAppletChanged: {
                    if (applet) {
                        applet.parent = container
                        applet.anchors.fill = container
                    } else {
                        appletsModel.remove(index)
                    }
                }

                active: applet && applet.Plasmoid.busy
                sourceComponent: PC3.BusyIndicator {
                    z: 999
                }

                property int oldX: 0
                property int oldY: 0
                onXChanged: if (oldX) animateFrom(oldX, y)
                onYChanged: if (oldY) animateFrom(x, oldY)
                transform: Translate{id: translation}
                function animateFrom(xa, ya) {
                    if (root.isHorizontal) translation.x = xa - x
                    else translation.y = ya - y
                    oldX = oldY = 0
                    translAnim.running = true
                }
                NumberAnimation {
                    id: translAnim
                    duration: Kirigami.Units.shortDuration
                    easing.type: Easing.OutCubic
                    target: translation
                    properties: "x,y"
                    to: 0
                }
            }
        }
//END components

//BEGIN UI elements

        anchors {
            leftMargin: root.isHorizontal ? Math.min(dropArea.spacingAtMinSize, panelSvg.fixedMargins.left + currentLayout.rowSpacing) : 0
            rightMargin: root.isHorizontal ? Math.min(dropArea.spacingAtMinSize, panelSvg.fixedMargins.right + currentLayout.rowSpacing) : 0
            topMargin: root.isHorizontal ? 0 : Math.min(dropArea.spacingAtMinSize, panelSvg.fixedMargins.top + currentLayout.rowSpacing)
            bottomMargin: root.isHorizontal ? 0 : Math.min(dropArea.spacingAtMinSize, panelSvg.fixedMargins.bottom + currentLayout.rowSpacing)
        }

        Item {
            id: dndSpacer
            property bool busy: false
            Layout.preferredWidth: width
            Layout.preferredHeight: height
            width: root.isHorizontal ? Kirigami.Units.iconSizes.sizeForLabels * 5 : currentLayout.width
            height: root.isHorizontal ? currentLayout.height : Kirigami.Units.iconSizes.sizeForLabels * 5
        }

        ListModel {
            id: appletsModel
        }

        GridLayout {
            id: currentLayout

            Repeater {
                model: appletsModel
                delegate: appletContainerComponent
            }

            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.smallSpacing

            x: 0
            readonly property int toolBoxSize: !root.toolBox || !Plasmoid.containment.corona.editMode ? 0 : (root.isHorizontal ? root.toolBox.width : root.toolBox.height)

            property int horizontalDisplacement: dropArea.anchors.leftMargin + dropArea.anchors.rightMargin + (root.isHorizontal ? currentLayout.toolBoxSize : 0)
            property int verticalDisplacement: dropArea.anchors.topMargin + dropArea.anchors.bottomMargin + (root.isHorizontal ? 0 : currentLayout.toolBoxSize)

    // BEGIN BUG 454095: use lastSpacer to left align applets, as implicitWidth is updated too late
            width: root.width - horizontalDisplacement
            height: root.height - verticalDisplacement

            Item {
                id: lastSpacer
                visible: !root.hasSpacer
                Layout.fillWidth: true
                Layout.fillHeight: true

                /**
                * This index will be used when adding a new panel.
                *
                * @see LayoutManager.indexAtCoordinates
                */
                readonly property alias index: appletsModel.count
            }
    // END BUG 454095

            rows: root.isHorizontal ? 1 : currentLayout.children.length
            columns: root.isHorizontal ? currentLayout.children.length : 1
            flow: root.isHorizontal ? GridLayout.LeftToRight : GridLayout.TopToBottom
            layoutDirection: Application.layoutDirection
        }
    }
    MouseArea {
        anchors.fill: parent
        visible: Containment.corona.editMode && !Plasmoid.userConfiguring
        hoverEnabled: true
        onClicked: Plasmoid.internalAction("configure").trigger()
        Rectangle {
            anchors.fill: parent
            color: Kirigami.Theme.highlightColor
            opacity: 0.5
            visible: parent.containsMouse
        }
        PlasmaCore.ToolTipArea {
            id: toolTipArea
            anchors.fill: parent
            // This is to avoid the presence of mnemonics, that would
            // show the text as "&Show Panel Configuration"
            // Strip out ampersands right before non-whitespace characters, i.e.
            // those used to determine the alt key shortcut
            // (except when the word ends in ; (HTML entities))
            mainText: Plasmoid.internalAction("configure").text.replace(/(&)(?!;)\S+(?>\s)/g, "")
            icon: "configure"
        }
        Accessible.name: toolTipArea.mainText
        Accessible.description: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info:whatsthis Accessible description for entering Panel edit mode click area", "Open Panel configuration ui")
        Accessible.role: Accessible.Button
    }
    PC3.ToolButton {
        id: addWidgetsButton
        anchors.centerIn: parent
        visible: appletsModel.count === 0
        text: root.isHorizontal ? i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button opens widget explorer", "Add Widgets…") : undefined
        icon.name: "list-add-symbolic"
        onClicked: Plasmoid.internalAction("add widgets").trigger()
    }
//END UI elements
}
