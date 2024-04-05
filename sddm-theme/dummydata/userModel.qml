import QtQuick 2.15

ListModel {

    property int lastIndex: 1
    property string lastUser: "someperson"

    property int disableAvatarsThreshold: 10

    ListElement {
        name: "david"
        realName: "David Edmundson"
        icon: "/home/david/.face.icon"
        homeDir: "/home/david"
    }
    ListElement {
        name: "afiestas"
        realName: "Alex Fiestas"
        icon: "../face1.jpeg"
        homeDir: ""
    }
    ListElement {
        //no real name and no icon
        name: "someperson"
        realName: ""
        homeDir: ""
    }
    ListElement {
        //deliberately long name
        name: "shadeslayer"
        realName: "Sebastian Schweinstager"
        icon: "/home/david/.face.icon"
        homeDir:""
    }
}
