/*
 *  SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.0

import org.kde.activities 0.1 as Activities
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.19 as Kirigami


KCM.ScrollViewKCM {

    implicitWidth: Kirigami.Units.gridUnit * 18
    implicitHeight: Kirigami.Units.gridUnit * 22

    KCM.ConfigModule.buttons: KCM.ConfigModule.Help

    view: ListView {
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
                    onTriggered: kcm.configureActivity(model.id);
                },
                Kirigami.Action {
                    visible: kcm.isNewActivityAuthorized
                    enabled:  activitiesList.count > 1
                    icon.name: "edit-delete"
                    tooltip: i18nc("@info:tooltip", "Delete %1 activity", model.name)
                    onTriggered: {
                        removePrompt.activityId = model.id;
                        removePrompt.activityName = model.name;
                        removePrompt.open();
                    }
                }
            ]
        }
    }

    actions: Kirigami.Action {
        visible: kcm.isNewActivityAuthorized
        text: i18n("Create New…")
        icon.name: "list-add"
        onTriggered: kcm.newActivity();
    }

    Kirigami.PromptDialog {
        id: removePrompt

        property string activityId: ""
        property string activityName: ""

        title: i18nc("@title:window", "Delete Activity")
        subtitle: i18nc("%1 is an activity name",
                         "Do you want to delete activity '%1'?", activityName)

        standardButtons: Kirigami.Dialog.Cancel
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete Activity")
                icon.name: "edit-delete"
                onTriggered: {
                    kcm.deleteActivity(removePrompt.activityId)
                    removePrompt.close()
                }
            }
        ]
    }
}
