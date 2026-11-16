/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
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

    property string originalWallpaper: ""
    property alias parentLayout: parentLayout
    property bool unsavedChanges: false

    onUnsavedChangesChanged: console.log("uCC", unsavedChanges)

    QQC2.ActionGroup {
        id: wallpaperPluginGroup
        readonly property string currentPlugin: checkedAction.pluginName
        function checkAction(plugin: string) : void {
            actions.find((action) => action.pluginName === plugin).checked = true
        }
        onCheckedActionChanged: {
            if (!originalWallpaper) {
                originalWallpaper = checkedAction.pluginName
            }
            if (configDialog.currentWallpaper === checkedAction.pluginName && main.sourceFile !== "tbd") {
                originalWallpaper = originalWallpaper || checkedAction.pluginName
                return;
            }
            configDialog.currentWallpaper = checkedAction.pluginName
            let idx = configDialog.wallpaperConfigModel.index(checkedAction.index, 0)
            main.sourceFile = idx.data(ConfigModel.SourceRole)
            appearanceRoot.checkUnsavedChanges()
        }
    }
    Connections {
        target: configDialog.wallpaperConfigModel
        function onWallpaperPluginsChanged() {
            wallpaperPluginGroup.checkAction(wallpaperPluginGroup.currentPlugin);
        }
    }
    Connections {
        target: configDialog
        function onCurrentWallpaperChanged() {
            console.log("current Wallpaper Changed", configDialog.currentWallpaper)
            wallpaperPluginGroup.checkAction(configDialog.currentWallpaper);
        }
    }

    component ModelAction: Kirigami.Action {
        id: modelAction
        property alias model: internal.model
        property alias delegate: internal.delegate
        readonly property Instantiator __instantiator: Instantiator {
            id: internal
            onObjectAdded: (index, action) => modelAction.children.push(action)
            onObjectRemoved: (index, action) => modelAction.children.splice(index, 1)
        }
    }

    actions: [
        ModelAction {
            id: wallPaperTypeAction
            text: {
                return "Type: " + wallpaperPluginGroup.checkedAction.name
            }
            model: configDialog.wallpaperConfigModel
            delegate: Kirigami.Action {
                id: actionDelegate
                required property string name
                required property string pluginName
                required property int index

                QQC2.ActionGroup.group: wallpaperPluginGroup
                text: name
                checkable: true
                checked: pluginName === configDialog.currentWallpaper
            }
        },
        NewStuff.Action {
            configFile: "wallpaperplugin.knsrc"
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button", "Get New Plugins…")
        }
    ]

    function saveConfig() {
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        }
        configDialog.currentWallpaper = wallpaperPluginGroup.currentPlugin
        appearanceRoot.originalWallpaper = wallpaperPluginGroup.currentPlugin
        configDialog.wallpaperConfiguration.keys().forEach(key => {
            if (main.currentItem["cfg_"+key] !== undefined) {
                configDialog.wallpaperConfiguration[key] = main.currentItem["cfg_"+key]
            }
        })
        configDialog.applyWallpaper()
        appearanceRoot.unsavedChanges = false
    }

    function checkUnsavedChanges() {
        const wallpaperConfig = configDialog.wallpaperConfiguration
        appearanceRoot.unsavedChanges = configDialog.currentWallpaper != appearanceRoot.originalWallpaper ||
                                        wallpaperConfig.keys().some(key => {
            const cfgKey = "cfg_" + key
            if (!(cfgKey in main.currentItem) || key.startsWith("PreviewImage") || key.endsWith("Default")) {
                return false
            }
            return main.currentItem[cfgKey] != wallpaperConfig[key] &&
                   main.currentItem[cfgKey].toString() != wallpaperConfig[key].toString()
        })
    }

    ColumnLayout {
        width: appearanceRoot.availableWidth
        height: Math.max(implicitHeight, appearanceRoot.availableHeight)
        spacing: 0 // unless it's 0 there will be an additional gap between two FormLayouts

        Kirigami.InlineMessage {
            visible: Plasmoid.immutable || animating
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info:status inlinemessage", "Layout changes have been restricted by the system administrator")
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
            visible: false
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
}
