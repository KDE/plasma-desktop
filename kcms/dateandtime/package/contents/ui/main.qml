/*
 * Copyright (C) 2015 Marco Martin <mart@kde.org>
 * Copyright (C) 2018 Eike Hein <hein@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as QtControls1

import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.2

Kirigami.Page {
    id: root

    implicitWidth: 500
    implicitHeight: 500

    Kirigami.OverlaySheet {
        id: addLanguagesSheet

        parent: root
        header: Kirigami.Heading { text: i18nc("@title:window", "Select time")}

        RowLayout {

            QtControls.SpinBox {
                value: 2
            }
            QtControls.Label {text:"/"}

            QtControls.SpinBox {
                value: 0
                textFromValue: function() {return "March"}
            }
            QtControls.Label {text:"/"}
            QtControls.SpinBox {
                value: 2017
            }

            QtControls.Label {text: "   "}

            QtControls.SpinBox {}
            QtControls.Label {text:":"}
            QtControls.SpinBox {}

        }
    }


    ColumnLayout {
        anchors.fill: parent

        QtControls.CheckBox {
            id: useNtp
            checked: true
            text: "Set date and time automatically"
        }

//        RowLayout {
//            QtControls1.Calendar {

//            }
//            Item {
//                Layout.fillWidth: true
//            }
//            ColumnLayout {
//                ClockFace {
//                    implicitWidth: 200
//                    implicitHeight: 200
//                }

//                //spin box with textFromValue + valueFromText nearly works, but
//                //the lack of input mask means that the user has to put ":" in the right place and the cursor isn't very good

//                //maybe 3 tumblers would be better? Maybe switch on context? or when you touch the clock face?

//                QtControls.TextField {
//                    //time represented as seconds after midnight. No date
//                    property int value

//                    Layout.alignment: Qt.AlignHCenter

//                    inputMethodHints: Qt.ImredhTime
//                    enabled: !useNtp.checked

//                    function formatTime(time) {
//                        var hours = 13;
//                        var minutes = 3
//                        var seconds = 2;
//                        var text;
//                        if (hours < 10)
//                            text += "0";
//                        text += hours;
//                        text += ":";
//                        if (minutes < 10)
//                            text += "0"
//                        text += minutes;
//                        text += ":";
//                        if (seconds < 10)
//                            text += "0";
//                        text += seconds;
//                        return text;
//                    }


//                    onValueChanged: {
//                        text = formatTime(text)
//                    }

//                    text: "10:04:03"
//                    inputMask: "09:09:09" //optional digit + mandatory digit * 3
//                }
//            }
//        }

        TimezoneTable {
            Layout.fillHeight: true
        }

        RowLayout {
            QtControls.Label {
                font.bold: true
                text: "14th April 2017  14:34"
                Layout.fillWidth: true
            }
            QtControls.Button {
                text: i18n("Set")
                onClicked: addLanguagesSheet.sheetOpen = !addLanguagesSheet.sheetOpen
            }
        }

    }
}

