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

SimpleKCM {
    id: root
    signal settingValueChanged

    property int formAlignment: wallpaperComboBox.Kirigami.ScenePosition.x - root.Kirigami.ScenePosition.x + (Kirigami.Units.largeSpacing/2)
    property string currentWallpaper: ""
    property string containmentPlugin: ""

    title: i18n("Appearance")

    function saveConfig() {
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        }
        for (var key in configDialog.wallpaperConfiguration) {
            if (main.currentItem["cfg_"+key] !== undefined) {
                configDialog.wallpaperConfiguration[key] = main.currentItem["cfg_"+key]
            }
        }
        configDialog.currentWallpaper = root.currentWallpaper;
        configDialog.applyWallpaper()
        configDialog.containmentPlugin = root.containmentPlugin
    }

    Item {
        id: parentItem
        width: root.width - root.leftPadding - root.rightPadding // Center FormLayouts

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
            id: inlineMessage
            visible: plasmoid.immutable || animating
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Layout changes have been restricted by the system administrator")
            showCloseButton: true
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                leftMargin: Kirigami.Units.smallSpacing
                rightMargin: Kirigami.Units.smallSpacing
                bottomMargin: Kirigami.Units.smallSpacing * 2
            }
        }

        Kirigami.FormLayout {
            id: parentLayout // needed for twinFormLayouts to work in wallpaper plugins
            twinFormLayouts: main.currentItem.formLayout || []
            anchors {
                top: inlineMessage.bottom
                left: parent.left
                right: parent.right
            }
            QQC2.ComboBox {
                id: pluginComboBox
                Layout.preferredWidth: Math.max(implicitWidth, wallpaperComboBox.implicitWidth)
                Kirigami.FormData.label: i18nd("plasma_shell_org.kde.plasma.desktop", "Layout:")
                enabled: !plasmoid.immutable
                model: configDialog.containmentPluginsConfigModel
                textRole: "name"
                onActivated: {
                    var model = configDialog.containmentPluginsConfigModel.get(currentIndex)
                    root.containmentPlugin = model.pluginName
                    root.settingValueChanged()
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
                        root.currentWallpaper = model.pluginName
                        configDialog.currentWallpaper = model.pluginName
                        main.sourceFile = model.source
                        root.settingValueChanged()
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
            anchors {
                top: parentLayout.bottom
                left: parent.left
                right: parent.right
            }
            height: visible ? implicitHeight : 0
            visible: configDialog.containmentPlugin !== root.containmentPlugin
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
            id: emptyConfig
        }

        QQC2.StackView {
            id: main

            anchors {
                top: switchContainmentWarning.bottom
                left: parent.left
                right: parent.right
            }

            visible: !switchContainmentWarning.visible

            // Bug 360862: if wallpaper has no config, sourceFile will be ""
            // so we wouldn't load emptyConfig and break all over the place
            // hence set it to some random value initially
            property string sourceFile: "tbd"
            onSourceFileChanged: {
                if (sourceFile) {
                    var props = {}

                    var wallpaperConfig = configDialog.wallpaperConfiguration
                    for (var key in wallpaperConfig) {
                        props["cfg_" + key] = wallpaperConfig[key]
                    }

                    var newItem = replace(Qt.resolvedUrl(sourceFile), props)

                    for (var key in wallpaperConfig) {
                        var changedSignal = newItem["cfg_" + key + "Changed"]
                        if (changedSignal) {
                            changedSignal.connect(root.settingValueChanged)
                        }
                    }
                } else {
                    replace(emptyConfig)
                }
            }
        }
    }


    states: [
        State {
            when: configDialog.currentWallpaper === "org.kde.image" || configDialog.currentWallpaper === "org.kde.slideshow"
            PropertyChanges {
                target: parentItem
                // The height of KCM.GridView depends on the height of the window.
                height: root.height - root.topPadding - root.bottomPadding
            }
            AnchorChanges {
                target: main
                anchors.bottom: parentItem.bottom
            }
        },
        State {
            when: !(configDialog.currentWallpaper === "org.kde.image" || configDialog.currentWallpaper === "org.kde.slideshow")
            PropertyChanges {
                target: main
                // Need to explicitly set the height of StackView.
                // See https://doc.qt.io/qt-5/qml-qtquick-controls2-stackview.html#size
                height: main.currentItem.implicitHeight
            }
            PropertyChanges {
                target: parentItem
                // HACK: Eliminate binding loop warnings on height
                height: inlineMessage.height
                      + parentLayout.implicitHeight
                      + switchContainmentWarning.height
                      + main.height
            }
        }
    ]
}
