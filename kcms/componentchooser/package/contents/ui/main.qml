/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.5 as KCM

KCM.SimpleKCM {
    topPadding: -12
    bottomPadding: 0
    Kirigami.FormLayout {
        id: form

        readonly property int longestComboBox: Math.max(browserCombo.implicitWidth,
                                                        fileManagerCombo.implicitWidth,
                                                        textEditorCombo.implicitWidth,
                                                        pdfViewerCombo.implicitWidth,
                                                        imageViewerCombo.implicitWidth,
                                                        musicPlayerCombo.implicitWidth,
                                                        videoPlayerCombo.implicitWidth,
                                                        emailCombo.implicitWidth,
                                                        archiveCombo.implicitWidth,
                                                        terminalCombo.implicitWidth,
                                                        mapCombo.implicitWidth,
                                                        dialerCombo.implicitWidth)
        Item {
            Kirigami.FormData.label: i18n("Internet")
            Kirigami.FormData.isSection: true
        }
        ComponentComboBox {
            id: browserCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.browsers
            label: i18n("Web browser:")

            KCM.SettingHighlighter {
                highlight: !kcm.browsers.isDefaults
            }
        }
        ComponentComboBox {
            id: emailCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.emailClients
            label: i18n("Email client:")

            KCM.SettingHighlighter {
                highlight: !kcm.emailClients.isDefaults
            }
        }
        ComponentComboBox {
            id: dialerCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.telUriHandlers
            label: i18nc("Default phone app", "Dialer:")

            KCM.SettingHighlighter {
                highlight: !kcm.telUriHandlers.isDefaults
            }
        }
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Utilities")
            Kirigami.FormData.isSection: true
        }
        ComponentComboBox {
            id: fileManagerCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.fileManagers
            label: i18n("File manager:")

            KCM.SettingHighlighter {
                highlight: !kcm.fileManagers.isDefaults
            }
        }
        ComponentComboBox {
            id: terminalCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.terminalEmulators
            label: i18n("Terminal emulator:")

            KCM.SettingHighlighter {
                highlight: !kcm.terminalEmulators.isDefaults
            }
        }
        ComponentComboBox {
            id: archiveCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.archiveManagers
            label: i18n("Archive manager:")

            KCM.SettingHighlighter {
                highlight: !kcm.archiveManagers.isDefaults
            }
        }
        ComponentComboBox {
            id: mapCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.geoUriHandlers
            label: i18n("Map:")

            KCM.SettingHighlighter {
                highlight: !kcm.geoUriHandlers.isDefaults
            }
        }
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Documents")
            Kirigami.FormData.isSection: true
        }
        ComponentComboBox {
            id: textEditorCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.textEditors
            label: i18n("Text editor:")

            KCM.SettingHighlighter {
                highlight: !kcm.textEditors.isDefaults
            }
        }
        ComponentComboBox {
            id: pdfViewerCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.pdfViewers
            label: i18n("PDF viewer:")

            KCM.SettingHighlighter {
                highlight: !kcm.pdfViewers.isDefaults
            }
        }
        Kirigami.Separator {
            Kirigami.FormData.label: i18n("Multimedia")
            Kirigami.FormData.isSection: true
        }

        ComponentComboBox {
            id: imageViewerCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.imageViewers
            label: i18n("Image viewer:")

            KCM.SettingHighlighter {
                highlight: !kcm.imageViewers.isDefaults
            }
        }
        ComponentComboBox {
            id: musicPlayerCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.musicPlayers
            label: i18n("Music player:")

            KCM.SettingHighlighter {
                highlight: !kcm.musicPlayers.isDefaults
            }
        }
        ComponentComboBox {
            id: videoPlayerCombo
            Layout.preferredWidth: form.longestComboBox
            component: kcm.videoPlayers
            label: i18n("Video player:")

            KCM.SettingHighlighter {
                highlight: !kcm.videoPlayers.isDefaults
            }
        }


    }
}
