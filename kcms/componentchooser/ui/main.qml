/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    function unsupportedMimeText(kcm_component) {
        return i18n("’%1’ seems to not support the following mimetypes associated with this kind of application: %2", kcm_component.applications[kcm_component.index]["name"], kcm_component.unsupportedMimeTypes.join(", "))
    }

    topPadding: 0
    bottomPadding: Kirigami.Units.gridUnit

    ComponentOverlay {
        id: overlay
        parent: root.QQC2.Overlay.overlay
        width: Math.min(root.width, Kirigami.Units.gridUnit * 25)
    }

    component MimeMessage: Kirigami.InlineMessage {
        property var componentChooser
        Layout.fillWidth: true
        visible: componentChooser.unsupportedMimeTypes.length > 0 || componentChooser.mimeTypesNotAssociated.length > 0
        type: Kirigami.MessageType.Warning
        text: i18nc("@info:status", "This application may not be able to open all file types.");
        actions: Kirigami.Action {
            text: i18nc("@action:button", "View Details")
            onTriggered: {
                overlay.componentChooser = componentChooser
                overlay.open()
            }
        }
    }

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
            Kirigami.FormData.label: i18nc("Internet related application’s category’s name", "Internet")
            Kirigami.FormData.isSection: true
        }

        ComponentComboBox {
            id: browserCombo
            Kirigami.FormData.label: i18n("Web browser:")
            component: kcm.browsers
            Layout.preferredWidth: form.longestComboBox

            KCM.SettingHighlighter {
                highlight: !kcm.browsers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.browsers
        }

        ComponentComboBox {
            id: emailCombo
            Kirigami.FormData.label: i18n("Email client:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.emailClients

            KCM.SettingHighlighter {
                highlight: !kcm.emailClients.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.emailClients
        }

        ComponentComboBox {
            id: dialerCombo
            Kirigami.FormData.label: i18nc("Default phone app", "Phone Numbers:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.telUriHandlers

            KCM.SettingHighlighter {
                highlight: !kcm.telUriHandlers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.telUriHandlers
        }


        Item {
            Kirigami.FormData.label: i18nc("Multimedia related application’s category’s name", "Multimedia")
            Kirigami.FormData.isSection: true
        }

        ComponentComboBox {
            id: imageViewerCombo
            Kirigami.FormData.label: i18n("Image viewer:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.imageViewers

            KCM.SettingHighlighter {
                highlight: !kcm.imageViewers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.imageViewers
        }

        ComponentComboBox {
            id: musicPlayerCombo
            Kirigami.FormData.label: i18n("Music player:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.musicPlayers

            KCM.SettingHighlighter {
                highlight: !kcm.musicPlayers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.musicPlayers
        }

        ComponentComboBox {
            id: videoPlayerCombo
            Kirigami.FormData.label: i18n("Video player:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.videoPlayers

            KCM.SettingHighlighter {
                highlight: !kcm.videoPlayers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.videoPlayers
        }


        Item {
            Kirigami.FormData.label: i18nc("Documents related application’s category’s name", "Documents")
            Kirigami.FormData.isSection: true
        }

        ComponentComboBox {
            id: textEditorCombo
            Kirigami.FormData.label: i18n("Text editor:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.textEditors

            KCM.SettingHighlighter {
                highlight: !kcm.textEditors.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.textEditors
        }


        ComponentComboBox {
            id: pdfViewerCombo
            Kirigami.FormData.label: i18n("PDF viewer:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.pdfViewers

            KCM.SettingHighlighter {
                highlight: !kcm.pdfViewers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.pdfViewers
        }


        Item {
            Kirigami.FormData.label: i18nc("Utilities related application’s category’s name", "Utilities")
            Kirigami.FormData.isSection: true
        }

        ComponentComboBox {
            id: fileManagerCombo
            Kirigami.FormData.label: i18n("File manager:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.fileManagers

            KCM.SettingHighlighter {
                highlight: !kcm.fileManagers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.fileManagers
        }

        ComponentComboBox {
            id: terminalCombo
            Kirigami.FormData.label: i18n("Terminal emulator:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.terminalEmulators

            KCM.SettingHighlighter {
                highlight: !kcm.terminalEmulators.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.terminalEmulators
        }

        ComponentComboBox {
            id: archiveCombo
            Kirigami.FormData.label: i18n("Archive manager:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.archiveManagers

            KCM.SettingHighlighter {
                highlight: !kcm.archiveManagers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.archiveManagers
        }

        ComponentComboBox {
            id: mapCombo
            Kirigami.FormData.label: i18nc("Map related application’s category’s name", "Map:")
            Layout.preferredWidth: form.longestComboBox
            component: kcm.geoUriHandlers

            KCM.SettingHighlighter {
                highlight: !kcm.geoUriHandlers.isDefaults
            }
        }
        MimeMessage {
            componentChooser: kcm.geoUriHandlers
        }
    }
}
