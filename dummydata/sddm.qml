import QtQuick 2.15

Item {
    signal loginFailed()
    signal loginSucceeded()

    function powerOff() {
        console.log("SDDM - POWERING OFF");
    }
    function reboot() {
        console.log("SDDM - REBOOTING");
    }
    function suspend() {
        console.log("SDDM - SUSPEND");
    }
    function hibernate() {
        console.log("SDDM - HIBERNATE");
    }
    function hybridSleep() {
        console.log("SDDM - HYBRID SLEEP");
    }

    function login(user, password, sessionIndex) {
        console.log("SDDM - logging in as ", user, password, sessionIndex)

        //modify as appropriate for testing
        var success = false

        if (success) {
            loginSucceeded();
        } else {
            tryLogin.start();
        }

    }

    Timer {
        id: tryLogin
        interval: 1000
        onTriggered: loginFailed();
    }

    property bool canPowerOff: true
    property bool canReboot: true
    property bool canSuspend: true
    property bool canHibernate: true
    property bool canHybridSleep: true
    property string hostname: "MyHostname"

}
