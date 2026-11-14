/*
    SPDX-FileCopyrightText: 2026 Alexander Berg <wehrwolfmann@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.Form {
    Kirigami.FormGroup {
        Kirigami.FormAction {
            subtitle: i18nc("@info", "Recommended in most situations. Makes text and everything else bigger and keeps their proportions; on low-resolution screens, some small graphics may look slightly blurry.")
            action: QQC2.Action {
                text: i18nc("@action:button", "Configure Display Scale…")
                icon.name: "preferences-desktop-display"
                onTriggered: KCM.KCMLauncher.openSystemSettings("kcm_kscreen")
            }
        }

        Kirigami.FormAction {
            subtitle: i18nc("@info", "Only suitable for small size changes. Changes the text alone, so large changes can make text no longer fit or look good next to icons and other elements.")
            action: QQC2.Action {
                text: i18nc("@action:button", "Configure Fonts…")
                icon.name: "preferences-desktop-font"
                onTriggered: KCM.KCMLauncher.openSystemSettings("kcm_fonts")
            }
        }
    }
}
