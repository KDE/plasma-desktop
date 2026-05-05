
import QtCore
import QtQml
import QtQml.Models as QQM
import QtQuick
import QtQuick.Controls as QQC
import QtQuick.Dialogs
import QtQuick.Layouts as QQL

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.private.kcms.shortcuts as Private


KCMUtils.ScrollViewKCM {
    title: i18nc("@title", "Touchpad Gestures")

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button Add new gesture/action assignment", "Add Gesture…")
            icon.name: "list-add-symbolic"
            displayHint: Kirigami.DisplayHint.KeepVisible
        }
    ]


    ListView {
        id: gestureList

        model: kcm.shortcutsModel

        anchors.fill: parent
        clip: true

        section.property: "componentName"
        section.delegate: Kirigami.ListSectionHeader {
            text: "...component..." // TODO model.componentName
            icon.name: "kwin" // TODO model.componentName

            width: gestureList.width
        }

        delegate: QQC.ItemDelegate {
            id: actionDelegate

            icon.name: "draw-polyline-symbolic" // TODO model.triggerPreviewIcon

            implicitWidth: gestureList.width

            contentItem: Kirigami.TitleSubtitleWithActions {
                title: "...action..." // TODO model.actionName 
                subtitle: "...gesture..." // TODO model.gestureText

                elide: Text.ElideRight
                displayHint: QQC.Button.IconOnly

                actions: [
                    Kirigami.Action {
                        id: renameAction
                        icon.name: "edit-entry-symbolic"
                        text: i18nc("@action:button", "Edit gesture")
                        tooltip: text
                        // onTriggered: // TODO
                    },
                    Kirigami.Action {
                        id: removeAction
                        icon.name: "edit-delete-remove-symbolic"
                        text: i18nc("@info:tooltip", "Remove gesture")
                        tooltip: text
                        // onTriggered: // TODO
                    }
                ]
            }
        }
    }
}
