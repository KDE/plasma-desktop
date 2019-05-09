/*
 * Copyright 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQml.Models 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.2 as KCM

import org.kde.notificationmanager 1.0 as NotificationManager

import org.kde.private.kcms.notifications 1.0 as Private

ColumnLayout {
    id: configColumn

    property var rootIndex

    readonly property string appDisplayName: kcm.sourcesModel.data(rootIndex, Qt.DisplayRole) || ""
    readonly property string appIconName: kcm.sourcesModel.data(rootIndex, Qt.DecorationRole) || ""
    readonly property string desktopEntry: kcm.sourcesModel.data(rootIndex, Private.SourcesModel.DesktopEntryRole) || ""
    readonly property string notifyRcName: kcm.sourcesModel.data(rootIndex, Private.SourcesModel.NotifyRcNameRole) || ""

    property int behavior: {
        if (configColumn.desktopEntry) {
            return kcm.settings.applicationBehavior(configColumn.desktopEntry);
        } else if (configColumn.notifyRcName) {
            return kcm.settings.serviceBehavior(configColumn.notifyRcName);
        }
        return -1;
    }

    function setBehavior(flag, enable) {
        var newBehavior = behavior;
        if (enable) {
            newBehavior |= flag;
        } else {
            newBehavior &= ~flag;
        }

        if (configColumn.desktopEntry) {
            return kcm.settings.setApplicationBehavior(configColumn.desktopEntry, newBehavior);
        } else if (configColumn.notifyRcName) {
            return kcm.settings.setServiceBehavior(configColumn.notifyRcName, newBehavior);
        }
    }

    function configureEvents(eventId) {
        kcm.configureEvents(configColumn.notifyRcName, eventId, this)
    }

    spacing: Kirigami.Units.smallSpacing

    Kirigami.FormLayout {
        id: form

        RowLayout {
            Kirigami.FormData.isSection: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                source: configColumn.appIconName
            }

            Kirigami.Heading {
                level: 2
                text: configColumn.appDisplayName
                elide: Text.ElideRight
                textFormat: Text.PlainText
            }
        }

        QtControls.CheckBox {
            id: showPopupsCheck
            text: i18n("Show popups")
            checked: configColumn.behavior & NotificationManager.Settings.ShowPopups
            onClicked: configColumn.setBehavior(NotificationManager.Settings.ShowPopups, checked)
        }

        RowLayout { // just for indentation
            QtControls.CheckBox {
                Layout.leftMargin: mirrored ? 0 : indicator.width
                Layout.rightMargin: mirrored ? indicator.width : 0
                text: i18n("Show in do not disturb mode")
                enabled: showPopupsCheck.checked
                checked: configColumn.behavior & NotificationManager.Settings.ShowPopupsInDoNotDisturbMode
                onClicked: configColumn.setBehavior(NotificationManager.Settings.ShowPopupsInDoNotDisturbMode, checked)
            }
        }

        QtControls.CheckBox {
            text: i18n("Show in history")
            checked: configColumn.behavior & NotificationManager.Settings.ShowInHistory
            onClicked: configColumn.setBehavior(NotificationManager.Settings.ShowInHistory, checked)
        }

        QtControls.CheckBox {
            text: i18n("Show notification badges")
            enabled: !!configColumn.desktopEntry
            checked: configColumn.behavior & NotificationManager.Settings.ShowBadges
            onClicked: configColumn.setBehavior(NotificationManager.Settings.ShowBadges, checked)
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.Button {
            id: configureEventsButton
            text: i18n("Configure Events...")
            icon.name: "preferences-desktop-notification"
            visible: !!configColumn.notifyRcName
            onClicked: configColumn.configureEvents()
        }
    }

    QtControls.Label {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: form.implicitWidth
        text: i18n("This application does not support configuring notifications on a per-event basis.");
        wrapMode: Text.WordWrap
        visible: !configColumn.notifyRcName
    }

    // compact layout
    Item {
        Layout.fillHeight: true
    }
}
