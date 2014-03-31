/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0
import org.kde.plasma.configuration 2.0


//TODO: all of this will be done with desktop components
Rectangle {
    id: root

//BEGIN properties
    color: syspal.window
    width: units.gridUnit * 40
    height: units.gridUnit * 30
//END properties

//BEGIN model
    property ConfigModel globalConfigModel:  globalAppletConfigModel

    ConfigModel {
        id: globalAppletConfigModel
        ConfigCategory {
            name: "Keyboard shortcuts"
            icon: "preferences-desktop-keyboard"
            source: "ConfigurationShortcuts.qml"
        }
    }
//END model

//BEGIN functions
    function saveConfig() {
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        } else {
            for (var key in plasmoid.configuration) {
                if (main.currentItem["cfg_"+key] !== undefined) {
                    plasmoid.configuration[key] = main.currentItem["cfg_"+key]
                }
            }
        }
    }

    function restoreConfig() {
        for (var key in plasmoid.configuration) {
            if (main.currentItem["cfg_"+key] !== undefined) {
                main.currentItem["cfg_"+key] = plasmoid.configuration[key]
            }
        }
    }
//END functions


//BEGIN connections
    Component.onCompleted: {
        if (configDialog.configModel && configDialog.configModel.count > 0) {
            main.sourceFile = configDialog.configModel.get(0).source
            main.title = configDialog.configModel.get(0).name
        } else {
            main.sourceFile = globalConfigModel.get(0).source
            main.title = globalConfigModel.get(0).name
        }
        root.restoreConfig()
//         root.width = mainColumn.implicitWidth
//         root.height = mainColumn.implicitHeight
    }
//END connections

//BEGIN UI components
    SystemPalette {id: syspal}

    ColumnLayout {
        id: mainColumn
        anchors {
            fill: parent
            margins: mainColumn.spacing //margins are hardcoded in QStyle we should match that here
        }
        property int implicitWidth: Math.max(contentRow.implicitWidth, buttonsRow.implicitWidth) + 8
        property int implicitHeight: contentRow.implicitHeight + buttonsRow.implicitHeight + 8

        RowLayout {
            id: contentRow
            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: units.largeSpacing
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height - buttonsRow.height

            QtControls.ScrollView {
                id: categoriesScroll
                frameVisible: true
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }
                visible: (configDialog.configModel ? configDialog.configModel.count : 0) + globalConfigModel.count > 1
                width: visible ? 100 : 0
                implicitWidth: width
                Flickable {
                    id: categoriesView
                    contentWidth: width
                    contentHeight: childrenRect.height
                    anchors.fill: parent

                    property Item currentItem: categoriesColumn.children[1]

                    Item {
                        id: categories
                        width: parent.width
                        height: categoriesColumn.height

                        Item {
                            width: parent.width
                            height: categoriesView.currentItem.height
                            y: categoriesView.currentItem.y
                            Rectangle {
                                color: syspal.highlight
                                radius: 3
                                anchors {
                                    fill: parent
                                    margins: 2
                                }
                            }
                            Behavior on y {
                                NumberAnimation {
                                    duration: units.longDuration
                                    easing.type: "InOutQuad"
                                }
                            }
                        }
                        Column {
                            id: categoriesColumn
                            width: parent.width
                            Repeater {
                                model: configDialog.configModel
                                delegate: ConfigCategoryDelegate {
                                    onClicked: {
                                        categoriesView.currentIndex = index
                                        pageTitle.text = name;
                                    }

                                }
                            }
                            Repeater {
                                model: globalConfigModel
                                delegate: ConfigCategoryDelegate {}
                            }
                        }
                    }
                }
            }
            Item {
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }
                Layout.fillWidth: true
                height: Math.max(categoriesScroll.height, main.currentItem != null ? main.currentItem.implicitHeight : 0)

                QtControls.Label {
                    id: pageTitle
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                    font.pointSize: theme.defaultFont.pointSize*2
                    text: main.title
                }

                QtControls.StackView {
                    id: main
                    property string title: ""
                    anchors {
                        top: pageTitle.bottom
                        topMargin: units.largeSpacing / 2
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    clip: true
                    property string sourceFile
                    Timer {
                        id: pageSizeSync
                        interval: 100
                        onTriggered: {
    //                                     root.width = mainColumn.implicitWidth
    //                                     root.height = mainColumn.implicitHeight
                        }
                    }
                    onImplicitWidthChanged: pageSizeSync.restart()
                    onImplicitHeightChanged: pageSizeSync.restart()
                    onSourceFileChanged: {
                        print("Source file changed in flickable" + sourceFile);
                        replace(Qt.resolvedUrl(sourceFile))
                        /*
                            * This is not needed on a desktop shell that has ok/apply/cancel buttons, i'll leave it here only for future reference until we have a prototype for the active shell.
                            * root.pageChanged will start a timer, that in turn will call saveConfig() when triggered

                        for (var prop in currentPage) {
                            if (prop.indexOf("cfg_") === 0) {
                                currentPage[prop+"Changed"].connect(root.pageChanged)
                            }
                        }*/
                    }
                }
            }
        }
        RowLayout {
            id: buttonsRow
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            QtControls.Button {
                iconName: "dialog-ok"
                text: i18n("Ok")
                onClicked: {
                    if (main.currentItem.saveConfig !== undefined) {
                        main.currentItem.saveConfig();
                    } else {
                        root.saveConfig();
                    }
                    configDialog.close();
                }
            }
            QtControls.Button {
                iconName: "dialog-ok-apply"
                text: i18n("Apply")
                onClicked: {
                    if (main.currentItem.saveConfig !== undefined) {
                        main.currentItem.saveConfig();
                    } else {
                        root.saveConfig();
                    }
                }
            }
            QtControls.Button {
                iconName: "dialog-cancel"
                text: i18n("Cancel")
                onClicked: configDialog.close()
            }
        }
    }
//END UI components
}
