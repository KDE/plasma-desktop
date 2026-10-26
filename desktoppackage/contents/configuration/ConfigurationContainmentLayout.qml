/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.plasma.plasmoid

KCM.AbstractKCM {
    id: layoutRoot
    signal configurationChanged

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    readonly property string desktopPlugin: "org.kde.desktopcontainment"
    readonly property string folderPlugin: "org.kde.plasma.folder"

    readonly property bool onlyDefaultPlugins: {
        const model = configDialog.containmentPluginsConfigModel
        if (model.count !== 2) {
            return false
        }
        const plugins = [model.get(0).pluginName, model.get(1).pluginName]
        return plugins.includes(desktopPlugin) && plugins.includes(folderPlugin)
    }

    property string selectedPlugin: configDialog.containmentPlugin
    readonly property bool pluginChanged: selectedPlugin !== configDialog.containmentPlugin

    readonly property string containmentPageSource: configDialog.configModel?.layoutConfigSource ?? ""
    readonly property bool containmentPageShown: onlyDefaultPlugins
        || (!pluginChanged && configDialog.containmentPlugin !== desktopPlugin)

    property bool containmentPageChanged: false
    property bool unsavedChanges: false

    function saveConfig() {
        const page = containmentPage.item
        if (page) {
            if (page.saveConfig) {
                page.saveConfig()
            }
            const config = Plasmoid.configuration
            config.keys().forEach(key => {
                const cfgKey = "cfg_" + key
                if (cfgKey in page) {
                    config[key] = page[cfgKey]
                }
            })
            config.writeConfig()
        }
        layoutRoot.containmentPageChanged = false
        configDialog.containmentPlugin = layoutRoot.selectedPlugin
        layoutRoot.checkUnsavedChanges()
    }

    function checkUnsavedChanges() {
        const page = containmentPage.item
        const config = Plasmoid.configuration
        const pageChanged = page !== null && (layoutRoot.containmentPageChanged || (page.unsavedChanges ?? false) || config.keys().some(key => {
            const cfgKey = "cfg_" + key
            if (!(cfgKey in page)) {
                return false
            }
            return page[cfgKey] != config[key] &&
                   page[cfgKey].toString() != config[key].toString()
        }))
        layoutRoot.unsavedChanges = layoutRoot.pluginChanged || pageChanged
    }

    onPluginChangedChanged: checkUnsavedChanges()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Kirigami.InlineMessage {
            visible: Plasmoid.immutable || animating
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info:status inlinemessage", "Layout changes have been restricted by the system administrator")
            showCloseButton: true
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
        }

        Kirigami.FormAlignmentGroup {
            id: formAlignmentGroup
        }

        Kirigami.Form {
            Kirigami.FormAlignmentGroup.group: formAlignmentGroup
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing

            Kirigami.FormGroup {
                Kirigami.FormEntry {
                    visible: layoutRoot.onlyDefaultPlugins
                    title: i18ndc("plasma_shell_org.kde.plasma.desktop", "@label", "Desktop:")
                    subtitle: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info", "Icons for items in the selected location appear on the desktop")
                    contentItem: QQC2.CheckBox {
                        enabled: !Plasmoid.immutable
                        text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@option:check", "Show files and folders")
                        checked: layoutRoot.selectedPlugin === layoutRoot.folderPlugin
                        onToggled: layoutRoot.selectedPlugin = checked ? layoutRoot.folderPlugin : layoutRoot.desktopPlugin
                    }
                }

                Kirigami.FormEntry {
                    visible: !layoutRoot.onlyDefaultPlugins
                    title: i18ndc("plasma_shell_org.kde.plasma.desktop", "@label:listbox", "Layout:")
                    contentItem: QQC2.ComboBox {
                        enabled: !Plasmoid.immutable
                        model: configDialog.containmentPluginsConfigModel
                        textRole: "name"
                        valueRole: "pluginName"
                        onActivated: layoutRoot.selectedPlugin = currentValue
                        Component.onCompleted: currentIndex = indexOfValue(configDialog.containmentPlugin)
                    }
                }
            }
        }

        Loader {
            id: containmentPage

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: item !== null && layoutRoot.containmentPageShown

            Component.onCompleted: {
                if (layoutRoot.containmentPageSource === "") {
                    return
                }
                const config = Plasmoid.configuration
                const props = {}
                const temp = Qt.createComponent(layoutRoot.containmentPageSource).createObject(layoutRoot)
                config.keys().forEach(key => {
                    const cfgKey = "cfg_" + key
                    if (cfgKey in temp) {
                        props[cfgKey] = config[key]
                    }
                })
                if ("formAlignmentGroup" in temp) {
                    props.formAlignmentGroup = formAlignmentGroup
                }
                temp.destroy()
                setSource(layoutRoot.containmentPageSource, props)
            }

            onLoaded: {
                const content = item.flickable?.contentItem ?? item
                content.enabled = Qt.binding(() => !Plasmoid.immutable && layoutRoot.selectedPlugin !== layoutRoot.desktopPlugin)

                Plasmoid.configuration.keys().forEach(key => {
                    const changedSignal = item["cfg_" + key + "Changed"]
                    if (changedSignal) {
                        changedSignal.connect(layoutRoot.checkUnsavedChanges)
                    }
                })
                if (item.configurationChanged) {
                    item.configurationChanged.connect(() => {
                        layoutRoot.containmentPageChanged = true
                        layoutRoot.checkUnsavedChanges()
                        layoutRoot.configurationChanged()
                    })
                }
                if (item.unsavedChangesChanged) {
                    item.unsavedChangesChanged.connect(layoutRoot.checkUnsavedChanges)
                }
                layoutRoot.checkUnsavedChanges()
            }
        }

        Item {
            id: switchContainmentWarning

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !layoutRoot.onlyDefaultPlugins && layoutRoot.pluginChanged

            Kirigami.PlaceholderMessage {
                width: parent.width - Kirigami.Units.largeSpacing * 8
                anchors.centerIn: parent

                icon.name: "documentinfo"
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info", "Layout changes must be applied before other changes can be made")

                helpfulAction: QQC2.Action {
                    icon.name: "dialog-ok-apply"
                    text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button", "Apply Now")
                    onTriggered: layoutRoot.saveConfig()
                }
            }
        }

        Item {
            visible: !containmentPage.visible && !switchContainmentWarning.visible
            Layout.fillHeight: true
        }
    }
}
