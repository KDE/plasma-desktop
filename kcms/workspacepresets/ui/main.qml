/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.GridViewKCM {
    id: root

    // The screen this module is displayed on; the selection and Apply are scoped to it.
    readonly property string currentScreenName: Screen.name

    view.model: kcm.presets

    function selectId(id) {
        view.currentIndex = kcm.presets.indexOfId(id);
        kcm.setSelection(id, root.currentScreenName);
    }
    // Reselect whatever preset is applied on the current screen (initial load / Reset). If the
    // screen has no recorded preset (e.g. it was connected after the module opened), fall back to
    // the first entry (the "Previous Layout"/current layout) so the grid never ends up with nothing
    // selected.
    function syncToCurrent() {
        var id = kcm.currentPresetForScreen(root.currentScreenName);
        if (id === "" || kcm.presets.indexOfId(id) < 0) {
            id = kcm.presets.idAt(0);
        }
        selectId(id);
    }

    Component.onCompleted: {
        kcm.setCurrentLayoutScreen(currentScreenName);
        syncToCurrent();
    }
    onCurrentScreenNameChanged: {
        kcm.setCurrentLayoutScreen(currentScreenName);
        syncToCurrent();
    }

    actions: Kirigami.Action {
        text: i18n("Save Current Layout…")
        icon.name: "document-save-symbolic"
        onTriggered: {
            saveDialog.presetName = "";
            saveDialog.open();
        }
    }

    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: feedback
            Layout.fillWidth: true
            showCloseButton: true
            visible: false
        }

        QQC2.CheckBox {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            text: i18n("Also set matching keyboard shortcuts when applying a preset")
            checked: kcm.applyShortcuts
            onToggled: kcm.applyShortcuts = checked
        }
    }

    Connections {
        target: kcm
        function onReinitialise() {
            root.syncToCurrent();
        }
        function onSelectPreset(id) {
            root.selectId(id);
        }
        function onErrorOccurred(message) {
            feedback.text = message;
            feedback.type = Kirigami.MessageType.Error;
            feedback.visible = true;
        }
        function onLayoutSaved(id) {
            feedback.text = i18n("The current layout has been saved as a preset.");
            feedback.type = Kirigami.MessageType.Positive;
            feedback.visible = true;
            root.selectId(id);
        }
    }

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.name
        toolTip: model.description
        thumbnailAvailable: true

        onClicked: root.selectId(model.presetId)

        thumbnail: Rectangle {
            id: screen

            // model is the delegate's context property; capture the panels role for the Repeater.
            readonly property var panelList: model.panels

            anchors.fill: parent
            radius: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.backgroundColor
            border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.3)
            border.width: 1

            // Draw each panel of the layout as a bar on the mini "screen".
            Repeater {
                model: screen.panelList

                Rectangle {
                    required property var modelData

                    readonly property bool vertical: modelData.edge === "left" || modelData.edge === "right"
                    readonly property real thickFrac: modelData.thin ? 0.09 : 0.15
                    readonly property real lenFrac: modelData.fill ? 1.0 : 0.5
                    readonly property real inset: modelData.floating ? parent.height * 0.05 : 0

                    color: Kirigami.Theme.highlightColor
                    opacity: 0.7
                    radius: modelData.floating ? height / 4 : 0

                    width: vertical ? parent.width * 0.10 : parent.width * lenFrac - 2 * inset
                    height: vertical ? parent.height - 2 * inset : parent.height * thickFrac

                    x: {
                        if (modelData.edge === "right") {
                            return parent.width - width - inset;
                        }
                        if (vertical) {
                            return inset;
                        }
                        return (parent.width - width) / 2;
                    }
                    y: {
                        if (modelData.edge === "bottom") {
                            return parent.height - height - inset;
                        }
                        return inset;
                    }
                }
            }
        }

        actions: [
            Kirigami.Action {
                icon.name: "edit-rename-symbolic"
                text: i18n("Rename…")
                visible: !model.builtIn
                onTriggered: {
                    renameDialog.presetId = model.presetId;
                    renameDialog.presetName = model.name;
                    renameDialog.open();
                }
            },
            Kirigami.Action {
                icon.name: "edit-delete-symbolic"
                text: i18n("Delete")
                visible: !model.builtIn
                onTriggered: {
                    deleteDialog.presetId = model.presetId;
                    deleteDialog.presetName = model.name;
                    deleteDialog.open();
                }
            }
        ]
    }

    Kirigami.PromptDialog {
        id: saveDialog
        property alias presetName: saveField.text

        title: i18n("Save Current Layout")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: {
            if (saveField.text.trim().length > 0) {
                kcm.saveCurrentLayout(saveField.text.trim());
            }
        }

        QQC2.TextField {
            id: saveField
            placeholderText: i18n("Preset name")
            onAccepted: saveDialog.accepted()
        }
    }

    Kirigami.PromptDialog {
        id: renameDialog
        property string presetId: ""
        property alias presetName: renameField.text

        title: i18n("Rename Preset")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: {
            if (renameField.text.trim().length > 0) {
                kcm.renamePreset(presetId, renameField.text.trim());
            }
        }

        QQC2.TextField {
            id: renameField
            onAccepted: renameDialog.accepted()
        }
    }

    Kirigami.PromptDialog {
        id: deleteDialog
        property string presetId: ""
        property string presetName: ""

        title: i18n("Delete Preset")
        subtitle: i18n("Delete the saved preset \"%1\"? This cannot be undone.", presetName)
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: kcm.deletePreset(presetId)
    }
}
