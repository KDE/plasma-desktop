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

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.3 as KCM
import org.kde.private.kcms.style 1.0 as Private

QtControls.Popup {
    id: effectSettingsPopup

    modal: true
    implicitWidth: formLayout.implicitWidth + 40 // Leave some room for SettingState buttons

    onOpened: {
        // can we do this automatically with "focus: true" somewhere?
        iconsOnButtonsCheckBox.forceActiveFocus();
    }

    Kirigami.FormLayout {
        id: formLayout
        // Popup's autosizing causes FormLayout to collapse when opening it a second time :(
        wideMode: true

        QtControls.CheckBox {
            id: iconsOnButtonsCheckBox
            Kirigami.FormData.label: i18n("Show icons:")
            text: i18n("On buttons")
            checked: kcm.styleSettings.iconsOnButtons
            onClicked: kcm.styleSettings.iconsOnButtons = checked

            KCM.SettingStateBinding {
                configObject: kcm.styleSettings
                settingName: "iconsOnButtons"
            }
        }

        QtControls.CheckBox {
            text: i18n("In menus")
            checked: kcm.styleSettings.iconsInMenus
            onClicked: kcm.styleSettings.iconsInMenus = checked

            KCM.SettingStateBinding {
                configObject: kcm.styleSettings
                settingName: "iconsInMenus"
            }
        }

        QtControls.ComboBox {
            id: mainToolBarStyleCombo
            Kirigami.FormData.label: i18n("Main toolbar label:")
            model: [
                {text: i18n("None"), value: Private.KCM.NoText},
                {text: i18n("Text only"), value: Private.KCM.TextOnly},
                {text: i18n("Beside icons"), value: Private.KCM.TextBesideIcon},
                {text: i18n("Below icon"), value: Private.KCM.TextUnderIcon}
            ]
            textRole: "text"
            currentIndex: model.findIndex(function (item) {
                return item.value === kcm.mainToolBarStyle
            })
            onActivated: kcm.mainToolBarStyle = model[currentIndex].value

            KCM.SettingStateBinding {
                configObject: kcm.styleSettings
                settingName: "toolButtonStyle"
            }
        }

        QtControls.ComboBox {
            Kirigami.FormData.label: i18n("Secondary toolbar label:")
            model: mainToolBarStyleCombo.model
            textRole: "text"
            currentIndex: model.findIndex(function (item) {
                return item.value === kcm.otherToolBarStyle
            })
            onActivated: kcm.otherToolBarStyle = model[currentIndex].value

            KCM.SettingStateBinding {
                configObject: kcm.styleSettings
                settingName: "toolButtonStyleOtherToolbars"
            }
        }
    }
}
