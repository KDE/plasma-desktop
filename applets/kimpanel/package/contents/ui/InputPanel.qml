/*
    SPDX-FileCopyrightText: 2014-2017 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.private.kimpanel 0.1 as Kimpanel


PlasmaCore.Dialog {
    id: inputpanel
    type: PlasmaCore.Dialog.PopupMenu
    flags: {
        var flag = Qt.WindowTransparentForInput | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus;
        if (Qt.platform.pluginName.includes("wayland")) {
            flag |= Qt.ToolTip;
        } else {
            flag |= Qt.Popup;
        }
        return flag;
    }
    location: PlasmaCore.Types.Floating
    visible: helper.auxVisible || helper.preeditVisible || helper.lookupTableVisible
    readonly property bool verticalLayout: (helper.lookupTableLayout === 1) || (helper.lookupTableLayout === 0 && plasmoid.configuration.vertical_lookup_table);
    property int highlightCandidate: helper.lookupTableCursor
    property int hoveredCandidate: -1
    property font preferredFont: plasmoid.configuration.use_default_font ? PlasmaCore.Theme.defaultFont : plasmoid.configuration.font
    readonly property alias textOffset: fontMetrics.ascent
    readonly property alias labelHeight: fontMetrics.height
    property rect position: helper.spotRect

    onPositionChanged : updatePosition();
    onWidthChanged : updatePosition();
    onHeightChanged : updatePosition();

    mainItem: Item {
        Layout.minimumWidth: childrenRect.width
        Layout.minimumHeight: childrenRect.height
        Layout.maximumWidth: childrenRect.width
        Layout.maximumHeight: childrenRect.height
        FontMetrics {
            id: fontMetrics
            font: preferredFont
        }
        Column {
            spacing: PlasmaCore.Units.smallSpacing
            Row {
                id: textLabel
                width: auxLabel.width + preedit.width
                height: inputpanel.labelHeight
                visible: helper.auxVisible || helper.preeditVisible
                baselineOffset: inputpanel.textOffset
                PlasmaComponents3.Label {
                    id: auxLabel
                    anchors.baseline: parent.baseline
                    font: preferredFont
                    text: helper.auxText
                    visible: helper.auxVisible
                }
                Item {
                    id: preedit
                    width: preeditLabel1.width + preeditLabel2.width + 2
                    height: parent.height
                    clip: true
                    visible: helper.preeditVisible
                    baselineOffset: inputpanel.textOffset
                    PlasmaComponents3.Label {
                        id: preeditLabel1
                        anchors.baseline: parent.baseline
                        anchors.left: parent.left
                        font: preferredFont
                    }
                    Rectangle {
                        color: PlasmaCore.Theme.textColor
                        height: parent.height
                        width: 1
                        opacity: 0.8
                        z: 1
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: preeditLabel1.right
                    }
                    PlasmaComponents3.Label {
                        id: preeditLabel2
                        anchors.baseline: parent.baseline
                        anchors.left: preeditLabel1.right
                        font: preferredFont
                    }
                }
            }

            GridLayout {
                flow: inputpanel.verticalLayout ? GridLayout.TopToBottom : GridLayout.LeftToRight
                columns: inputpanel.verticalLayout ? 1 : tableList.count + 1
                rows: inputpanel.verticalLayout ? tableList.count + 1 : 1

                Repeater {
                    model: ListModel {
                        id: tableList
                        dynamicRoles: true
                    }
                    delegate: Item {
                        id: candidateDelegate
                        width: candidate.width + highlight.marginHints.left + highlight.marginHints.right
                        height: inputpanel.labelHeight + highlight.marginHints.top + highlight.marginHints.bottom
                        Layout.minimumWidth: width
                        Layout.minimumHeight: height
                        Layout.maximumWidth: width
                        Layout.maximumHeight: height

                        readonly property bool selected: inputpanel.highlightCandidate === model.index || candidateMouseArea.pressed

                        Row {
                            id: candidate
                            width: childrenRect.width
                            height: childrenRect.height
                            x: highlight.marginHints.left
                            y: highlight.marginHints.top
                            baselineOffset: inputpanel.textOffset
                            spacing: PlasmaCore.Units.smallSpacing
                            PlasmaComponents3.Label {
                                id: tableLabel
                                text: model.label
                                font: preferredFont
                                opacity: 0.8
                                color: PlasmaCore.Theme.textColor
                                anchors.baseline: parent.baseline
                            }
                            PlasmaComponents3.Label {
                                id: textLabel
                                text: model.text
                                font: preferredFont
                                color: PlasmaCore.Theme.textColor
                                anchors.baseline: parent.baseline
                            }
                        }
                        MouseArea {
                            id: candidateMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onReleased: helper.selectCandidate(model.index)
                            onContainsMouseChanged: {
                                inputpanel.hoveredCandidate = containsMouse ? model.index : -1;
                            }
                        }
                        CandidateHighlight {
                            id: highlight
                            z: -1
                            visible: inputpanel.highlightCandidate === model.index || inputpanel.hoveredCandidate === model.index
                            selected: candidateDelegate.selected
                            anchors {
                                fill: parent
                            }
                        }
                    }
                }
                Row {
                    id: button
                    width: inputpanel.labelHeight * 2
                    height: inputpanel.labelHeight
                    Layout.minimumWidth: width
                    Layout.minimumHeight: height
                    Layout.maximumWidth: width
                    Layout.maximumHeight: height
                    spacing: PlasmaCore.Units.smallSpacing
                    PlasmaCore.IconItem {
                        id: prevButton
                        source: inputpanel.verticalLayout ? "arrow-up" : "arrow-left"
                        width: inputpanel.labelHeight
                        height: width
                        scale: prevButtonMouseArea.pressed ? 0.9 : 1
                        active: prevButtonMouseArea.containsMouse
                        MouseArea {
                            id: prevButtonMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onReleased: helper.lookupTablePageUp()
                        }
                    }
                    PlasmaCore.IconItem {
                        id: nextButton
                        source: inputpanel.verticalLayout ? "arrow-down" : "arrow-right"
                        width: inputpanel.labelHeight
                        height: width
                        scale: nextButtonMouseArea.pressed ? 0.9 : 1
                        active: nextButtonMouseArea.containsMouse
                        MouseArea {
                            id: nextButtonMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onReleased: helper.lookupTablePageDown()
                        }
                    }
                }
            }
        }

        Kimpanel.Screen {
            id: screen
        }

        // Kimpanel's update may come in with in several DBus message. Use
        // timer to delegate the update so we get less flicker.
        Timer {
            id: timer
            interval: 1
            onTriggered: updateLookupTable()
        }

        Connections {
            target: helper

            function onPreeditTextChanged() {
                var charArray = [...helper.preeditText];
                preeditLabel1.text = charArray.slice(0, helper.caretPos).join('');
                preeditLabel2.text = charArray.slice(helper.caretPos).join('');
            }

            function onLookupTableChanged() {
                timer.restart();
            }
        }
    }

    function updateLookupTable() {
        inputpanel.hoveredCandidate = -1;
        button.visible = helper.lookupTableVisible && (helper.hasPrev || helper.hasNext);

        var labels = helper.labels;
        var texts = helper.texts;

        var length = Math.min(labels.length, texts.length);

        if (helper.lookupTableVisible) {
            if (length< tableList.count) {
                tableList.remove(length, tableList.count - length);
            }
            for (var i = 0; i < length; i ++) {
                if (i >= tableList.count) {
                    tableList.append({'label' : labels[i], 'text': texts[i], 'index': i});
                } else {
                    tableList.set(i, {'label' : labels[i], 'text': texts[i], 'index': i});
                }
            }
        } else {
            tableList.clear();
        }
    }

    function updatePosition() {
        var rect = screen.geometryForPoint(position.x, position.y);
        var devicePerPixelRatio = screen.devicePixelRatioForPoint(position.x, position.y);
        var x, y;
        var width = inputpanel.width * devicePerPixelRatio;
        var height = inputpanel.height * devicePerPixelRatio;
        if (position.x < rect.x) {
            x = rect.x;
        } else {
            x = position.x;
        }
        if (position.y < rect.y) {
            y = rect.y;
        } else {
            y = position.y + position.height;
        }

        if (x + width > rect.x + rect.width) {
            x = rect.x + rect.width - width;
        }

        if (y + height > rect.y + rect.height) {
            if (y > rect.y + rect.height) {
                y = rect.y + rect.height - height - 40;
            } else {
                y = y - height - (position.height === 0 ? 40 : position.height);
            }
        }

        var newRect = screen.geometryForPoint(x, y);
        devicePerPixelRatio = screen.devicePixelRatioForPoint(x, y);

        inputpanel.x = newRect.x + (x - newRect.x) / devicePerPixelRatio;
        inputpanel.y = newRect.y + (y - newRect.y) / devicePerPixelRatio;
    }
}
