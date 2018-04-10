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
import org.kde.kcm 1.1

SimpleKCM {
    id: root

    ConfigModule.quickHelp: i18n("Language")

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    Kirigami.OverlaySheet {
        id: addLanguagesSheet

        parent: root.parent

        topPadding: 0
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0

        header: Kirigami.Heading { text: i18n("Add Languages") }

        property var selectedLanguages: []

        ListView {
            implicitWidth: Kirigami.Units.gridUnit * 18
            implicitHeight: Kirigami.Units.gridUnit * 15

            model: PlasmaCore.SortFilterModel {
                id: availableLanguagesModel

                sourceModel: addLanguagesSheet.sheetOpen ? kcm.translationsModel : null

                filterRole: "IsSelected"
                filterCallback: function(source_row, value) { return !value; }

                sortRole: "display"
            }

            delegate: Kirigami.BasicListItem {
                Layout.fillWidth: true

                property string languageCode: model.LanguageCode

                reserveSpaceForIcon: false

                label: model.display

                checkable: true
                onCheckedChanged: {
                    if (checked) {
                        addLanguagesSheet.selectedLanguages.push(index);
                    } else {
                        addLanguagesSheet.selectedLanguages = addLanguagesSheet.selectedLanguages.filter(function(item) { return item !== index })
                    }
                }
            }
        }

        footer: QtControls.Button {
            width: parent.width

            text: i18n("Add")

            onClicked: {
                var langs = [];
                addLanguagesSheet.selectedLanguages.sort().forEach(function(index) {
                    langs.push(availableLanguagesModel.get(index).LanguageCode);
                });

                if (addLanguagesSheet.selectedLanguages.length) {
                    if (kcm.translationsModel.selectedLanguages.length) {
                        kcm.translationsModel.selectedLanguages = kcm.translationsModel.selectedLanguages.concat(langs);
                    } else {
                        kcm.translationsModel.selectedLanguages = langs;
                    }
                }

                addLanguagesSheet.sheetOpen = false;
            }
        }
    }

    header: QtControls.Control {
        width: parent.width
        height: messagesLayout.implicitHeight + (messagesLayout.implicitHeight ? bottomPadding : 0)

        bottomPadding: Kirigami.Units.smallSpacing

        ColumnLayout {
            id: messagesLayout

            width: parent.width

            spacing: Kirigami.Units.largeSpacing

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.rightMargin: Kirigami.Units.smallSpacing

                type: kcm.everSaved ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information

                text: (kcm.everSaved ? i18n("Your changes will take effect the next time you log in.")
                    : i18n("There are currently no preferred languages configured."))

                visible: !languagesList.count || kcm.everSaved
            }

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.rightMargin: Kirigami.Units.smallSpacing

                type: Kirigami.MessageType.Error

                text: i18ncp("%2 is the language code",
                    "The translation files for the language with the code '%2' could not be found. The language will be removed from your configuration. If you want to add it back, please install the localization files for it and add the language again.",
                    "The translation files for the languages with the codes '%2' could not be found. These languages will be removed from your configuration. If you want to add them back, please install the localization files for it and the languages again.",
                    kcm.translationsModel.missingLanguages.length,
                    kcm.translationsModel.missingLanguages.join("', '"))

                visible: kcm.translationsModel.missingLanguages.length
            }
        }
    }

    ListView {
        id: languagesList

        Layout.fillWidth: true

        model: PlasmaCore.SortFilterModel {
            sourceModel: kcm.translationsModel

            filterRole: "IsSelected"
            filterCallback: function(source_row, value) { return value; }

            sortRole: "SelectedPriority"
        }

        delegate: Kirigami.SwipeListItem {
            id: listItem

            Layout.fillWidth: true
            Layout.margins: 0

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

                    text: model.display

                    color: (model.IsMissing ? Kirigami.Theme.negativeTextColor
                        : (listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate)
                        ? listItem.activeTextColor : listItem.textColor))
                }
            }

        actions: [
            Kirigami.Action {
                enabled: !model.IsMissing && index > 0
                iconName: "arrow-up"
                onTriggered: kcm.translationsModel.selectedLanguages.splice(index - 1, 0, kcm.translationsModel.selectedLanguages.splice(index, 1)[0])

            },
            Kirigami.Action {
                enabled: !model.IsMissing && index < (languagesList.count - 1)
                iconName: "arrow-down"
                onTriggered: kcm.translationsModel.selectedLanguages.splice(index + 1, 0, kcm.translationsModel.selectedLanguages.splice(index, 1)[0])

            },
            Kirigami.Action {
                enabled: !model.IsMissing
                iconName: "list-remove"
                onTriggered: kcm.translationsModel.selectedLanguages = kcm.translationsModel.selectedLanguages.filter(function(item) { return item !== model.LanguageCode })
            }]
        }
    }

    footer: QtControls.Control {
        width: parent.width
        height: footerLayout.implicitHeight + topPadding

        topPadding: Kirigami.Units.smallSpacing
        leftPadding: Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.smallSpacing

        RowLayout {
            id: footerLayout

            width: parent.width

            QtControls.Button {
                Layout.alignment: Qt.AlignHCenter

                text: i18n("Add languages...")

                onClicked: addLanguagesSheet.sheetOpen = !addLanguagesSheet.sheetOpen

                checkable: true
                checked: addLanguagesSheet.sheetOpen
            }
        }
    }
}

