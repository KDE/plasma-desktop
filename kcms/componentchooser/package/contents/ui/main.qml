/***************************************************************************
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

import QtQuick 2.12

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.5 as KCM

KCM.SimpleKCM {
    Kirigami.FormLayout {
        ComponentComboBox {
            component: kcm.browsers
            label: i18n("Web browser:")

            KCM.SettingHighlighter {
                highlight: !kcm.browsers.isDefaults
            }
        }
        ComponentComboBox {
            component: kcm.fileManagers
            label: i18n("File manager:")

            KCM.SettingHighlighter {
                highlight: !kcm.fileManagers.isDefaults
            }
        }
        ComponentComboBox {
            component: kcm.emailClients
            label: i18n("Email client:")

            KCM.SettingHighlighter {
                highlight: !kcm.emailClients.isDefaults
            }
        }
        ComponentComboBox {
            component: kcm.terminalEmulators
            label: i18n("Terminal emulator:")

            KCM.SettingHighlighter {
                highlight: !kcm.terminalEmulators.isDefaults
            }
        }
    }
}
