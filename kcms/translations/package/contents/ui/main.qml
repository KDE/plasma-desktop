/*
 * Copyright (C) 2015 Marco Martin <mart@kde.org>
 * Copyright (C) 2018 Eike Hein <hein@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kcm 1.2

ScrollViewKCM {
    id: root

    ConfigModule.quickHelp: i18n("Language")

    PlasmaCore.SortFilterModel {
        id: availableLanguagesModel

        sourceModel: kcm.translationsModel

        filterRole: "IsSelected"
        filterCallback: function(source_row, value) { return !value; }

        sortRole: "display"
    }

    Kirigami.OverlaySheet {
        id: addLanguagesSheet

        parent: root.parent

        topPadding: 0
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0

        header: Kirigami.Heading { text: i18nc("@title:window", "Add Languages") }

        property var selectedLanguages: []

        onSheetOpenChanged: selectedLanguages = []

        ListView {
            implicitWidth: 18 * Kirigami.Units.gridUnit

            model: availableLanguagesModel

            delegate: Kirigami.BasicListItem {
                property string languageCode: model.LanguageCode

                reserveSpaceForIcon: false

                label: model.display

                checkable: true
                onCheckedChanged: {
                    if (checked) {
                        addLanguagesSheet.selectedLanguages.push(index);

                        // There's no property change notification for pushing to an array
                        // in a var prop, so we can't bind selectedLanguages.length to
                        // addLanguagesButton.enabled.
                        addLanguagesButton.enabled = true;
                    } else {
                        addLanguagesSheet.selectedLanguages = addLanguagesSheet.selectedLanguages.filter(function(item) { return item !== index });

                        // There's no property change notification for pushing to an array
                        // in a var prop, so we can't bind selectedLanguages.length to
                        // addLanguagesButton.enabled.
                        if (!addLanguagesSheet.selectedLanguages.length) {
                            addLanguagesButton.enabled = false;
                        }
                    }
                }
            }
        }

        footer: RowLayout {
            QtControls.Button {
                id: addLanguagesButton

                Layout.alignment: Qt.AlignHCenter

                text: i18nc("@action:button", "Add")

                enabled: false

                onClicked: {
                    var langs = kcm.translationsModel.selectedLanguages.slice();
                    addLanguagesSheet.selectedLanguages.sort().forEach(function(index) {
                        langs.push(availableLanguagesModel.get(index).LanguageCode);
                    });

                    kcm.translationsModel.selectedLanguages = langs;

                    addLanguagesSheet.sheetOpen = false;
                }
            }
        }
    }

    header: ColumnLayout {
        id: messagesLayout

        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true

            type: Kirigami.MessageType.Information

            text: i18nc("@info", "There are no languages available on this system.")

            visible: !availableLanguagesModel.count
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true

            type: kcm.everSaved ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information

            text: (kcm.everSaved ? i18nc("@info", "Your changes will take effect the next time you log in.")
                : i18nc("@info", "There are currently no preferred languages configured."))

            visible: !languagesList.count || kcm.everSaved
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true

            type: Kirigami.MessageType.Error

            text: i18ncp("@info %2 is the language code",
                "The translation files for the language with the code '%2' could not be found. The language will be removed from your configuration. If you want to add it back, please install the localization files for it and add the language again.",
                "The translation files for the languages with the codes '%2' could not be found. These languages will be removed from your configuration. If you want to add them back, please install the localization files for it and the languages again.",
                kcm.translationsModel.missingLanguages.length,
                kcm.translationsModel.missingLanguages.join("', '"))

            visible: kcm.translationsModel.missingLanguages.length
        }

        QtControls.Label {
            Layout.fillWidth: true

            visible: languagesList.count

            text: i18n("The language at the top of this list is the one you want to see and use most often.")
        }
    }

    view: ListView {
        id: languagesList

        model: PlasmaCore.SortFilterModel {
            sourceModel: kcm.translationsModel

            filterRole: "IsSelected"
            filterCallback: function(source_row, value) { return value; }

            sortRole: "SelectedPriority"
        }

        delegate: Kirigami.SwipeListItem {
            id: listItem

            width: ListView.view.width

            contentItem: RowLayout {
                width: implicitWidth
                height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)

                anchors.verticalCenter: parent.verticalCenter

                Kirigami.Icon {
                    visible: model.IsMissing

                    Layout.alignment: Qt.AlignVCenter

                    width: Kirigami.Units.iconSizes.smallMedium
                    height: width

                    source: "error"
                    color: Kirigami.Theme.negativeTextColor
                }

                QtControls.Label {
                    Layout.fillWidth: true

                    Layout.alignment: Qt.AlignVCenter

                    text: (index == 0) ? i18nc("@item:inlistbox 1 = Language name", "%1 (Default)", model.display) : model.display

                    color: (model.IsMissing ? Kirigami.Theme.negativeTextColor
                        : (listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate)
                        ? listItem.activeTextColor : listItem.textColor))
                }
            }

        actions: [
            Kirigami.Action {
                enabled: !model.IsMissing && index > 0
                iconName: "go-top"
                tooltip: i18nc("@info:tooltip", "Promote to default")
                onTriggered: kcm.translationsModel.moveSelectedLanguage(index, 0)
            },
            Kirigami.Action {
                enabled: !model.IsMissing && index > 0
                iconName: "go-up"
                tooltip: i18nc("@info:tooltip", "Move up")
                onTriggered: kcm.translationsModel.moveSelectedLanguage(index, index - 1)
            },
            Kirigami.Action {
                enabled: !model.IsMissing && index < (languagesList.count - 1)
                iconName: "go-down"
                tooltip: i18nc("@info:tooltip", "Move down")
                onTriggered: kcm.translationsModel.moveSelectedLanguage(index, index + 1)
            },
            Kirigami.Action {
                enabled: !model.IsMissing
                iconName: "list-remove"
                tooltip: i18nc("@info:tooltip", "Remove")
                onTriggered: kcm.translationsModel.removeSelectedLanguage(model.LanguageCode)
            }]
        }
    }

    footer: RowLayout {
        id: footerLayout

        QtControls.Button {
            Layout.alignment: Qt.AlignRight

            enabled: availableLanguagesModel.count

            text: i18nc("@action:button", "Add languages...")

            onClicked: addLanguagesSheet.sheetOpen = !addLanguagesSheet.sheetOpen

            checkable: true
            checked: addLanguagesSheet.sheetOpen
        }
    }
}

