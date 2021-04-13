/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

EmptyPage {
    id: root
    property real preferredSideBarWidth: Math.max(footer.implicitTabBarWidth, stackView.currentItem ? stackView.currentItem.implicitSideBarWidth : 0)

    contentItem: HorizontalStackView {
        id: stackView
        focus: true
        reverseTransitions: footer.tabBarCurrentIndex === 1
        initialItem: applicationsPage
        Component {
            id: applicationsPage
            ApplicationsPage {
                preferredSideBarWidth: root.preferredSideBarWidth + KickoffSingleton.backgroundMetrics.margins.left
            }
        }
        Component {
            id: placesPage
            PlacesPage {
                preferredSideBarWidth: root.preferredSideBarWidth + KickoffSingleton.backgroundMetrics.margins.left
            }
        }
        Connections {
            target: footer
            function onTabBarCurrentIndexChanged() {
                if (footer.tabBarCurrentIndex === 0) {
                    stackView.replace(applicationsPage)
                } else if (footer.tabBarCurrentIndex === 1) {
                    stackView.replace(placesPage)
                }
            }
        }
    }

    footer: Footer {
        id: footer
        preferredTabBarWidth: root.preferredSideBarWidth
        Binding {
            target: KickoffSingleton
            property: "footer"
            value: footer
            restoreMode: Binding.RestoreBinding
        }
    }
}
