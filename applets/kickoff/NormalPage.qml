/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T

EmptyPage {
    id: root
    property real preferredSideBarWidth: Math.max(footer.tabBar.implicitWidth, applicationsPage.implicitSideBarWidth)

    contentItem: HorizontalStackView {
        id: stackView
        focus: true
        reverseTransitions: footer.tabBar.currentIndex === 1
        initialItem: ApplicationsPage {
            id: applicationsPage
            preferredSideBarWidth: root.preferredSideBarWidth + kickoff.backgroundMetrics.leftPadding
        }
        Component {
            id: placesPage
            PlacesPage {
                preferredSideBarWidth: root.preferredSideBarWidth + kickoff.backgroundMetrics.leftPadding
                preferredSideBarHeight: applicationsPage.implicitSideBarHeight
            }
        }
        Connections {
            target: footer.tabBar
            function onCurrentIndexChanged() {
                if (footer.tabBar.currentIndex === 0) {
                    stackView.replace(applicationsPage)
                } else if (footer.tabBar.currentIndex === 1) {
                    stackView.replace(placesPage)
                }
            }
        }
    }

    footer: Footer {
        id: footer
        preferredTabBarWidth: root.preferredSideBarWidth
        Binding {
            target: kickoff
            property: "footer"
            value: footer
            restoreMode: Binding.RestoreBinding
        }
        // Eat down events to prevent them from reaching the contentArea or searchField
        Keys.onDownPressed: event => {}
    }
}
