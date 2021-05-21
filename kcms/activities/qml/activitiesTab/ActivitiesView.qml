/*   vim:set foldenable foldmethod=marker:

    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.activities 0.1 as Activities
import org.kde.activities.settings 0.1
import org.kde.kirigami 2.5 as Kirigami

ColumnLayout {
    id: root

    QQC2.ScrollView {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Component.onCompleted: background.visible = true;

        ListView {
            id: activitiesList

            clip: true

            model: Activities.ActivityModel {
                id: kactivities
            }

            delegate: Kirigami.SwipeListItem {
                hoverEnabled: true

                contentItem: RowLayout {
                    id: row

                    Kirigami.Icon {
                        id: icon
                        height: Kirigami.Units.iconSizes.medium
                        width: height
                        source: model.icon
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: model.name
                    }
                }

                actions: [
                    Kirigami.Action {
                        icon.name: "configure"
                        tooltip: i18nc("@info:tooltip", "Configure %1 activity", model.name)
                        onTriggered: ActivitySettings.configureActivity(model.id);
                    },
                    Kirigami.Action {
                        visible: ActivitySettings.newActivityAuthorized
                        enabled:  activitiesList.count > 1
                        icon.name: "edit-delete"
                        tooltip: i18nc("@info:tooltip", "Delete %1 activity", model.name)
                        onTriggered: ActivitySettings.deleteActivity(model.id);
                    }
                ]
            }
        }
    }

    QQC2.Button {
        id: buttonCreateActivity
        visible: ActivitySettings.newActivityAuthorized
        text: i18nd("kcm_activities5", "Create New…")
        icon.name: "list-add"
        onClicked: ActivitySettings.newActivity();
    }
}
