/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore
import QtQuick

import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

/**
 * Shows the widget's KPluginMetaData in a Kirigami.AboutItem
 */
KCM.SimpleKCM {
    id: page
    title: i18nc("@title:window About this widget", "About")

    property var metaData: Plasmoid.metaData

    Kirigami.AboutItem {
        id: aboutItem

        readonly property var programLogo: {
            const iconName = page.metaData.iconName;
            if (iconName.charAt(0) == "/") {
                const fileName = "plasma/plasmoids/" +
                    page.metaData.pluginId + "/contents" +
                    iconName;
                return StandardPaths.locate(StandardPaths.
                    GenericDataLocation, fileName);
            }
            return iconName || "application-x-plasma";
        }

        showLibraries: false
        aboutData: {
            "displayName": page.metaData.name,
            "productName": "",
            "componentName": page.metaData.pluginId,
            "programLogo": aboutItem.programLogo,
            "shortDescription": page.metaData.description,
            "homepage": page.metaData.website,
            "bugAddress": page.metaData.bugReportUrl,
            "version": page.metaData.version,
            "otherText": "",
            "authors": page.metaData.authors,
            "credits": page.metaData.otherContributors,
            "translators": page.metaData.translators,
            "licenses": page.metaData.license.length > 0 ? [{
                "name": page.metaData.license,
                "text": page.metaData.licenseText,
                "spdx": page.metaData.license,
            }] : [],
            "copyrightStatement": page.metaData.copyrightText,
            "desktopFileName": "",
        }
    }
}
