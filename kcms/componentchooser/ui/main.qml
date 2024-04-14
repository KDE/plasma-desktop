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
        RowLayout {
            Kirigami.FormData.label: i18n("Web browser:")

            ComponentComboBox {
                id: browserCombo
                component: kcm.browsers
                Layout.preferredWidth: form.longestComboBox

                KCM.SettingHighlighter {
                    highlight: !kcm.browsers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.browsers.isSaveNeeded && (kcm.browsers.unsupportedMimeTypes.length > 0 || kcm.browsers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.browsers
                    overlay.open()
                }
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18n("Email client:")

            ComponentComboBox {
                id: emailCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.emailClients

                KCM.SettingHighlighter {
                    highlight: !kcm.emailClients.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.emailClients.isSaveNeeded && (kcm.emailClients.unsupportedMimeTypes.length > 0 || kcm.emailClients.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.emailClients
                    overlay.open()
                }
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18nc("Default phone app", "Phone Numbers:")

            ComponentComboBox {
                id: dialerCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.telUriHandlers

                KCM.SettingHighlighter {
                    highlight: !kcm.telUriHandlers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.telUriHandlers.isSaveNeeded && (kcm.telUriHandlers.unsupportedMimeTypes.length > 0 || kcm.telUriHandlers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.telUriHandlers
                    overlay.open()
                }
            }
        }
        Item {
            Kirigami.FormData.label: i18nc("Multimedia related application’s category’s name", "Multimedia")
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Image viewer:")

            ComponentComboBox {
                id: imageViewerCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.imageViewers

                KCM.SettingHighlighter {
                    highlight: !kcm.imageViewers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.imageViewers.isSaveNeeded && (kcm.imageViewers.unsupportedMimeTypes.length > 0 || kcm.imageViewers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.imageViewers
                    overlay.open()
                }
            }
        }


        RowLayout {
            Kirigami.FormData.label: i18n("Music player:")

            ComponentComboBox {
                id: musicPlayerCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.musicPlayers

                KCM.SettingHighlighter {
                    highlight: !kcm.musicPlayers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.musicPlayers.isSaveNeeded && (kcm.musicPlayers.unsupportedMimeTypes.length > 0 || kcm.musicPlayers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.musicPlayers
                    overlay.open()
                }
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Video player:")

            ComponentComboBox {
                id: videoPlayerCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.videoPlayers

                KCM.SettingHighlighter {
                    highlight: !kcm.videoPlayers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.videoPlayers.isSaveNeeded && (kcm.videoPlayers.unsupportedMimeTypes.length > 0 || kcm.videoPlayers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.videoPlayers
                    overlay.open()
                }
            }
        }
        Item {
            Kirigami.FormData.label: i18nc("Documents related application’s category’s name", "Documents")
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Text editor:")

            ComponentComboBox {
                id: textEditorCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.textEditors

                KCM.SettingHighlighter {
                    highlight: !kcm.textEditors.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.textEditors.isSaveNeeded && (kcm.textEditors.unsupportedMimeTypes.length > 0 || kcm.textEditors.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.textEditors
                    overlay.open()
                }
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18n("PDF viewer:")

            ComponentComboBox {
                id: pdfViewerCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.pdfViewers

                KCM.SettingHighlighter {
                    highlight: !kcm.pdfViewers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.pdfViewers.isSaveNeeded && (kcm.pdfViewers.unsupportedMimeTypes.length > 0 || kcm.pdfViewers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.pdfViewers
                    overlay.open()
                }
            }
        }
        Item {
            Kirigami.FormData.label: i18nc("Utilities related application’s category’s name", "Utilities")
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18n("File manager:")

            ComponentComboBox {
                id: fileManagerCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.fileManagers

                KCM.SettingHighlighter {
                    highlight: !kcm.fileManagers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.fileManagers.isSaveNeeded && (kcm.fileManagers.unsupportedMimeTypes.length > 0 || kcm.fileManagers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.fileManagers
                    overlay.open()
                }
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18n("Terminal emulator:")

            ComponentComboBox {
                id: terminalCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.terminalEmulators

                KCM.SettingHighlighter {
                    highlight: !kcm.terminalEmulators.isDefaults
                }
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18n("Archive manager:")

            ComponentComboBox {
                id: archiveCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.archiveManagers

                KCM.SettingHighlighter {
                    highlight: !kcm.archiveManagers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.archiveManagers.isSaveNeeded && (kcm.archiveManagers.unsupportedMimeTypes.length > 0 || kcm.archiveManagers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.archiveManagers
                    overlay.open()
                }
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18nc("Map related application’s category’s name", "Map:")

            ComponentComboBox {
                id: mapCombo
                Layout.preferredWidth: form.longestComboBox
                component: kcm.geoUriHandlers

                KCM.SettingHighlighter {
                    highlight: !kcm.geoUriHandlers.isDefaults
                }
            }

            QQC2.Button {
                visible: !kcm.geoUriHandlers.isSaveNeeded && (kcm.geoUriHandlers.unsupportedMimeTypes.length > 0 || kcm.geoUriHandlers.mimeTypesNotAssociated.length > 0)
                icon.name: "help-contextual"
                onClicked: {
                    overlay.componentChooser = kcm.geoUriHandlers
                    overlay.open()
                }
            }
        }
    }
}
