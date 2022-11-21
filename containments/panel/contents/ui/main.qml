/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop

import "LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    width: 640
    height: 48

//BEGIN properties
    Layout.preferredWidth: fixedWidth || currentLayout.implicitWidth
    Layout.preferredHeight: fixedHeight || currentLayout.implicitHeight

    property Item toolBox
    property var layoutManager: LayoutManager

    property Item configOverlay

    property bool isHorizontal: plasmoid.formFactor !== PlasmaCore.Types.Vertical
    property int fixedWidth: 0
    property int fixedHeight: 0
    property bool hasSpacer
    // True when a widget is being drag and dropped within the panel.
    property bool dragAndDropping: false

    // These are invisible and only used to read panel margins
    // Both will fallback to "standard" panel margins if the theme does not
    // define a normal or a thick margin.
    PlasmaCore.FrameSvgItem {
        id: panelSvg
        visible: false
        prefix: [{[PlasmaCore.Types.LeftEdge]: 'west', [PlasmaCore.Types.TopEdge]: 'north',
                 [PlasmaCore.Types.RightEdge]: 'east', [PlasmaCore.Types.BottomEdge]: 'south'
        }[plasmoid.location], ""]
        imagePath: "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: ['thick'].concat(panelSvg.prefix)
        imagePath: "widgets/panel-background"
    }
    property bool marginAreasEnabled: panelSvg.margins != thickPanelSvg.margins
    property var marginHighlightSvg: PlasmaCore.Svg{imagePath: "widgets/margins-highlight"}
    //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.
    //the size a margin should be to force a panel to be the required size above
    readonly property real spacingAtMinSize: Math.floor(Math.max(1, (isHorizontal ? root.height : root.width) - PlasmaCore.Units.iconSizes.smallMedium)/2)

//END properties

//BEGIN functions
function checkLastSpacer() {
    var flexibleFound = false;
    for (var i = 0; i < appletsModel.count; ++i) {
        const applet = appletsModel.get(i).applet;
        if (!applet || !applet.visible || !applet.Layout) {
            continue;
        }
        if ((root.isHorizontal && applet.Layout.fillWidth) ||
            (!root.isHorizontal && applet.Layout.fillHeight)) {
                hasSpacer = true;
            return
        }
    }
    hasSpacer = false;
}
//END functions

//BEGIN connections
    Component.onCompleted: {
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = currentLayout;
        LayoutManager.marginHighlights = [];
        LayoutManager.appletsModel = appletsModel;
        LayoutManager.restore();

        plasmoid.action("configure").visible = Qt.binding(function() {
            return !plasmoid.immutable;
        });
        plasmoid.action("configure").enabled = Qt.binding(function() {
            return !plasmoid.immutable;
        });
    }

    onDragEnter: {
        if (plasmoid.immutable) {
            event.ignore();
            return;
        }
        //during drag operations we disable panel auto resize
        root.fixedWidth = root.Layout.preferredWidth
        root.fixedHeight = root.Layout.preferredHeight
        appletsModel.insert(LayoutManager.indexAtCoordinates(event.x, event.y), {applet: dndSpacer})
    }

    onDragMove: {
        LayoutManager.move(dndSpacer.parent, LayoutManager.indexAtCoordinates(event.x, event.y));
    }

    onDragLeave: {
        /*
         * When reordering task items, dragLeave signal will be emitted directly
         * without dragEnter, and in this case parent.index is undefined, so also
         * check if dndSpacer is in appletsModel.
         */
        if (typeof(dndSpacer.parent.index) === "number") {
            appletsModel.remove(dndSpacer.parent.index);
            root.fixedWidth = root.fixedHeight = 0;
        }
    }

    onDrop: {
        appletsModel.remove(dndSpacer.parent.index);
        plasmoid.processMimeData(event.mimeData, event.x, event.y);
        event.accept(event.proposedAction);
        root.fixedWidth = root.fixedHeight = 0;
    }

    Containment.onAppletAdded: {
        LayoutManager.addApplet(applet, x, y);
        checkLastSpacer();
        // When a new preset panel is added, avoid calling save() multiple times
        Qt.callLater(LayoutManager.save);
    }

    Containment.onAppletRemoved: {
        appletsModel.remove(applet.parent.index);
        checkLastSpacer();
        LayoutManager.save();
    }

    Plasmoid.onUserConfiguringChanged: {
        if (!Plasmoid.userConfiguring) {
            if (root.configOverlay) {
                root.configOverlay.destroy();
                root.configOverlay = null;
            }
            return;
        }

        if (Plasmoid.immutable) {
            return;
        }

        Plasmoid.applets.forEach(applet => applet.expanded = false);
        const component = Qt.createComponent("ConfigOverlay.qml");
        root.configOverlay = component.createObject(root, {
            "anchors.fill": root,
        });
        component.destroy();
    }

