import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.3

import org.kde.kirigami 2.4 as Kirigami

/**
 * This Item provides an entry box for inputting a date.
 * It is represented in a form suitable for entering a known date (i.e date of birth, current date)
 * rather than choosing dates where a gridview might work better
 */
RowLayout {
    id: layout

    //DAVE - if I'm in an RTL country are my date formats pre-reversed?
    //i.e Wikipedia says afghan is d/m/yyyy   should year go on the left or the right?

    property date value

    property string dateFormat: Qt.locale().dateFormat(Locale.ShortFormat)

    //date formats can be in big endian (china), little endian (Europe), or absolutely mental endian
    //separators are also different
    Component.onCompleted: {
        value = new Date();

        //dave, some idiot could have added children externally, maybe  would be safer to make RowLayout internal?
        for (var i in layout.children) {
            layout.children[i].destroy();
        }

        var tabletMode = Kirigami.Settings.tabletMode

        var parse = /([^dMy]*)([dMy]+)([^dMy]*)([dMy]+)([^dMy]*)([dMy]+)([^dMy]*)/
        var parts = parse.exec(dateFormat);
        for(var i=1; i < parts.length; i++) {
            var part = parts[i];

            if (!part) {
                continue;
            }

            if (part.startsWith("d")) {
                if (tabletMode) {
                    daySelectTouchComponent.createObject(layout);
                } else {
                    daySelectComponent.createObject(layout);
                }
            }
            else if (part.startsWith("M")) {
                if (tabletMode) {
                    monthSelectTouchComponent.createObject(layout);
                } else {
                    monthSelectComponent.createObject(layout);
                }            }
            else if (part.startsWith("y")) {
                if (tabletMode) {
                    yearSelectTouchComponent.createObject(layout);
                } else {
                    yearSelectComponent.createObject(layout);
                }                   }
            else {
                labelComponent.createObject(layout, {"text": part})
            }
        }
    }

    Component {
        id: daySelectComponent
        SpinBox {
            from: 1
            to: 31
            editable: true
            value: layout.value.getDay();
            onValueModified: layout.value.setDate(value); //setDate only sets the day. It makes no sense
        }
    }
    Component {
        id: daySelectTouchComponent
        Tumbler {
            model: 31
        }
        //Tumbler doesn't have a separate user modified signal...booooooo!!!!!
    }

    Component {
        id: monthSelectComponent
        SpinBox {
            from: 1
            to: 12
            editable: true
            value: layout.value.getMonth();
            onValueModified: layout.value.setMonth(value);
        }
    }
    Component {
        id: monthSelectTouchComponent
        Tumbler {
            model: 12
        }
    }
    Component {
        id: yearSelectComponent
        SpinBox {
            from: 1970
            to: 2100 //I assume we'll have a new LTS release by then
            editable: true
            textFromValue: function(value) {return value} //default implementation does toLocaleString which looks super weird
            value: layout.value.getFullYear();
            onValueModified: layout.value.setFullYear(value);
        }
    }

    Component {
        id: yearSelectTouchComponent
        Tumbler {
            model: 12
        }
    }

    Component {
        id: labelComponent
        Label {
        }
    }
}
