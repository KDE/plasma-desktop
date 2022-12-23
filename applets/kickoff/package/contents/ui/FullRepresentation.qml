/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15
import QtQml 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.kicker 0.1 as Kicker

EmptyPage {
    id: root

    // plasmoid.rootItem is Kickoff.qml
    leftPadding: -plasmoid.rootItem.backgroundMetrics.leftPadding
    rightPadding: -plasmoid.rootItem.backgroundMetrics.rightPadding
    topPadding: 0
    bottomPadding: -plasmoid.rootItem.backgroundMetrics.bottomPadding
    readonly property var appletInterface: plasmoid.self

    Layout.minimumWidth: implicitWidth
    Layout.maximumWidth: PlasmaCore.Units.gridUnit * 80
    Layout.minimumHeight: implicitHeight
    Layout.maximumHeight: PlasmaCore.Units.gridUnit * 40

    property alias normalPage: normalPage

    /* NOTE: Important things to know about keyboard input handling:
     *
     * - Key events are passed up to parent items until the end is reached.
     * Be mindful of this when using `Keys.forwardTo`.
     *
     * - Keys defaults to BeforeItem while KeyNavigation defaults to AfterItem.
     *
     * - When Keys and KeyNavigation are using the same priority, it seems like
     * the one declared first in the QML file gets priority over the other.
     *
     * - Except for Keys.onPressed, all Keys.on*Pressed signals automatically
     * set `event.accepted = true`.
     *
     * - If you do `item.forceActiveFocus()` and `item` is a focus scope, the
     * children of `item` won't necessarily get focus. It seems like
     * `forceActiveFocus()` is better for forcing a specific thing to be focused
     * while KeyNavigation is better at passing focus down to children of the
     * thing you want to focus when dealing with focus scopes.
     *
     * - KeyNavigation uses BacktabFocusReason (TabFocusReason if mirrored) for left,
     * TabFocusReason (BacktabFocusReason if mirrored) for right,
     * BacktabFocusReason for up and TabFocusReason for down.
     */

    header: Header {
        id: header
        preferredNameAndIconWidth: normalPage.preferredSideBarWidth
        Binding {
            target: plasmoid.rootItem
            property: "header"
            value: header
            restoreMode: Binding.RestoreBinding
        }
    }

    contentItem: VerticalStackView {
        id: contentItemStackView
        focus: true
        movementTransitionsEnabled: true
        // Not using a component to prevent it from being destroyed
        initialItem: NormalPage {
            id: normalPage
            objectName: "normalPage"
        }

        Component {
            id: searchViewComponent
            KickoffListView {
                id: searchView
                objectName: "searchView"
                mainContentView: true
                implicitWidth: normalPage.implicitWidth
                implicitHeight: normalPage.implicitHeight
                // Forces the function be re-run every time runnerModel.count changes.
                // This is absolutely necessary to make the search view work reliably.
                model: plasmoid.rootItem.runnerModel.count ? plasmoid.rootItem.runnerModel.modelForRow(0) : null
                delegate: KickoffListDelegate {
                    width: view.availableWidth
                    isSearchResult: true
                }
                activeFocusOnTab: true
                // always focus the first item in the header focus chain
                KeyNavigation.tab: root.header.nextItemInFocusChain()
                T.StackView.onActivated: {
                    plasmoid.rootItem.sideBar = null
                    plasmoid.rootItem.contentArea = searchView
                }

                Loader {
                    anchors.centerIn: searchView.view
                    width: searchView.view.width - (PlasmaCore.Units.largeSpacing * 4)

                    active: searchView.view.count === 0
                    visible: active
                    asynchronous: true

                    sourceComponent: PlasmaExtras.PlaceholderMessage {
                        id: emptyHint

                        iconName: "edit-none"
                        opacity: 0
                        text: i18nc("@info:status", "No matches")

                        Connections {
                            target: Plasmoid.rootItem.runnerModel
                            function onQueryFinished() {
                                showAnimation.restart()
                            }
                        }

                        NumberAnimation {
                            id: showAnimation
                            duration: PlasmaCore.Units.longDuration
                            easing.type: Easing.OutCubic
                            property: "opacity"
                            target: emptyHint
                            to: 1
                        }
                    }
                }
            }
        }

        Keys.priority: Keys.AfterItem
        // This is here rather than root because events are implicitly forwarded
        // to parent items. Don't want to send multiple events to searchField.
        Keys.forwardTo: plasmoid.rootItem.searchField

        Connections {
            target: root.header
            function onSearchTextChanged() {
                if (root.header.searchText.length === 0 && contentItemStackView.currentItem.objectName !== "normalPage") {
                    contentItemStackView.reverseTransitions = true
                    contentItemStackView.replace(normalPage)
                } else if (root.header.searchText.length > 0 && contentItemStackView.currentItem.objectName !== "searchView") {
                    contentItemStackView.reverseTransitions = false
                    contentItemStackView.replace(searchViewComponent)
                }
            }
        }
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
