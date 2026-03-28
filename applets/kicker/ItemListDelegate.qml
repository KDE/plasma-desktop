/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.private.kicker as Kicker


ItemAbstractDelegate {
    id: item

    required property bool showIcons

    readonly property bool iconAndLabelsShouldlookSelected: pressed && !hasChildren

    width: ListView.view.width
    height: implicitHeight

    // if it's not disabled and is either a leaf node or a node with children
    enabled: !isSeparator && !disabled && (!isParent || (isParent && hasChildren))
    favoritesModel: ListView.view.model.favoritesModel
    baseModel: ListView.view.model

    Accessible.role: isSeparator ? Accessible.Separator : Accessible.ListItem
    Accessible.description: isParent
        ? i18nc("@action:inmenu accessible description for opening submenu", "Open category")
        : i18nc("@action:inmenu accessible description for opening app or file", "Launch")

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem === item) {
            ListView.view.currentIndex = -1;
        }
    }

    onClicked: {
        if (!item.hasChildren && !dragHandler.active) {
            item.baseModel.trigger(index, "", null);
            item.interactionConcluded()
        }
    }

    Keys.onReturnPressed: item.clicked()
    Keys.onEnterPressed: item.clicked()

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

            active: item.isSeparator
            visible: active

            asynchronous: false
            sourceComponent: separatorComponent
        }
    }

    DragHandler {
        id: dragHandler
        target: null
        onActiveChanged: {
            if (active && item.url) {
                // we need dragHelper and can't use attached Drag; submenus are destroyed too soon and Plasma crashes
                if (!item.favoriteId) {
                    dragHelper.startDrag(kicker, item.url, item.decoration)
                } else {
                    let type = item.favoritesModel instanceof Kicker.SimpleFavoritesModel
                        ? "text/xx-kicker-simplefavorite-id"
                        : "text/xx-kicker-kastatsfavorite-id"
                    dragHelper.startDrag(kicker, item.url, item.decoration,
                                         type, item.favoriteId)
                }
            }
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
}
