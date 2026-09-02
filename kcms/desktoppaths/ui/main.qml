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

    Kirigami.SizeGroup {
        mode: Kirigami.SizeGroup.Width
        items: [
            desktop.contentItem,
            documents.contentItem,
            downloads.contentItem,
            videos.contentItem,
            pictures.contentItem,
            music.contentItem,
            publicPath.contentItem,
            templates.contentItem,
            projects.contentItem
        ]
    }

    Kirigami.Form {
        Kirigami.FormGroup {
            UrlRequester {
                id: desktop

                title: i18nc("@label:textbox", "Desktop folder:")

                location: kcm.settings.desktopLocation
                defaultLocation: kcm.settings.defaultDesktopLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder contains all the files you see on your desktop.")

                onNewLocationSelected: (newLocation) => kcm.settings.desktopLocation = newLocation
            }

            UrlRequester {
                id: documents

                title: i18nc("@label:textbox", "Documents folder:")

                location: kcm.settings.documentsLocation
                defaultLocation: kcm.settings.defaultDocumentsLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save documents.")

                onNewLocationSelected: (newLocation) => kcm.settings.documentsLocation = newLocation
            }

            UrlRequester {
                id: downloads

                title: i18nc("@label:textbox", "Downloads folder:")

                location: kcm.settings.downloadsLocation
                defaultLocation: kcm.settings.defaultDownloadsLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to save your downloaded items.")

                onNewLocationSelected: (newLocation) => kcm.settings.downloadsLocation = newLocation
            }

            UrlRequester {
                id: videos

                title: i18nc("@label:textbox", "Videos folder:")

                location: kcm.settings.videosLocation
                defaultLocation: kcm.settings.defaultVideosLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save video files.")

                onNewLocationSelected: (newLocation) => kcm.settings.videosLocation = newLocation
            }

            UrlRequester {
                id: pictures

                title: i18nc("@label:textbox", "Pictures folder:")

                location: kcm.settings.picturesLocation
                defaultLocation: kcm.settings.defaultPicturesLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save image files.")

                onNewLocationSelected: (newLocation) => kcm.settings.picturesLocation = newLocation
            }

            UrlRequester {
                id: music

                title: i18nc("@label:textbox", "Music folder:")

                location: kcm.settings.musicLocation
                defaultLocation: kcm.settings.defaultMusicLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save music files.")

                onNewLocationSelected: (newLocation) => kcm.settings.musicLocation = newLocation
            }

            UrlRequester {
                id: publicPath

                title: i18nc("@label:textbox", "Public folder:")

                location: kcm.settings.publicLocation
                defaultLocation: kcm.settings.defaultPublicLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default for publicly-shared files when network sharing is enabled.")

                onNewLocationSelected: (newLocation) => kcm.settings.publicLocation = newLocation
            }

            UrlRequester {
                id: templates

                title: i18nc("@label:textbox", "Templates folder:")

                location: kcm.settings.templatesLocation
                defaultLocation: kcm.settings.defaultTemplatesLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save file templates.")

                onNewLocationSelected: (newLocation) => kcm.settings.templatesLocation = newLocation
            }

            UrlRequester {
                id: projects

                title: i18nc("@label:textbox", "Projects folder:")

                location: kcm.settings.projectsLocation
                defaultLocation: kcm.settings.defaultProjectsLocation
                Accessible.description: i18nc("@info:tooltip and accessible description", "This folder will be used by default to open or save projects.")

                onNewLocationSelected: (newLocation) => kcm.settings.projectsLocation = newLocation
            }
        }
    }
}