//END connections

//BEGIN components
    Component {
        id: appletContainerComponent
        // This loader conditionally manages the BusyIndicator, it's not
        // loading the applet. The applet becomes a regular child item.
        Loader {
            id: container
            required property Item applet
            required property int index
            property Item dragging
            property bool isAppletContainer: true
            property bool isMarginSeparator: ((applet.constraintHints & PlasmaCore.Types.MarginAreasSeparator) == PlasmaCore.Types.MarginAreasSeparator)
            property int appletIndex: index // To make sure it's always readable even inside other models
            property bool inThickArea: false
            visible: applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && plasmoid.userConfiguring);

            //when the applet moves caused by its resize, don't animate.
            //this is completely heuristic, but looks way less "jumpy"
            property bool movingForResize: false

            Layout.fillWidth: applet && applet.Layout.fillWidth
            Layout.onFillWidthChanged: {
                if (plasmoid.formFactor !== PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }
            Layout.fillHeight: applet && applet.Layout.fillHeight
            Layout.onFillHeightChanged: {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }

            function getMargins(side, returnAllMargins = false, overrideFillArea = null, overrideThickArea = null) {
                //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon + smallSpace) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.
                let fillArea = overrideFillArea === null ? applet && (applet.constraintHints & PlasmaCore.Types.CanFillArea) : overrideFillArea
                let inThickArea = overrideThickArea === null ? container.inThickArea : overrideThickArea
                var layout = {
                    top: isHorizontal, bottom: isHorizontal,
                    right: !isHorizontal, left: !isHorizontal
                };
                return ((layout[side] || returnAllMargins) && !fillArea) ? Math.round(Math.min(spacingAtMinSize, (inThickArea ? thickPanelSvg.fixedMargins[side] : panelSvg.fixedMargins[side]))) : 0;
            }

            Layout.topMargin: getMargins('top')
            Layout.bottomMargin: getMargins('bottom')
            Layout.leftMargin: getMargins('left')
            Layout.rightMargin: getMargins('right')

// BEGIN BUG 454095: do not combine these expressions to a function or the bindings won't work
            Layout.minimumWidth: (root.isHorizontal ? (applet && applet.Layout.minimumWidth > 0 ? applet.Layout.minimumWidth : root.height) : root.width) - Layout.leftMargin - Layout.rightMargin
            Layout.minimumHeight: (!root.isHorizontal ? (applet && applet.Layout.minimumHeight > 0 ? applet.Layout.minimumHeight : root.width) : root.height) - Layout.bottomMargin - Layout.topMargin

            Layout.preferredWidth: (root.isHorizontal ? (applet && applet.Layout.preferredWidth > 0 ? applet.Layout.preferredWidth : root.height) : root.width) - Layout.leftMargin - Layout.rightMargin
            Layout.preferredHeight: (!root.isHorizontal ? (applet && applet.Layout.preferredHeight > 0 ? applet.Layout.preferredHeight : root.width) : root.height) - Layout.bottomMargin - Layout.topMargin

            Layout.maximumWidth: (root.isHorizontal ? (applet && applet.Layout.maximumWidth > 0 ? applet.Layout.maximumWidth : (Layout.fillWidth ? root.width : root.height)) : root.height) - Layout.leftMargin - Layout.rightMargin
            Layout.maximumHeight: (!root.isHorizontal ? (applet && applet.Layout.maximumHeight > 0 ? applet.Layout.maximumHeight : (Layout.fillHeight ? root.height : root.width)) : root.width) - Layout.bottomMargin - Layout.topMargin
// END BUG 454095

            Item {
                id: marginHighlightElements
                anchors.fill: parent
                // index -1 is for floating applets, which do not need a margin highlight
                opacity: plasmoid.editMode && marginAreasEnabled && !root.dragAndDropping && index != -1 ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                component SideMargin: PlasmaCore.SvgItem {
                    property string side; property bool fill: true
                    property int inset; property int padding
                    property var west: ({'left': 'top', 'top': 'left', 'right': 'top', 'bottom': 'left'})
                    property var mirror: ({'left': 'right', 'top': 'bottom', 'right': 'left', 'bottom': 'top'})
                    property var onComponentCompleted: {
                        let left = west[side]
                        let right = mirror[left]
                        let up = mirror[side]
                        anchors[up] = undefined
                        this[isHorizontal ? 'height' : 'width'] = padding
                        anchors[left+'Margin'] = - currentLayout.rowSpacing/2 - (appletIndex == 0 ? panelSvg.margins[left] + currentLayout.x : 0)
                        anchors[right+'Margin'] = - currentLayout.rowSpacing/2 - (appletIndex == appletsModel.count-1 ? panelSvg.margins[right] + currentLayout.toolBoxSize : 0)
                        anchors[side+'Margin'] = - inset
                    }
                    elementId: fill ? 'fill' : (isHorizontal ? side + (inThickArea ? 'left' : 'right') : (inThickArea ? 'top' : 'bottom') + side)
                    svg: marginHighlightSvg
                    anchors {top: parent.top; left: parent.left; right: parent.right; bottom: parent.bottom}
                }
                Repeater {
                    model: ['top', 'bottom', 'right', 'left']
                    SideMargin{
                        side: modelData;
                        inset: getMargins(side);
                        visible: (modelData == 'top' || modelData == 'bottom') == isHorizontal
                        padding: getMargins(side, false, false, isMarginSeparator ? false : inThickArea)
                    }
                }
                Repeater {
                    model: ['top', 'bottom', 'right', 'left']
                    SideMargin{
                        side: modelData;
                        inset: -getMargins(side, false, false, false);
                        padding: getMargins(side, false, false, true) + inset;
                        visible: isMarginSeparator && (modelData == 'top' || modelData == 'bottom') == isHorizontal;
                        fill: false
                    }
                }
            }

            onAppletChanged: {
                applet.parent = container
                applet.anchors.fill = container
            }

            active: applet && applet.busy
            sourceComponent: PlasmaComponents.BusyIndicator {
                z: 999
            }

            property int oldX: 0
            property int oldY: 0
            onXChanged: if (oldX) animateFrom(oldX, y)
            onYChanged: if (oldY) animateFrom(x, oldY)
            transform: Translate{id: translation}
            function animateFrom(xa, ya) {
                if (isHorizontal) translation.x = xa - x
                else translation.y = ya - y
                oldX = oldY = 0
                translAnim.running = true
            }
            NumberAnimation {
                id: translAnim
                duration: PlasmaCore.Units.shortDuration
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
        leftMargin: isHorizontal ? Math.min(spacingAtMinSize, panelSvg.fixedMargins.left) : 0
        rightMargin: isHorizontal ? Math.min(spacingAtMinSize, panelSvg.fixedMargins.right) : 0
        topMargin: isHorizontal ? 0 : Math.min(spacingAtMinSize, panelSvg.fixedMargins.top)
        bottomMargin: isHorizontal ? 0 : Math.min(spacingAtMinSize, panelSvg.fixedMargins.bottom)
    }

    Item {
        id: dndSpacer
        property bool busy: false
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        width: isHorizontal ? PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 5 : currentLayout.width
        height: isHorizontal ? currentLayout.height : PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 5
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

        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.smallSpacing

        x: Qt.application.layoutDirection === Qt.RightToLeft && isHorizontal ? toolBoxSize : 0;
        readonly property int toolBoxSize:  !toolBox || !plasmoid.editMode || Qt.application.layoutDirection === Qt.RightToLeft ? 0 : (isHorizontal ? toolBox.width : toolBox.height)

// BEGIN BUG 454095: use lastSpacer to left align applets, as implicitWidth is updated too late
        width: root.width - (isHorizontal ? toolBoxSize : 0)
        height: root.height - (!isHorizontal ? toolBoxSize : 0)

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

        rows: isHorizontal ? 1 : currentLayout.children.length
        columns: isHorizontal ? currentLayout.children.length : 1
        flow: isHorizontal ? GridLayout.LeftToRight : GridLayout.TopToBottom
        layoutDirection: Qt.application.layoutDirection
    }
//END UI elements
}
