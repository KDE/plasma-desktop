/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils
import org.kde.kcmutils 1.0 as KCMUtils

SimpleKCM {
    id: root

    KCMUtils.ConfigModule.buttons: KCMUtils.ConfigModule.Default | KCMUtils.ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: Kirigami.Units.gridUnit * 20

    Kirigami.FormLayout {
        UrlRequester {
            Kirigami.FormData.label: i18n("Desktop path:")
            location: kcm.settings.desktopLocation
            defaultLocation: kcm.settings.defaultDesktopLocation
            Accessible.description: i18n("This folder contains all the files which you see on your desktop. You can change the location of this folder if you want to, and the contents will move automatically to the new location as well.")

            onNewLocationSelected: (newLocation) => kcm.settings.desktopLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Documents path:")
            location: kcm.settings.documentsLocation
            defaultLocation: kcm.settings.defaultDocumentsLocation
            Accessible.description: i18n("This folder will be used by default to load or save documents from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.documentsLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Downloads path:")
            location: kcm.settings.downloadsLocation
            defaultLocation: kcm.settings.defaultDownloadsLocation
            Accessible.description: i18n("This folder will be used by default to save your downloaded items.")

            onNewLocationSelected: (newLocation) => kcm.settings.downloadsLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Videos path:")
            location: kcm.settings.videosLocation
            defaultLocation: kcm.settings.defaultVideosLocation
            Accessible.description: i18n("This folder will be used by default to load or save movies from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.videosLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Pictures path:")
            location: kcm.settings.picturesLocation
            defaultLocation: kcm.settings.defaultPicturesLocation
            Accessible.description: i18n("This folder will be used by default to load or save pictures from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.picturesLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Music path:")
            location: kcm.settings.musicLocation
            defaultLocation: kcm.settings.defaultMusicLocation
            Accessible.description: i18n("This folder will be used by default to load or save music from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.musicLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Public path:")
            location: kcm.settings.publicLocation
            defaultLocation: kcm.settings.defaultPublicLocation
            Accessible.description: i18n("This folder will be used by default to load or save movies from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.publicLocation = newLocation
        }

        UrlRequester {
            Kirigami.FormData.label: i18n("Templates path:")
            location: kcm.settings.templatesLocation
            defaultLocation: kcm.settings.defaultTemplatesLocation
            Accessible.description: i18n("This folder will be used by default to load or save templates from or to.")

            onNewLocationSelected: (newLocation) => kcm.settings.templatesLocation = newLocation
        }
    }
}
