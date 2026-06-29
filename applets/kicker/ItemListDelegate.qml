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
    id: root

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
        if (!hasChildren && root.ListView.isCurrentItem) {
            root.ListView.view.currentIndex = -1;
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

            visible: root.showIcons & !root.isSeparator

            animated: false
            selected: root.iconAndLabelsShouldlookSelected
            source: root.model.decoration
        }

        PlasmaComponents3.Label {
            id: label

            enabled: !root.isParent || (root.isParent && root.hasChildren)
            LayoutMirroring.enabled: (Application.layoutDirection === Qt.RightToLeft)
            visible: !root.isSeparator

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            verticalAlignment: Text.AlignVCenter

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            color: root.iconAndLabelsShouldlookSelected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            text: root.model.display ?? ""
        }

        Loader {
            visible: active
            active: root.isNewlyInstalled ?? false

            sourceComponent: Kirigami.Badge {
                text: root.hasChildren ? "" : Accessible.name
                type: Kirigami.Badge.Type.Positive
                Accessible.name: i18nc("Newly-installed app, badge, keep short", "New!")
                Accessible.description: root.hasChildren ? i18nc("@info:whatsthis Accessible description for badge", "There is a newly-installed application in this category")
                    : i18nc("@info:whatsthis Accessible description for badge", "Newly-installed application")
            }
        }

        Kirigami.Icon {
            id: arrow

            Layout.alignment: Qt.AlignVCenter

            implicitWidth: visible ? Kirigami.Units.iconSizes.small : 0
            implicitHeight: implicitWidth

            visible: root.hasChildren && !root.isSeparator
            opacity: (root.ListView.view.currentIndex === root.index) ? 1.0 : 0.4
            selected: root.iconAndLabelsShouldlookSelected
            source: root.dialogDefaultRight
                ? "go-next-symbolic"
                : "go-next-rtl-symbolic"
        }

        Loader {
            id: separatorLoader

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            active: root.isSeparator
            visible: active

            asynchronous: false
            sourceComponent: separatorComponent
        }
    }

    DragHandler {
        id: dragHandler
        target: null
        enabled: (root.url && root.url.toString() !== "") || (root.favoriteId != "")
        onActiveChanged: {
            if (active) {
                // we need dragHelper and can't use attached Drag; submenus are destroyed too soon and Plasma crashes
                if (!root.favoriteId) {
                    dragHelper.startDrag(kicker, root.url, root.decoration)
                } else {
                    let type = root.favoritesModel instanceof Kicker.SimpleFavoritesModel
                        ? "text/xx-kicker-simplefavorite-id"
                        : "text/xx-kicker-kastatsfavorite-id"
                    dragHelper.startDrag(kicker, root.url, root.decoration,
                                         type, root.favoriteId)
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
        active: label.truncated || root.showDescriptionInTooltip
        mainText: {
            // if it's name (description) or description (name), we split them on separate lines
            // but only if the compactName is available (e.g. for search results it's not)
            let name = (Plasmoid.configuration.appNameFormat > 1 && root.compactName.length > 1) ? root.compactName : root.text
            return label.truncated ? name ?? "" : ""
        }
        subText: root.showDescriptionInTooltip || Plasmoid.configuration.appNameFormat > 1 ? root.description ?? "" : ""
    }
}
