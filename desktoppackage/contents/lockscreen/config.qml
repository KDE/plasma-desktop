import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcmutils as KCM

Kirigami.FormLayout {
    property alias cfg_alwaysShowClock: alwaysClock.checked
    property alias cfg_showMediaControls: showMediaControls.checked
    property bool cfg_alwaysShowClockDefault: false
    property bool cfg_showMediaControlsDefault: false

    twinFormLayouts: parentLayout

    QQC2.CheckBox {
        id: alwaysClock
        Kirigami.FormData.label: i18ndc("plasma_lookandfeel_org.kde.lookandfeel",
                                        "@title: group",
                                        "Clock:")
        text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel",
                     "@option:check",
                     "Keep visible when unlocking prompt disappears")

        KCM.SettingHighlighter {
            highlight: cfg_alwaysShowClockDefault != cfg_alwaysShowClock
        }
    }

    QQC2.CheckBox {
        id: showMediaControls
        Kirigami.FormData.label: i18ndc("plasma_lookandfeel_org.kde.lookandfeel",
                                        "@title: group",
                                        "Media controls:")
        text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel",
                     "@option:check",
                     "Show under unlocking prompt")

        KCM.SettingHighlighter {
            highlight: cfg_showMediaControlsDefault != cfg_showMediaControls
        }
    }
}
