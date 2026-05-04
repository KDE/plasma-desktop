/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.private.kicker as Kicker
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

ItemAbstractDelegate {
    id: item

    required property bool showIcons

    property bool showDescriptionInTooltip: false
    property alias containsMouse: toolTipArea.containsMouse

    readonly property bool iconAndLabelsShouldlookSelected: pressed && !hasChildren

    width: ListView.view.width
    height: implicitHeight

    // if it's not disabled and is either a leaf node or a node with children
    enabled: !isSeparator && !disabled && (!isParent || (isParent && hasChildren))
    hoverEnabled: false // containsMouse is more robust if the tooltip covers delegates
    favoritesModel: ListView.view.model.favoritesModel
    baseModel: ListView.view.model
    dragActive: dragHandler.active

    Accessible.role: isSeparator ? Accessible.Separator : Accessible.ListItem
    Accessible.description: isParent
        ? i18nc("@action:inmenu accessible description for opening submenu", "Open category")
        : i18nc("@action:inmenu accessible description for opening app or file", "Launch")

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem === item) {
            ListView.view.currentIndex = -1;
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
            source: item.model.decoration
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

        Loader {
            visible: active
            active: item.isNewlyInstalled ?? false

            sourceComponent: Kirigami.Badge {
                text: item.hasChildren ? "" : Accessible.name
                type: Kirigami.Badge.Type.Positive
                Accessible.name: i18nc("Newly-installed app, badge, keep short", "New!") // qmllint disable unqualified
                Accessible.description: item.hasChildren ? i18nc("@info:whatsthis Accessible description for badge", "There is a newly-installed application in this category") // qmllint disable unqualified
                : i18nc("@info:whatsthis Accessible description for badge", "Newly-installed application") // qmllint disable unqualified
            }
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

    PlasmaCore.ToolTipArea {
        id: toolTipArea
        // needs to be ToolTipArea, as ItemListDialog will clip (attached) ToolTip if the submenu
        // has very few entries (and makes it feel glitchy then)
        anchors.fill: parent
        active: label.truncated || item.showDescriptionInTooltip
        mainText: {
            // if it's name (description) or description (name), we split them on separate lines
            // but only if the compactName is available (e.g. for search results it's not)
            let name = (Plasmoid.configuration.appNameFormat > 1 && item.compactName.length > 1) ? item.compactName : item.text
            return label.truncated ? name ?? "" : ""
        }
        subText: item.showDescriptionInTooltip || Plasmoid.configuration.appNameFormat > 1 ? item.description ?? "" : ""
    }
}
