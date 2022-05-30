/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.configuration 2.0
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.newstuff 1.62 as NewStuff
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.4
import org.kde.plasma.plasmoid 2.0

Item {
    id: appearanceRoot
    signal configurationChanged

    property int formAlignment: wallpaperComboBox.Kirigami.ScenePosition.x - appearanceRoot.Kirigami.ScenePosition.x + (Kirigami.Units.largeSpacing/2)
    property string currentWallpaper: ""
    property string containmentPlugin: ""

    function saveConfig() {
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        }
        for (var key in configDialog.wallpaperConfiguration) {
            if (main.currentItem["cfg_"+key] !== undefined) {
                configDialog.wallpaperConfiguration[key] = main.currentItem["cfg_"+key]
            }
        }
        configDialog.currentWallpaper = appearanceRoot.currentWallpaper;
        configDialog.applyWallpaper()
        configDialog.containmentPlugin = appearanceRoot.containmentPlugin
    }

    /*
     * BUG 407619: `wallpaperGraphicsObject`, `wallpaper` or `wallpaperInterface`
     * is not set before calling `ContainmentInterface::loadWallpaper()`, so wait
     * until `wallpaperInterfaceChanged` signal is emitted. At that time
     * `applyWallpaper()` will call `syncWallpaperObjects()` to update
     * `wallpaperConfiguration`.
     */
    Connections {
        target: Plasmoid.self
        function onWallpaperInterfaceChanged() {
            configDialog.applyWallpaper();
            main.loadSourceFile();
        }
    }

    ColumnLayout {
        width: root.availableWidth
        height: Math.max(implicitHeight, root.availableHeight)
        spacing: 0 // unless it's 0 there will be an additional gap between two FormLayouts

        Component.onCompleted: {
            for (var i = 0; i < configDialog.containmentPluginsConfigModel.count; ++i) {
                var data = configDialog.containmentPluginsConfigModel.get(i);
                if (configDialog.containmentPlugin === data.pluginName) {
                    pluginComboBox.currentIndex = i
                    pluginComboBox.activated(i);
                    break;
                }
            }

            for (var i = 0; i < configDialog.wallpaperConfigModel.count; ++i) {
                var data = configDialog.wallpaperConfigModel.get(i);
                if (configDialog.currentWallpaper === data.pluginName) {
                    wallpaperComboBox.currentIndex = i
                    wallpaperComboBox.activated(i);
                    break;
                }
            }
        }

        Kirigami.InlineMessage {
            visible: plasmoid.immutable || animating
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
                enabled: !plasmoid.immutable
                model: configDialog.containmentPluginsConfigModel
                textRole: "name"
                onActivated: {
                    var model = configDialog.containmentPluginsConfigModel.get(currentIndex)
                    appearanceRoot.containmentPlugin = model.pluginName
                    appearanceRoot.configurationChanged()
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: !switchContainmentWarning.visible
                Kirigami.FormData.label: i18nd("plasma_shell_org.kde.plasma.desktop", "Wallpaper type:")
                QQC2.ComboBox {
                    id: wallpaperComboBox
                    Layout.preferredWidth: Math.max(implicitWidth, pluginComboBox.implicitWidth)
                    model: configDialog.wallpaperConfigModel
                    textRole: "name"
                    onActivated: {
                        var model = configDialog.wallpaperConfigModel.get(currentIndex)
                        appearanceRoot.currentWallpaper = model.pluginName
                        configDialog.currentWallpaper = model.pluginName
                        main.sourceFile = model.source
                        appearanceRoot.configurationChanged()
                    }
                }
                NewStuff.Button {
                    configFile: "wallpaperplugin.knsrc"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Get New Plugins…")
                    Layout.preferredHeight: wallpaperComboBox.height
                }
            }
        }

        ColumnLayout {
            id: switchContainmentWarning
            Layout.fillWidth: true
            visible: configDialog.containmentPlugin !== appearanceRoot.containmentPlugin
            QQC2.Label {
                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Layout changes must be applied before other changes can be made")
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply now")
                onClicked: saveConfig()
            }

            Binding {
                target: categoriesScroll //from parent scope AppletConfiguration
                property: "enabled"
                value: !switchContainmentWarning.visible
                restoreMode: Binding.RestoreBinding
            }
        }

        Item {
            Layout.fillHeight: true
            visible: switchContainmentWarning.visible
        }

        Item {
            id: emptyConfig
        }

        QQC2.StackView {
            id: main

            implicitHeight: main.empty ? 0 : currentItem.implicitHeight

            Layout.fillHeight: true;
            Layout.fillWidth: true;

            visible: !switchContainmentWarning.visible

            // Bug 360862: if wallpaper has no config, sourceFile will be ""
            // so we wouldn't load emptyConfig and break all over the place
            // hence set it to some random value initially
            property string sourceFile: "tbd"

            onSourceFileChanged: loadSourceFile()

            function loadSourceFile() {
                const wallpaperConfig = configDialog.wallpaperConfiguration
                // BUG 407619: wallpaperConfig can be null before calling `ContainmentInterface::loadWallpaper()`
                if (wallpaperConfig && sourceFile) {
                    var props = {}

                    for (var key in wallpaperConfig) {
                        props["cfg_" + key] = wallpaperConfig[key]
                    }

                    var newItem = replace(Qt.resolvedUrl(sourceFile), props)

                    for (var key in wallpaperConfig) {
                        var changedSignal = newItem["cfg_" + key + "Changed"]
                        if (changedSignal) {
                            changedSignal.connect(appearanceRoot.configurationChanged)
                        }
                    }

                    const configurationChangedSignal = newItem.configurationChanged
                    if (configurationChangedSignal) {
                        configurationChangedSignal.connect(appearanceRoot.configurationChanged)
                    }
                } else {
                    replace(emptyConfig)
                }
            }
        }
    }
}
