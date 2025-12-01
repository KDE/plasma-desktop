/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.configuration
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml

import org.kde.newstuff as NewStuff
import org.kde.kirigami as Kirigami
import org.kde.kcmutils
import org.kde.plasma.plasmoid
import org.kde.plasma.configuration

SimpleKCM {
    id: appearanceRoot
    signal configurationChanged

    leftPadding: 0 // let wallpaper config touch the edges in case there's a List/GridView'
    rightPadding: 0
    bottomPadding: 0

    property int formAlignment: wallpaperComboBox.Kirigami.ScenePosition.x - appearanceRoot.Kirigami.ScenePosition.x + Kirigami.Units.smallSpacing
    property string originalWallpaper: ""
    property alias parentLayout: parentLayout
    property bool unsavedChanges: false

    function saveConfig() {
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        }
        configDialog.currentWallpaper = wallpaperComboBox.currentValue
        appearanceRoot.originalWallpaper = wallpaperComboBox.currentValue
        configDialog.wallpaperConfiguration.keys().forEach(key => {
            if (main.currentItem["cfg_"+key] !== undefined) {
                configDialog.wallpaperConfiguration[key] = main.currentItem["cfg_"+key]
            }
        })
        configDialog.applyWallpaper()
        configDialog.containmentPlugin = pluginComboBox.currentValue
        appearanceRoot.closeContainmentWarning()
        appearanceRoot.unsavedChanges = false
    }

    function checkUnsavedChanges() {
        const wallpaperConfig = configDialog.wallpaperConfiguration
        appearanceRoot.unsavedChanges = configDialog.currentWallpaper != appearanceRoot.originalWallpaper ||
                                        configDialog.containmentPlugin != pluginComboBox.currentValue ||
                                        wallpaperConfig.keys().some(key => {
            const cfgKey = "cfg_" + key
            if (!(cfgKey in main.currentItem) || key.startsWith("PreviewImage") || key.endsWith("Default")) {
                return false
            }
            return main.currentItem[cfgKey] != wallpaperConfig[key] &&
                   main.currentItem[cfgKey].toString() != wallpaperConfig[key].toString()
        })
    }

    function closeContainmentWarning() {
        if (main.currentItem?.objectName === "switchContainmentWarningItem") {
            main.pop();
            categoriesScroll.enabled = true;
        }
    }

    ColumnLayout {
        width: appearanceRoot.availableWidth
        height: Math.max(implicitHeight, appearanceRoot.availableHeight)
        spacing: 0 // unless it's 0 there will be an additional gap between two FormLayouts

        Kirigami.InlineMessage {
            visible: Plasmoid.immutable || animating
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Layout changes have been restricted by the system administrator")
            showCloseButton: true
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing * 2 // we need this because ColumnLayout's spacing is 0
        }

        Kirigami.FormLayout {
            id: parentLayout // needed for twinFormLayouts to work in wallpaper plugins
            twinFormLayouts: main.currentItem.formLayout || []
            Layout.fillWidth: true
            QQC2.ComboBox {
                id: pluginComboBox
                Layout.preferredWidth: Math.max(implicitWidth, wallpaperComboBox.implicitWidth)
                Kirigami.FormData.label: i18nd("plasma_shell_org.kde.plasma.desktop", "Layout:")
                enabled: !Plasmoid.immutable
                model: configDialog.containmentPluginsConfigModel
                textRole: "name"
                valueRole: "pluginName"
                onActivated: {
                    if (configDialog.containmentPlugin !== pluginComboBox.currentValue) {
                        main.push(switchContainmentWarning);
                        categoriesScroll.enabled = false;
                    } else {
                        closeContainmentWarning()
                    }
                    appearanceRoot.checkUnsavedChanges()
                }
                Component.onCompleted: {
                    currentIndex = indexOfValue(configDialog.containmentPlugin)
                    activated(currentIndex)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                enabled: main.currentItem.objectName !== "switchContainmentWarningItem"
                Kirigami.FormData.label: i18nd("plasma_shell_org.kde.plasma.desktop", "Wallpaper type:")
                Kirigami.FormData.buddyFor: wallpaperComboBox

                QQC2.ComboBox {
                    id: wallpaperComboBox

                    function selectCurrentWallpaperPlugin() {
                        currentIndex = indexOfValue(configDialog.currentWallpaper)
                        appearanceRoot.originalWallpaper = currentValue
                        activated(currentIndex)
                    }

                    Layout.preferredWidth: Math.max(implicitWidth, pluginComboBox.implicitWidth)
                    model: configDialog.wallpaperConfigModel
                    textRole: "name"
                    valueRole: "pluginName"
                    onActivated: {
                        var idx = configDialog.wallpaperConfigModel.index(currentIndex, 0)
                        if (configDialog.currentWallpaper === currentValue && main.sourceFile !== "tbd") {
                            return;
                        }
                        configDialog.currentWallpaper = currentValue
                        main.sourceFile = idx.data(ConfigModel.SourceRole)
                        appearanceRoot.checkUnsavedChanges()
                    }
                    Component.onCompleted: {
                        selectCurrentWallpaperPlugin();
                    }

                    Connections {
                        enabled: true
                        target: configDialog.wallpaperConfigModel
                        function onWallpaperPluginsChanged() {
                            wallpaperComboBox.selectCurrentWallpaperPlugin();
                        }
                    }
                }
                NewStuff.Button {
                    configFile: "wallpaperplugin.knsrc"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Get New Plugins…")
                    visibleWhenDisabled: true // don't hide on disabled
                    Layout.preferredHeight: wallpaperComboBox.height
                }
            }
        }

        Item {
            id: emptyConfig
        }

        QQC2.StackView {
            id: main

            implicitHeight: main.empty ? 0 : currentItem.implicitHeight

            Layout.fillHeight: true;
            Layout.fillWidth: true;

            // Bug 360862: if wallpaper has no config, sourceFile will be ""
            // so we wouldn't load emptyConfig and break all over the place
            // hence set it to some random value initially
            property string sourceFile: "tbd"

            onSourceFileChanged: loadSourceFile()

            function loadSourceFile() {
                const wallpaperConfig = configDialog.wallpaperConfiguration
                // BUG 407619: wallpaperConfig can be null before calling `ContainmentItem::loadWallpaper()`
                if (wallpaperConfig && sourceFile) {
                    var props = {
                        "configDialog": configDialog,
                        "wallpaperConfiguration": Qt.binding(() => Plasmoid.wallpaperGraphicsObject.configuration)
                    }

                    // Some third-party wallpaper plugins need the config keys to be set initially.
                    // We should not break them within one Plasma major version, but setting everything
                    // will lead to an error message for every unused property (and some, like KConfigXT
                    // default values, are used by almost no plugin configuration). We load the config
                    // page in a temp variable first, then use that to figure out which ones we need to
                    // set initially.
                    // TODO Plasma 7: consider whether we can drop this workaround
                    const temp = Qt.createComponent(Qt.resolvedUrl(sourceFile)).createObject(appearanceRoot, props)
                    wallpaperConfig.keys().forEach(key => {
                        const cfgKey = "cfg_" + key;
                        if (cfgKey in temp) {
                            props[cfgKey] = wallpaperConfig[key]
                        }
                    })
                    temp.destroy()

                    var newItem = replace(Qt.resolvedUrl(sourceFile), props)

                    wallpaperConfig.keys().forEach(key => {
                        const cfgKey = "cfg_" + key;
                        if (cfgKey in newItem) {
                            let changedSignal = main.currentItem[cfgKey + "Changed"]
                            if (changedSignal) {
                                changedSignal.connect(appearanceRoot.checkUnsavedChanges)
                            }
                        }
                    });

                    const configurationChangedSignal = newItem.configurationChanged
                    if (configurationChangedSignal) {
                        configurationChangedSignal.connect(appearanceRoot.configurationChanged)
                    }
                } else {
                    replace(emptyConfig)
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }
    }

    Component {
        id: switchContainmentWarning

        Item {
            objectName: "switchContainmentWarningItem"

            Kirigami.PlaceholderMessage {
                id: message
                width: parent.width - Kirigami.Units.largeSpacing * 8
                anchors.centerIn: parent

                icon.name: "documentinfo"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Layout changes must be applied before other changes can be made")

                helpfulAction: QQC2.Action {
                    icon.name: "dialog-ok-apply"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply Now")
                    onTriggered: appearanceRoot.saveConfig()
                }
            }
        }
    }
}
