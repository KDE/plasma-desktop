/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>
   Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
   Copyright (c) 2019 Kevin Ottens <kevin.ottens@enioka.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.newstuff 1.62 as NewStuff
import org.kde.kcm 1.1 as KCM
import org.kde.private.kcms.desktoptheme 1.0 as Private


KCM.GridViewKCM {
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the Plasma style.")

    view.model: kcm.filteredModel
    view.currentIndex: kcm.filteredModel.selectedThemeIndex

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

    enabled: !kcm.downloadingFile && !kcm.desktopThemeSettings.isImmutable("name")

    DropArea {
        anchors.fill: parent
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false;
            }
        }
        onDropped: kcm.installThemeFromFile(drop.urls[0])
    }
     header: RowLayout {
        Layout.fillWidth: true

        QtControls.TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: i18n("Search...")
            leftPadding: LayoutMirroring.enabled ? clearButton.width : undefined
            rightPadding: LayoutMirroring.enabled ? undefined : clearButton.width
            // this could be useful as a component
            MouseArea {
                id: clearButton
                anchors {
                top: parent.top
                    topMargin: parent.topPadding
                    right: parent.right
                    // the TextField's padding is taking into account the clear button's size
                    // so we just use the opposite one for positioning the clear button
                    rightMargin: LayoutMirroring.enabled ? parent.rightPadding: parent.leftPadding
                    bottom: parent.bottom
                    bottomMargin: parent.bottomPadding
                }
                width: height

                opacity: searchField.length > 0 ? 1 : 0
                onClicked: searchField.clear()

                Kirigami.Icon {
                    anchors.fill: parent
                    active: parent.pressed
                    source: "edit-clear-locationbar-" + (LayoutMirroring.enabled ? "ltr" : "rtl")
                }

                Behavior on opacity {
                    NumberAnimation { duration: Kirigami.Units.longDuration }
                }
            }
        }
        QtControls.ComboBox {
            id: filterCombo
            textRole: "text"
            model: [
                {text: i18n("All Themes"), filter: Private.FilterProxyModel.AllThemes},
                {text: i18n("Light Themes"), filter: Private.FilterProxyModel.LightThemes},
                {text: i18n("Dark Themes"), filter: Private.FilterProxyModel.DarkThemes},
                {text: i18n("Color scheme compatible"), filter: Private.FilterProxyModel.ThemesFollowingColors}
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

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        subtitle: model.colorType == Private.ThemesModel.FollowsColorTheme
            && view.model.filter != Private.FilterProxyModel.ThemesFollowingColors ? i18n("Follows color scheme") : ""
        toolTip: model.description || model.display

        opacity: model.pendingDeletion ? 0.3 : 1
        Behavior on opacity {
            NumberAnimation { duration: Kirigami.Units.longDuration }
        }

        thumbnailAvailable: true
        thumbnail: ThemePreview {
            id: preview
            anchors.fill: parent
            themeName: model.pluginName
        }

        actions: [
            Kirigami.Action {
                iconName: "document-edit"
                tooltip: i18n("Edit Theme...")
                enabled: !model.pendingDeletion
                visible: kcm.canEditThemes
                onTriggered: kcm.editTheme(model.pluginName)
            },
            Kirigami.Action {
                iconName: "edit-delete"
                tooltip: i18n("Remove Theme")
                enabled: model.isLocal
                visible: !model.pendingDeletion
                onTriggered: model.pendingDeletion = true;
            },
            Kirigami.Action {
                iconName: "edit-undo"
                tooltip: i18n("Restore Theme")
                visible: model.pendingDeletion
                onTriggered: model.pendingDeletion = false;
            }
        ]

        onClicked: {
            kcm.desktopThemeSettings.name = model.pluginName;
            view.forceActiveFocus();
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
                }
                onShowErrorMessage: {
                    infoLabel.type = Kirigami.MessageType.Error;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight

            QtControls.Button {
                text: i18n("Install from File...")
                icon.name: "document-import"
                onClicked: fileDialogLoader.active = true;
            }

            NewStuff.Button {
                id: newStuffButton
                text: i18n("Get New Plasma Styles...")
                configFile: "plasma-themes.knsrc"
                viewMode: NewStuff.Page.ViewMode.Preview
                onChangedEntriesChanged: kcm.load();
            }
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: FileDialog {
            title: i18n("Open Theme")
            folder: shortcuts.home
            nameFilters: [ i18n("Theme Files (*.zip *.tar.gz *.tar.bz2)") ]
            Component.onCompleted: open()
            onAccepted: {
                kcm.installThemeFromFile(fileUrls[0])
                fileDialogLoader.active = false
            }
            onRejected: {
                fileDialogLoader.active = false
            }
        }
    }
}
