import QtQuick 2.15

//NOTE: We do not implement a fully working dummy screenModel,
//we only implement what's needed for a basic multi-monitor test

ListModel {
    property int primary: 0

    //Creating multiple ListElement objects doesn't work, because a ListElement can only take numbers,
    //strings, booleans or enums, but we need a rect (it stores the screen size/position).
    //We can insert rects into the ListModel by using this workaround
    //It's taken from: https://stackoverflow.com/questions/20537417/add-statically-object-to-listmodel

    //We create two monitors here, the left one 800x600 and the right one 800x400 pixels
    //To disable a screen delete one of the "append" function calls
    Component.onCompleted: {
        append({
            name: "Screen 1",
            geometry: {x: 0, y: 0, width: 1600, height: 900},
        });

        append({
            name: "Screen 2",
            geometry: {x: 1980, y: 0, width: 1600, height: 900},
        });
    }

    function geometry() {
        //return the primary monitor size
        return Qt.rect(800, 0, 800, 400);
    }
}
