/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.buttons: KCM.ConfigModule.Default | KCM.ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: Kirigami.Units.gridUnit * 20

                                                                      // FIXME: get actual scrollbar width,
                                                                      // don't assume gridUnit is right
    readonly property int availableSpace: root.width - (__flickableOverflows ? Kirigami.Units.gridUnit : 0)
                                                     - buttonMetrics.implicitWidth
                                                     - desktop.spacing
                                                     - leftPadding
                                                     - rightPadding

    readonly property int commonFieldWidth: Math.min(availableSpace,
                                                     Math.max(desktop.implicitTextFieldWidth,
                                                              documents.implicitTextFieldWidth,
                                                              downloads.implicitTextFieldWidth,
                                                              videos.implicitTextFieldWidth,
                                                              pictures.implicitTextFieldWidth,
                                                              music.implicitTextFieldWidth,
                                                              publicPath.implicitTextFieldWidth,
                                                              templates.implicitTextFieldWidth))

    // Need to get the width of a standard button since UrlRequester includes one,
    // so we can subtract it from the available width for the text field.Otherwise
    // the layout overflows in FormLayout's narrow mode
    QQC2.Button {
        id: buttonMetrics
        visible: false
        icon.name: "document-open"
    }

    Kirigami.FormLayout {
        UrlRequester {
            id: desktop

            Kirigami.FormData.label: i18nc("@label:textbox", "Desktop folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.desktopLocation
            defaultLocation: kcm.settings.defaultDesktopLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder contains all the files you see on your desktop.")

            onNewLocationSelected: (newLocation) => kcm.settings.desktopLocation = newLocation
        }

        UrlRequester {
            id: documents

            Kirigami.FormData.label: i18nc("@label:textbox", "Documents folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.documentsLocation
            defaultLocation: kcm.settings.defaultDocumentsLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save documents.")

            onNewLocationSelected: (newLocation) => kcm.settings.documentsLocation = newLocation
        }

        UrlRequester {
            id: downloads

            Kirigami.FormData.label: i18nc("@label:textbox", "Downloads folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.downloadsLocation
            defaultLocation: kcm.settings.defaultDownloadsLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to save your downloaded items.")

            onNewLocationSelected: (newLocation) => kcm.settings.downloadsLocation = newLocation
        }

        UrlRequester {
            id: videos

            Kirigami.FormData.label: i18nc("@label:textbox", "Videos folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.videosLocation
            defaultLocation: kcm.settings.defaultVideosLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save video files.")

            onNewLocationSelected: (newLocation) => kcm.settings.videosLocation = newLocation
        }

        UrlRequester {
            id: pictures

            Kirigami.FormData.label: i18nc("@label:textbox", "Pictures folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.picturesLocation
            defaultLocation: kcm.settings.defaultPicturesLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save image files.")

            onNewLocationSelected: (newLocation) => kcm.settings.picturesLocation = newLocation
        }

        UrlRequester {
            id: music

            Kirigami.FormData.label: i18nc("@label:textbox", "Music folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.musicLocation
            defaultLocation: kcm.settings.defaultMusicLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save music files.")

            onNewLocationSelected: (newLocation) => kcm.settings.musicLocation = newLocation
        }

        UrlRequester {
            id: publicPath

            Kirigami.FormData.label: i18nc("@label:textbox", "Public folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.publicLocation
            defaultLocation: kcm.settings.defaultPublicLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default for publicly-shared files when network sharing is enabled.")

            onNewLocationSelected: (newLocation) => kcm.settings.publicLocation = newLocation
        }

        UrlRequester {
            id: templates

            Kirigami.FormData.label: i18nc("@label:textbox", "Templates folder:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.templatesLocation
            defaultLocation: kcm.settings.defaultTemplatesLocation
            Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save file templates.")

            onNewLocationSelected: (newLocation) => kcm.settings.templatesLocation = newLocation
        }
    }
}
