/*
 * Copyright 2018 Kai Uwe Broulik <kde@privat.broulik.de>
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

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.0 as QtDialogs
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.8 as Kirigami
import org.kde.newstuff 1.62 as NewStuff
import org.kde.kcm 1.3 as KCM
import org.kde.private.kcms.colors 1.0 as Private

KCM.GridViewKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the color scheme.")

    view.model: kcm.filteredModel
    view.currentIndex: kcm.filteredModel.selectedSchemeIndex

    Binding {
        target: kcm.filteredModel
        property: "query"
        value: searchField.text
    }

    Binding {
        target: kcm.filteredModel
        property: "filter"
        value:  filterCombo.model[filterCombo.currentIndex].filter
    }

    KCM.SettingStateBinding {
        configObject: kcm.colorsSettings
        settingName: "colorScheme"
        extraEnabledConditions: !kcm.downloadingFile
    }

    Component.onCompleted: {
        // The thumbnails are a bit more elaborate and need more room, especially when translated
        view.implicitCellWidth = Kirigami.Units.gridUnit * 13;
        view.implicitCellHeight = Kirigami.Units.gridUnit * 12;
    }

    DropArea {
        anchors.fill: parent
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false;
            }
        }
        onDropped: {
            infoLabel.visible = false;
            kcm.installSchemeFromFile(drop.urls[0]);
        }
    }

    // putting the InlineMessage as header item causes it to show up initially despite visible false
    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: notInstalledWarning
            Layout.fillWidth: true

            type: Kirigami.MessageType.Warning
            showCloseButton: true
            visible: false

            Connections {
                target: kcm
                onShowSchemeNotInstalledWarning: {
                    notInstalledWarning.text = i18n("The color scheme '%1' is not installed. Selecting the default theme instead.", schemeName)
                    notInstalledWarning.visible = true;
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Kirigami.SearchField {
                id: searchField
                Layout.fillWidth: true
            }

            QtControls.ComboBox {
                id: filterCombo
                textRole: "text"
                model: [
                    {text: i18n("All Schemes"), filter: Private.KCM.AllSchemes},
                    {text: i18n("Light Schemes"), filter: Private.KCM.LightSchemes},
                    {text: i18n("Dark Schemes"), filter: Private.KCM.DarkSchemes}
                ]

                // HACK QQC2 doesn't support icons, so we just tamper with the desktop style ComboBox's background
                // and inject a nice little filter icon.
                Component.onCompleted: {
                    if (!background || !background.hasOwnProperty("properties")) {
                        // not a KQuickStyleItem
                        return;
                    }

                    var props = background.properties || {};

                    background.properties = Qt.binding(function() {
                        var newProps = props;
                        newProps.currentIcon = "view-filter";
                        newProps.iconColor = Kirigami.Theme.textColor;
                        return newProps;
                    });
                }
            }
        }
    }

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display

        thumbnailAvailable: true
        thumbnail: Rectangle {
            anchors.fill: parent

            opacity: model.pendingDeletion ? 0.3 : 1
            Behavior on opacity {
                NumberAnimation { duration: Kirigami.Units.longDuration }
            }

            color: model.palette.window

            Kirigami.Theme.highlightColor: model.palette.highlight
            Kirigami.Theme.highlightedTextColor: model.palette.highlightedText
            Kirigami.Theme.linkColor: model.palette.link
            Kirigami.Theme.textColor: model.palette.text

            Rectangle {
                id: windowTitleBar
                width: parent.width
                height: Math.round(Kirigami.Units.gridUnit * 1.5)
                gradient: Gradient {
                    // from Breeze Decoration::paintTitleBar
                    GradientStop { position: 0.0; color: Qt.lighter(model.activeTitleBarBackground, 1.2) }
                    GradientStop { position: 0.8; color: model.activeTitleBarBackground }
                }

                color: model.activeTitleBarBackground

                QtControls.Label {
                    anchors {
                        fill: parent
                        leftMargin: Kirigami.Units.smallSpacing
                        rightMargin: Kirigami.Units.smallSpacing
                    }
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: model.activeTitleBarForeground
                    text: i18n("Window Title")
                    elide: Text.ElideRight
                }
            }

            ColumnLayout {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: windowTitleBar.bottom
                    bottom: parent.bottom
                    margins: Kirigami.Units.smallSpacing
                }

                RowLayout {
                    Layout.fillWidth: true

                    QtControls.Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        verticalAlignment: Text.AlignVCenter
                        text: i18n("Window text")
                        elide: Text.ElideRight
                        color: model.palette.windowText
                    }

                    QtControls.Button {
                        Layout.alignment: Qt.AlignBottom
                        text: i18n("Button")
                        Kirigami.Theme.backgroundColor: model.palette.button
                        Kirigami.Theme.textColor: model.palette.buttonText
                        activeFocusOnTab: false
                    }
                }

                QtControls.Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Kirigami.Theme.backgroundColor: model.palette.base
                    // FIXME Make Frame still visible but without any inner padding
                    padding: 1
                    activeFocusOnTab: false

                    Column {
                        id: listPreviewColumn

                        readonly property string demoText: " <a href='#'>%2</a> <a href='#'><font color='%3'>%4</font></a>"
                            .arg(i18nc("Hyperlink", "link"))
                            .arg(model.palette.linkVisited)
                            .arg(i18nc("Visited hyperlink", "visited"))

                        width: parent.width

                        QtControls.ItemDelegate {
                            width: parent.width
                            text: i18n("Normal text") + listPreviewColumn.demoText
                            activeFocusOnTab: false
                        }

                        QtControls.ItemDelegate {
                            width: parent.width
                            highlighted: true
                            // TODO use proper highlighted link color
                            text: i18n("Highlighted text") + listPreviewColumn.demoText
                            activeFocusOnTab: false
                        }

                        QtControls.ItemDelegate {
                            width: parent.width
                            enabled: false
                            text: i18n("Disabled text") + listPreviewColumn.demoText
                            activeFocusOnTab: false
                        }
                    }
                }
            }

            // Make the preview non-clickable but still reacting to hover
            MouseArea {
                anchors.fill: parent
                onClicked: delegate.clicked()
                onDoubleClicked: delegate.doubleClicked()
            }
        }

        actions: [
            Kirigami.Action {
                iconName: "document-edit"
                tooltip: i18n("Edit Color Scheme...")
                enabled: !model.pendingDeletion
                onTriggered: kcm.editScheme(model.schemeName, root)
            },
            Kirigami.Action {
                iconName: "edit-delete"
                tooltip: i18n("Remove Color Scheme")
                enabled: model.removable
                visible: !model.pendingDeletion
                onTriggered: model.pendingDeletion = true
            },
            Kirigami.Action {
                iconName: "edit-undo"
                tooltip: i18n("Restore Color Scheme")
                visible: model.pendingDeletion
                onTriggered: model.pendingDeletion = false
            }
        ]
        onClicked: {
            kcm.model.selectedScheme = model.schemeName;
            view.forceActiveFocus();
        }
        onDoubleClicked: {
            kcm.save();
        }
    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            id: infoLabel
            Layout.fillWidth: true

            showCloseButton: true

            Connections {
                target: kcm
                onShowSuccessMessage: {
                    infoLabel.type = Kirigami.MessageType.Positive;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                    // Avoid dual message widgets
                    notInstalledWarning.visible = false;
                }
                onShowErrorMessage: {
                    infoLabel.type = Kirigami.MessageType.Error;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                    notInstalledWarning.visible = false;
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight

            QtControls.Button {
                text: i18n("Install from File...")
                icon.name: "document-import"
                onClicked: fileDialogLoader.active = true
            }

            NewStuff.Button {
                id: newStuffButton
                text: i18n("Get New Color Schemes...")
                configFile: "colorschemes.knsrc"
                viewMode: NewStuff.Page.ViewMode.Tiles
                onChangedEntriesChanged: kcm.reloadModel(newStuffButton.changedEntries);
            }
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: QtDialogs.FileDialog {
            title: i18n("Open Color Scheme")
            folder: shortcuts.home
            nameFilters: [ i18n("Color Scheme Files (*.colors)") ]
            Component.onCompleted: open()
            onAccepted: {
                infoLabel.visible = false;
                kcm.installSchemeFromFile(fileUrls[0])
                fileDialogLoader.active = false
            }
            onRejected: {
                fileDialogLoader.active = false
            }
        }
    }
}
