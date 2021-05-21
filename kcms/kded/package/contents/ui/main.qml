/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import QtQml 2.15

import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.5 as KCM
import org.kde.private.kcms.style 1.0 as Private

KCM.ScrollViewKCM {
    id: root

    KCM.ConfigModule.quickHelp: i18n("<p>This module allows you to have an overview of all plugins of the KDE Daemon, also referred to as KDE Services. Generally, there are two types of service:</p> <ul><li>Services invoked at startup</li><li>Services called on demand</li></ul> <p>The latter are only listed for convenience. The startup services can be started and stopped. You can also define whether services should be loaded at startup.</p> <p><b>Use this with care: some services are vital for Plasma; do not deactivate services if you  do not know what you are doing.</b></p>")

    Binding {
        target: kcm.filteredModel
        property: "query"
        value: searchField.text
        restoreMode: Binding.RestoreBinding
    }

    Binding {
        target: kcm.filteredModel
        property: "statusFilter"
        value: filterCombo.model[filterCombo.currentIndex].statusFilter
        restoreMode: Binding.RestoreBinding
    }

    header: ColumnLayout {
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            text: i18n("The background services manager (kded5) is currently not running. Make sure it is installed correctly.");
            type: Kirigami.MessageType.Error
            showCloseButton: false
            visible: !kcm.kdedRunning
        }

        Kirigami.InlineMessage {
            id: selfDisablingModulesHint
            Layout.fillWidth: true
            text: i18n("Some services disable themselves again when manually started if they are not useful in the current environment.")
            type: Kirigami.MessageType.Information
            showCloseButton: true
            visible: false
        }

        Kirigami.InlineMessage {
            id: runningModulesChangedAfterSaveHint
            Layout.fillWidth: true
            text: i18n("Some services were automatically started/stopped when the background services manager (kded5) was restarted to apply your changes.")
            type: Kirigami.MessageType.Information
            showCloseButton: true
            visible: false
        }

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true

            type: Kirigami.MessageType.Error
            showCloseButton: true
            visible: false

            Connections {
                target: kcm
                function onErrorMessage(errorString) {
                    errorMessage.text = errorString;
                    errorMessage.visible = true;
                }
                function onShowSelfDisablingModulesHint() {
                    selfDisablingModulesHint.visible = true;
                }
                function onShowRunningModulesChangedAfterSaveHint() {
                    runningModulesChangedAfterSaveHint.visible = true;
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Kirigami.SearchField {
                id: searchField
                Layout.fillWidth: true
            }

            QtControls.ComboBox {
                id: filterCombo
                textRole: "text"
                enabled: kcm.kdedRunning || currentIndex > 0
                model: [
                    {text: i18n("All Services"), statusFilter: Private.KCM.UnknownStatus},
                    {text: i18nc("List running services", "Running"), statusFilter: Private.KCM.Running},
                    {text: i18nc("List not running services", "Not Running"), statusFilter: Private.KCM.NotRunning}
                ]

                // HACK QQC2 doesn't support icons, so we just tamper with the desktop style ComboBox's background
                // and inject a nice little filter icon.
                Component.onCompleted: {
                    if (!background || !background.hasOwnProperty("properties")) {
                        // not a KQuickStyleItem
                        return;
                    }

                    var props = background.properties || {};

                    background.properties = Qt.binding(function() {
                        var newProps = props;
                        newProps.currentIcon = "view-filter";
                        newProps.iconColor = Kirigami.Theme.textColor;
                        return newProps;
                    });
                }
            }
        }
    }

    view: ListView {
        id: list
        clip: true
        activeFocusOnTab: true

        model: kcm.filteredModel

        section.property: "type"
        section.delegate: Kirigami.ListSectionHeader {
            width: list.width
            label: {
                switch (Number(section)) {
                    case Private.KCM.AutostartType: return i18n("Startup Services");
                    case Private.KCM.OnDemandType: return i18n("Load-on-Demand Services");
                }
            }
        }

        Component {
            id: listDelegateComponent

            Kirigami.AbstractListItem {
                id: delegate
                // FIXME why does the padding logic to dodge the ScrollBars not work here?
                text: model.display
                enabled: !model.immutable
                checkable: model.type !== Private.KCM.OnDemandType
                checked: model.autoloadEnabled === true
                hoverEnabled: checkable
                focusPolicy: Qt.ClickFocus
                Accessible.description: i18n("Toggle automatically loading this service on startup")
                onClicked: {
                    if (checkable) {
                        model.autoloadEnabled = !model.autoloadEnabled;
                    }
                }

                Component.onCompleted: {
                    // Checkable Kirigami.ListItem has blue background which we don't want
                    // as we have a dedicated CheckBox. Still using those properties for accessibility.
                    background.color = Qt.binding(function() {
                        return delegate.highlighted || (delegate.supportsMouseEvents && delegate.pressed)
                            ? delegate.activeBackgroundColor
                            : delegate.backgroundColor
                    });
                }

                contentItem: RowLayout {
                    QtControls.CheckBox {
                        id: autoloadCheck
                        // Keep focus on the delegate
                        focusPolicy: Qt.NoFocus
                        checked: delegate.checked
                        visible: delegate.checkable
                        onToggled: model.autoloadEnabled = !model.autoloadEnabled

                        QtControls.ToolTip {
                            text: delegate.Accessible.description
                        }

                        KCM.SettingHighlighter {
                            highlight: !model.autoloadEnabled
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        QtControls.Label {
                            id: displayLabel
                            Layout.fillWidth: true
                            text: delegate.text
                            elide: Text.ElideRight
                            textFormat: Text.PlainText
                            color: (delegate.highlighted || (delegate.pressed && delegate.supportsMouseEvents)) ? delegate.activeTextColor : delegate.textColor
                        }

                        QtControls.Label {
                            Layout.fillWidth: true
                            text: model.description
                            // FIXME do we have a descriptive label component?
                            opacity: delegate.hovered ? 0.8 : 0.6
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                            color: displayLabel.color
                        }
                    }

                    QtControls.Label {
                        id: statusLabel
                        horizontalAlignment: Text.AlignRight
                        opacity: model.status === Private.KCM.Running ? 1 : delegate.hovered ? 0.8 : 0.6
                        color: model.status === Private.KCM.Running ? Kirigami.Theme.positiveTextColor : displayLabel.color
                        visible: kcm.kdedRunning && model.type !== Private.KCM.OnDemandType
                        text: {
                            switch (model.status) {
                            case Private.KCM.NotRunning: return i18n("Not running");
                            case Private.KCM.Running: return i18n("Running");
                            }
                            return "";
                        }
                    }

                    QtControls.Button {
                        icon.name: model.status === Private.KCM.Running ? "media-playback-pause" : "media-playback-start"
                        visible: kcm.kdedRunning && model.status !== Private.KCM.UnknownStatus && model.type !== Private.KCM.OnDemandType
                        onClicked: {
                            errorMessage.visible = false;

                            console.log("DELEGATE", delegate)
                            if (model.status === Private.KCM.Running) {
                                kcm.stopModule(model.moduleName);
                            } else {
                                kcm.startModule(model.moduleName);
                            }
                        }
                        Accessible.name: model.status === Private.KCM.Running ? i18n("Stop Service") : i18n("Start Service")

                        QtControls.ToolTip {
                            text: parent.Accessible.name
                        }
                    }
                }
            }
        }

        delegate: Kirigami.DelegateRecycler {
            width: list.width
            sourceComponent: listDelegateComponent
        }
    }
}
