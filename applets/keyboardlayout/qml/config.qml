import QtQuick
import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
        name: i18nc("@title:group for configuration dialog page", "General")
        icon: "configure"
        source: "configGeneral.qml"
    }
}
