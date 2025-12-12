/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3

import "code/tools.js" as Tools

PlasmaComponents3.ItemDelegate {
    id: item

    width: ListView.view.width
    height: isSeparator && !showSeparators ? 0 : implicitHeight

    // if it's not disabled and is either a leaf node or a node with children
    enabled: !isSeparator && !disabled && (!isParent || (isParent && hasChildren))

    required property int index
    required property bool isSeparator
    required property bool hasChildren
    required property bool isParent
    required property bool disabled
    required property bool hasActionList
    required property string favoriteId
    required property var /*QVariantList*/ actionList
    required property url url
    required property string description
    required property string decoration
    required property var model // for display, which would shadow ItemDelegate
    required property bool showIcons

    readonly property bool iconAndLabelsShouldlookSelected: pressed && !hasChildren
    readonly property ActionMenu menu: actionMenu

    property bool showSeparators: true
    property bool dialogDefaultRight: Application.layoutDirection !== Qt.RightToLeft

    signal interactionConcluded

    Accessible.role: isSeparator ? Accessible.Separator : Accessible.ListItem
    Accessible.description: isParent
        ? i18nc("@action:inmenu accessible description for opening submenu", "Open category")
        : i18nc("@action:inmenu accessible description for opening app or file", "Launch")
    text: model.display
    icon.name: decoration

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem === item) {
            ListView.view.currentIndex = -1;
        }
    }

    onClicked: {
        if (!item.hasChildren) {
            item.ListView.view.model.trigger(index, "", null);
            item.interactionConcluded()
        }
    }

    function openActionMenu(visualParent, x, y) {
        const actionList = item.hasActionList ? item.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, item.ListView.view.model.favoritesModel, item.favoriteId);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: (actionId, actionArgument) => {
            if (Tools.triggerAction(item.ListView.view.model, item.index, actionId, actionArgument) === true) {
                item.interactionConcluded()
            }
        }
    }

    DragHandler {
        target: null
        onActiveChanged: {
            if (active && item.url) {
                // we need dragHelper and can't use attached Drag; submenus are destroyed too soon and Plasma crashes
                dragHelper.startDrag(kicker, item.url, item.decoration)
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.RightButton

        onPressed: mouse => {
            if (item.hasActionList || item.favoriteId !== null) {
                item.openActionMenu(mouseArea, mouse.x, mouse.y);
            }
        }
    }

    contentItem: RowLayout {
        id: row

        spacing: Kirigami.Units.smallSpacing * 2

        LayoutMirroring.enabled: (Application.layoutDirection === Qt.RightToLeft)

        Kirigami.Icon {
            id: icon

            Layout.alignment: Qt.AlignVCenter
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: implicitWidth

            visible: item.showIcons & !item.isSeparator

            animated: false
            selected: item.iconAndLabelsShouldlookSelected
            source: item.icon.name
        }

        PlasmaComponents3.Label {
            id: label

            enabled: !item.isParent || (item.isParent && item.hasChildren)
            LayoutMirroring.enabled: (Application.layoutDirection === Qt.RightToLeft)
            visible: !item.isSeparator

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            verticalAlignment: Text.AlignVCenter

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            color: item.iconAndLabelsShouldlookSelected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            text: item.model.display ?? ""
        }

        Kirigami.Icon {
            id: arrow

            Layout.alignment: Qt.AlignVCenter

            implicitWidth: visible ? Kirigami.Units.iconSizes.small : 0
            implicitHeight: implicitWidth

            visible: item.hasChildren && !item.isSeparator
            opacity: (item.ListView.view.currentIndex === item.index) ? 1.0 : 0.4
            selected: item.iconAndLabelsShouldlookSelected
            source: item.dialogDefaultRight
                ? "go-next-symbolic"
                : "go-next-rtl-symbolic"
        }

        Loader {
            id: separatorLoader

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            // Separator positions don't make sense when sorting everything alphabetically
            active: item.isSeparator && item.showSeparators
            visible: active

            asynchronous: false
            sourceComponent: separatorComponent
        }
    }

    Component {
        id: separatorComponent

        KSvg.SvgItem {
            width: parent.width

            imagePath: "widgets/line"
            elementId: "horizontal-line"
        }
    }

    Keys.onMenuPressed: {
        if (item.hasActionList || item.favoriteId !== null) {
            item.openActionMenu(mouseArea)
        }
    }
    Keys.onReturnPressed: item.clicked()
    Keys.onEnterPressed: Keys.returnPressed()
}
