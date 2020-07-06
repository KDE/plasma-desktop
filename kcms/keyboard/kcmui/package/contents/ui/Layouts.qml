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

    Kirigami.OverlaySheet {
        id: addLayoutDialog
        
        property var selected: []
        
        parent: root.parent.parent
        
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0
        
        header: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            
            Kirigami.Heading {
                text: i18nc("@title:window", "Select Layout")
            }
            
            Controls.TextArea {
                id: filterText
                implicitWidth: parent.width
            }
        }
        
        onSheetOpenChanged: {
            if (sheetOpen) {
                filterText.forceActiveFocus();
                filterText.text = "";
            } else {
                selected = []
                layoutSelectList.positionViewAtBeginning();
            }
        }

        ListView {
            id: layoutSelectList

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

        
        footer: RowLayout {
            Controls.Button {
                Layout.alignment: Qt.AlignRight
                text: i18nc("@action:button", "Add")
                
                onClicked: {
                    selected.forEach(function(checked, index) {
                        if (checked) {
                            dataModel.layoutListModel.add(index);
                            changed();
                        }
                    });
                    addLayoutDialog.sheetOpen = false;
                }
            }
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

            actions: [
                Kirigami.Action {
                    iconName: "configure"
                    tooltip: i18nc("@info:tooltip", "Configure")
                    onTriggered: {
                        var component;
                        var imConfigDialog;
                        if (model.source === 1) { // xkb
                            component = Qt.createComponent("XkbLayoutConfig.qml");
                        }
                        else if (model.source === 2) { // fcitx
                            component = Qt.createComponent("FcitxIMConfig.qml");
                        }
                        else {
                            return;
                        }

                        if (component.status === Component.Ready) {
                            imConfigDialog = component.createObject(root.parent, {width:700, height:500})
                            imConfigDialog.openForModel(model.config_model);
                        }
                        else {
                            console.log(component.errorString())
                        }
                    }
                },
                Kirigami.Action {
                    iconName: "list-remove"
                    tooltip: i18nc("@info:tooltip", "Remove")
                    onTriggered: {
                        dataModel.currentLayoutListModel.remove(index)
                        changed();
                    }
                }
            ]
        }
    }


    view: ListView {
        id: layoutList

        model: dataModel.currentLayoutListModel

        clip: true

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

            onClicked: addLayoutDialog.sheetOpen = true;
        }

        /* TODO
        Controls.Button {
            text: i18n("Preview...")
            onClicked: previewDialog.open();
        }
        */
    }
}
