/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.buttons: KCM.ConfigModule.Default | KCM.ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: Kirigami.Units.gridUnit * 20

    property int commonFieldWidth: Math.min(Math.round(root.width/2),
                                            Math.max(desktop.implicitTextFieldWidth,
                                                     documents.implicitTextFieldWidth,
                                                     downloads.implicitTextFieldWidth,
                                                     videos.implicitTextFieldWidth,
                                                     pictures.implicitTextFieldWidth,
                                                     music.implicitTextFieldWidth,
                                                     publicPath.implicitTextFieldWidth,
                                                     templates.implicitTextFieldWidth))

    Kirigami.FormLayout {
        UrlRequester {
            id: desktop

            Kirigami.FormData.label: i18n("Desktop path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.desktopLocation
            defaultLocation: kcm.settings.defaultDesktopLocation
            Accessible.description: i18n("This folder contains all the files which you see on your desktop. You can change the location of this folder if you want to, and the contents will move automatically to the new location as well.")

            onNewLocationSelected: (newLocation) => kcm.settings.desktopLocation = newLocation
        }

        UrlRequester {
            id: documents

            Kirigami.FormData.label: i18n("Documents path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.documentsLocation
            defaultLocation: kcm.settings.defaultDocumentsLocation
            Accessible.description: i18n("This folder will be used by default to load or save documents from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.documentsLocation = newLocation
        }

        UrlRequester {
            id: downloads

            Kirigami.FormData.label: i18n("Downloads path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.downloadsLocation
            defaultLocation: kcm.settings.defaultDownloadsLocation
            Accessible.description: i18n("This folder will be used by default to save your downloaded items.")

            onNewLocationSelected: (newLocation) => kcm.settings.downloadsLocation = newLocation
        }

        UrlRequester {
            id: videos

            Kirigami.FormData.label: i18n("Videos path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.videosLocation
            defaultLocation: kcm.settings.defaultVideosLocation
            Accessible.description: i18n("This folder will be used by default to load or save movies from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.videosLocation = newLocation
        }

        UrlRequester {
            id: pictures

            Kirigami.FormData.label: i18n("Pictures path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.picturesLocation
            defaultLocation: kcm.settings.defaultPicturesLocation
            Accessible.description: i18n("This folder will be used by default to load or save pictures from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.picturesLocation = newLocation
        }

        UrlRequester {
            id: music

            Kirigami.FormData.label: i18n("Music path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.musicLocation
            defaultLocation: kcm.settings.defaultMusicLocation
            Accessible.description: i18n("This folder will be used by default to load or save music from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.musicLocation = newLocation
        }

        UrlRequester {
            id: publicPath

            Kirigami.FormData.label: i18n("Public path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.publicLocation
            defaultLocation: kcm.settings.defaultPublicLocation
            Accessible.description: i18n("This folder will be used by default to load or save movies from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.publicLocation = newLocation
        }

        UrlRequester {
            id: templates

            Kirigami.FormData.label: i18n("Templates path:")

            textFieldWidth: root.commonFieldWidth

            location: kcm.settings.templatesLocation
            defaultLocation: kcm.settings.defaultTemplatesLocation
            Accessible.description: i18n("This folder will be used by default to load or save templates from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.templatesLocation = newLocation
        }
    }
}
