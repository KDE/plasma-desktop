import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import org.kde.kirigami as Kirigami

Item {
    id: root

    property string title
    default property alias entries: innerLayout.data
    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    Layout.fillWidth: true

    Kirigami.HeaderFooterLayout {
        id: layout
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing
        // TODO: HeaderFooterLayout needs headerMargin/footerMargin

        header: Kirigami.Heading {
            level: 5
            font.weight: Font.DemiBold
            visible: text.length > 0
            text: root.title
        }
        contentItem: QQC.Control {
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            contentItem: ColumnLayout {
                id: innerLayout
                spacing: 0
            }
            background: Kirigami.ShadowedRectangle {
                radius: Kirigami.Units.cornerRadius

                border {
                    color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.15)
                    width: 1
                }

                shadow {
                    size: Kirigami.Units.largeSpacing
                    color: Qt.alpha(Kirigami.Theme.textColor, 0.10)
                }
            }
        }
    }
}
