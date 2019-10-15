import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kcm 1.2 as KCM
import org.kde.kquickcontrols 2.0 as KQuickControls

KCM.ScrollViewKCM {
    id: root

    property var dataModel;
    signal changed();

    Controls.Dialog {
        id: addLayoutDialog
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Discard
        title: i18n("Select Layout")

        x: Math.floor((parent.width - width) / 2)
        y: Math.floor((parent.height - height) / 2)
        width: 500
        height: Math.floor(Math.min(parent.height, 500));

        property var selected: []

        ColumnLayout {
            anchors.fill: parent

            Component {
                id: sectionHeading
                Rectangle {
                    width: parent.width
                    height: childrenRect.height
                    color: "transparent"

                    Text {
                        text: section
                        font.bold: true
                        font.pixelSize: 20
                    }
                }
            }

            ListView {
                id: layoutSelectList
                implicitWidth: parent.width
                Layout.fillHeight: true

                Controls.ScrollBar.vertical: Controls.ScrollBar {}

                model: PlasmaCore.SortFilterModel {
                    id: nameFilterModel
                    sourceModel: PlasmaCore.SortFilterModel {
                        id: enabledFilterModel
                        sourceModel: dataModel.layoutListModel

                        filterRole: "enabled"
                        filterCallback: function(source_row, value) { return !value; }

                        sortRole: "languages"
                    }

                    filterRole: "description"
                    filterCaseSensitivity: Qt.CaseInsensitive
                    filterString: filterText.text
                }

                clip: true

                section.property: "languages"
                section.delegate: sectionHeading

                delegate: Kirigami.BasicListItem {
                    label: model.description
                    checkable: true
                    icon: "input-keyboard"

                    function origIdx() {
                        return enabledFilterModel.mapRowToSource(
                            nameFilterModel.mapRowToSource(model.index));
                    }

                    checked: addLayoutDialog.selected[origIdx()] === true
                    onCheckedChanged: addLayoutDialog.selected[origIdx()] = checked;
                }
            }

            Controls.TextArea {
                id: filterText
                implicitWidth: parent.width
            }
        }

        onOpened: filterText.forceActiveFocus()
        onOpenedChanged: filterText.text = ""

        onAccepted: {
            selected.forEach(function(checked, index) {
                if (checked) {
                    dataModel.layoutListModel.add(index);
                    changed();
                }
            });
            close();
        }
        onDiscarded: close();
        onClosed: {
            selected = []
            layoutSelectList.positionViewAtBeginning();
        }
    }

    Controls.Dialog {
        id: previewDialog
        title: "Preview"
        standardButtons: Controls.Dialog.Ok

        x: Math.floor((parent.width - width) / 2)
        y: Math.floor((parent.height - height) / 2)
        width: 500
        height: Math.floor(Math.min(parent.height, 500));

        LayoutPreview {
            id: preview
        }
    }

    header: Kirigami.FormLayout {
        id: formLayout

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Layout Indicator Icon")
        }

        Controls.CheckBox {
            id: showLayoutIndicatorIconCheckbox
            Kirigami.FormData.label: i18n("Show layout indicator icon")

            Binding on checked {
                value: dataModel.showLayoutIndicator
            }

            onCheckedChanged: {
                dataModel.showLayoutIndicator = checked;
                root.changed();
            }
        }

        Controls.CheckBox {
            Kirigami.FormData.label: i18n("Show for single layout")

            enabled: showLayoutIndicatorIconCheckbox.checked

            Binding on checked {
                value: dataModel.showForSingleLayout
            }

            onCheckedChanged: {
                dataModel.showForSingleLayout = checked;
                root.changed();
            }
        }

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Shortcuts for Switching Layout")
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Main shift key:")
            model: dataModel.mainShiftKeyModel

            Binding on currentIndex {
                value: dataModel.mainShiftKeyIndex
            }

            onActivated: {
                dataModel.mainShiftKeyIndex = currentIndex;
                root.changed();
            }
        }

        Controls.ComboBox {
            Kirigami.FormData.label: i18n("3rd level shift key:")
            model: dataModel.thirdLevelShortcutModel

            Binding on currentIndex {
                value: dataModel.thirdLevelShortcutIndex
            }

            onActivated: {
                dataModel.thirdLevelShortcutIndex = currentIndex;
                root.changed();
            }
        }

        KQuickControls.KeySequenceItem {
            Kirigami.FormData.label: i18n("Cycle through layouts:")
            width: 200

            Binding on keySequence {
                value: dataModel.alternativeShortcut
            }

            onKeySequenceChanged: {
                dataModel.alternativeShortcut = keySequence;
                root.changed();
            }
        }

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Layouts")
        }

        Kirigami.InlineMessage {
            Kirigami.FormData.isSection: true
            id: inlineMessage

            PlasmaCore.SortFilterModel {
                id: missingLayouts
                sourceModel: dataModel.layoutListModel

                filterRole: "source"
                filterCallback: function(source_row, value) { return value == 0; }

                function join(str) {
                    var l = []
                    for (var i = 0; i < missingLayouts.count; ++i) {
                        l.push(missingLayouts.get(i).save_name);
                    }
                    return l.join(str);
                }
            }

            Layout.fillWidth: true

            type: Kirigami.MessageType.Error

            Connections {
                target: dataModel.layoutListModel
                onMissingCountChanged: {
                    inlineMessage.text =
                            i18ncp("@info %2 is the layout code",
                                "The input support for the keyboard layout with the code '%2' could not be found. If it is not needed, please remove it from your configuration.",
                                "The input support for the keyboard layouts with the codes '%2' could not be found. If they are not needed, please remove them from your configuration.",
                                missingLayouts.count,
                                missingLayouts.join("', '"))
                    inlineMessage.visible = missingLayouts.count > 0
                }
            }
        }
    }

    Component {
        id: listviewDelegateComponent
        Kirigami.SwipeListItem {
            checkable: true
            id: listItem

            contentItem: RowLayout {
                Kirigami.ListItemDragHandle {
                    listItem: listItem
                    listView: layoutList
                    onMoveRequested: {
                        dataModel.currentLayoutListModel.simulateMove(oldIndex, newIndex);
                    }
                    onDropped: {
                        dataModel.currentLayoutListModel.applyOrderChanges();
                        changed();
                    }
                }

                Kirigami.Icon {
                    visible: model.source == 0

                    Layout.alignment: Qt.AlignVCenter

                    width: Kirigami.Units.iconSizes.smallMedium
                    height: width

                    source: "error"
                    color: Kirigami.Theme.negativeTextColor
                }

                Controls.Label {
                    Layout.fillWidth: true
                    text: model.description
                }
            }

            onClicked: {
                if (checked) {
                    if (layoutList.checkedItem.length === 1) {
                        layoutList.checkedItem[0].item.checked = false;
                    }
                    layoutList.checkedItem[0] = {index: model.index, model: model, item: this};
                    configureLayoutButton.enabled = model.is_configurable;
                }
                else {
                    layoutList.checkedItem = []
                    configureLayoutButton.enabled = false;
                }
            }


            actions: [
                Kirigami.Action {
                    iconName: "list-remove"
                    tooltip: i18nc("@info:tooltip", "Remove")
                    onTriggered: {
                        dataModel.currentLayoutListModel.remove(index)
                        changed();
                    }
                }]
        }
    }


    view: ListView {
        id: layoutList

        property var checkedItem;

        Component.onCompleted: checkedItem = [];

        Rectangle {
            color: "white"
            anchors.fill: parent
            z: -10
        }

        implicitWidth: formLayout.width - buttonColumn.width - parent.spacing
        implicitHeight: 300

        model: dataModel.currentLayoutListModel

        clip: true

        Controls.ScrollBar.vertical: Controls.ScrollBar {
            active: true
        }

        delegate: Kirigami.DelegateRecycler {
            width: layoutList.width
            sourceComponent: listviewDelegateComponent
        }
    }

    footer: Row {
        id: buttonColumn

        Controls.Button {
            text: i18n("Add...")
            icon.name: "list-add"

            onClicked: addLayoutDialog.open();
        }

        /* TODO
        Controls.Button {
            text: i18n("Preview...")
            onClicked: previewDialog.open();
        }
        */

        Controls.Button {
            id: configureLayoutButton
            text: i18n("Configure...");
            enabled: false;
            icon.name: "configure"

            onClicked: {
                var item = layoutList.checkedItem[0].model;
                var component;
                var imConfigDialog;
                if (item.source === 1) { // xkb
                    component = Qt.createComponent("XkbLayoutConfig.qml");
                }
                else if (item.source === 2) { // fcitx
                    component = Qt.createComponent("FcitxIMConfig.qml");
                }
                else {
                    return;
                }

                if (component.status === Component.Ready) {
                    imConfigDialog = component.createObject(root.parent, {width:700, height:500})
                    imConfigDialog.openForModel(item.config_model);
                }
                else {
                    console.log(component.errorString())
                }

            }
        }
    }
}
