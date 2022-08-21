/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

GridLayout {
    id: root

    enum LayoutType {
        HorizontalPanel,
        VerticalPanel,
        Desktop,
        DesktopCompact
    }

    property var reason
    property var errorInformation: {}

    readonly property real minimumPreferredWidth: PlasmaCore.Units.gridUnit * 12
    readonly property real minimumPreferredHeight: PlasmaCore.Units.gridUnit * 12

    // To properly show the error message in panel
    readonly property int layoutForm: {
        if (root.width >= root.minimumPreferredWidth) {
            if (root.height >= root.minimumPreferredHeight) {
                return AppletError.Desktop;
            } else if (root.height >= PlasmaCore.Units.iconSizes.huge + buttonLayout.implicitHeight) {
                return AppletError.DesktopCompact;
            }
        }

        return Plasmoid.formFactor === PlasmaCore.Types.Vertical ? AppletError.VerticalPanel : AppletError.HorizontalPanel;
    }

    Layout.minimumWidth: {
        switch (root.layoutForm) {
        case AppletError.Desktop:
        case AppletError.DesktopCompact:
            // [Icon] [Text]
            //    [Button]
            // [Information]
            return Math.max(root.minimumPreferredWidth, buttonLayout.implicitWidth);
        case AppletError.VerticalPanel:
            // [Icon]
            // [Copy]
            // [Open]
            return Math.max(headerIcon.implicitWidth, buttonLayout.implicitWidth);
        case AppletError.HorizontalPanel:
            // [Icon] [Copy] [Open]
            return headingLayout.implicitWidth + root.rowSpacing + buttonLayout.implicitWidth;
        }
    }
    Layout.minimumHeight: {
        switch (root.layoutForm) {
        case AppletError.Desktop:
            return headingLayout.implicitHeight + root.columnSpacing + buttonLayout.implicitHeight + root.columnSpacing + fullContentView.implicitHeight;
        case AppletError.DesktopCompact:
            return Math.max(headingLayout.implicitHeight, buttonLayout.implicitHeight);
        case AppletError.VerticalPanel:
            return headingLayout.implicitHeight + root.columnSpacing + buttonLayout.implicitHeight;
        case AppletError.HorizontalPanel:
            return Math.max(headingLayout.implicitHeight, buttonLayout.implicitHeight);
        }
    }
    // Same as systray popups
    Layout.preferredWidth: PlasmaCore.Units.gridUnit * 24
    Layout.preferredHeight: PlasmaCore.Units.gridUnit * 24

    rowSpacing: textArea.topPadding
    columnSpacing: rowSpacing
    flow: {
        switch (root.layoutForm) {
        case AppletError.HorizontalPanel:
            return GridLayout.LeftToRight;
        default:
            return GridLayout.TopToBottom;
        }
    }

    RowLayout {
        id: headingLayout

        Layout.margins: root.layoutForm !== AppletError.Desktop ? 0 : PlasmaCore.Units.gridUnit
        Layout.maximumWidth: root.width
        spacing: 0

        PlasmaCore.IconItem {
            id: headerIcon
            implicitWidth: Math.min(PlasmaCore.Units.iconSizes.huge, root.width, root.height)
            implicitHeight: implicitWidth

            activeFocusOnTab: true
            source: "dialog-error"

            Accessible.description: heading.text

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                enabled: !heading.visible || heading.truncated
                mainText: heading.text
                textFormat: Text.PlainText
            }
        }

        PlasmaExtras.Heading {
            id: heading
            visible: root.layoutForm !== AppletError.VerticalPanel
            // Descent is equal to the amount of space above and below capital letters.
            // Add descent to the sides to make the spacing around Latin text look more even.
            leftPadding: headingFontMetrics.descent
            rightPadding: headingFontMetrics.descent
            text: root.errorInformation && root.errorInformation.appletName ?
                i18nd("plasma_shell_org.kde.plasma.desktop", "Sorry! There was an error loading %1.", root.errorInformation.appletName)
                // This is just to suppress warnings. Users should never see this.
                : "No error information."
            level: 2
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.maximumHeight: headerIcon.implicitHeight

            FontMetrics {
                id: headingFontMetrics
                font: heading.font
            }
        }
    }

    GridLayout {
        id: buttonLayout

        Layout.alignment: Qt.AlignCenter

        rowSpacing: parent.rowSpacing
        columnSpacing: parent.columnSpacing
        flow: {
            switch (root.layoutForm) {
            case AppletError.HorizontalPanel:
            case AppletError.VerticalPanel:
                return root.flow;
            default:
                return GridLayout.LeftToRight;
            }
        }

        PC3.Button {
            id: copyButton
            display: root.layoutForm === AppletError.HorizontalPanel || root.layoutForm === AppletError.VerticalPanel ? PC3.AbstractButton.IconOnly : PC3.AbstractButton.TextBesideIcon
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Copy to Clipboard")
            icon.name: "edit-copy"
            onClicked: {
                textArea.selectAll()
                textArea.copy()
                textArea.deselect()
            }

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                enabled: parent.display === PC3.AbstractButton.IconOnly
                mainText: parent.text
                textFormat: Text.PlainText
            }
        }

        Loader {
            id: compactContentLoader
            active: root.layoutForm !== AppletError.Desktop
            visible: active
            sourceComponent: PC3.Button {
                display: copyButton.display
                icon.name: "window-new"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "View Error Details…")
                checked: dialog.visible
                onClicked: dialog.visible = !dialog.visible

                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    enabled: parent.display === PC3.AbstractButton.IconOnly
                    mainText: parent.text
                    textFormat: Text.PlainText
                }

                QQC2.ApplicationWindow {
                    id: dialog
                    flags: Qt.Dialog | Qt.WindowStaysOnTopHint | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint
                    minimumWidth: dialogFontMetrics.height * 12
                        + dialogTextArea.leftPadding + dialogTextArea.rightPadding
                    minimumHeight: dialogFontMetrics.height * 12
                        + dialogTextArea.topPadding + dialogTextArea.bottomPadding
                    width: PlasmaCore.Units.gridUnit * 24
                    height: PlasmaCore.Units.gridUnit * 24
                    color: palette.base
                    QQC2.ScrollView {
                        id: dialogScrollView
                        anchors.fill: parent
                        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                        QQC2.TextArea {
                            id: dialogTextArea
                            // HACK: silence binding loop warnings.
                            // contentWidth seems to be causing the binding loop,
                            // but contentWidth is read-only and we have no control
                            // over how it is calculated.
                            implicitWidth: 0
                            wrapMode: TextEdit.Wrap
                            text: textArea.text
                            font.family: "monospace"
                            readOnly: true
                            selectByMouse: true
                            background: null
                            FontMetrics {
                                id: dialogFontMetrics
                                font: dialogTextArea.font
                            }
                        }
                        background: null
                    }
                }
            }
        }
    }

    PC3.ScrollView {
        id: fullContentView
        // Not handled by a Loader because we need
        // TextEdit::copy() to copy to clipboard.
        visible: !compactContentLoader.active
        Layout.fillHeight: true
        Layout.fillWidth: true
        PC3.ScrollBar.horizontal.policy: PC3.ScrollBar.AlwaysOff
        PC3.TextArea {
            id: textArea
            // HACK: silence binding loop warnings.
            // contentWidth seems to be causing the binding loop,
            // but contentWidth is read-only and we have no control
            // over how it is calculated.
            implicitWidth: 0
            wrapMode: TextEdit.Wrap
            text: root.errorInformation && root.errorInformation.errors ?
                root.errorInformation.errors.join("\n\n")
                // This is just to suppress warnings. Users should never see this.
                : "No error information."
            font.family: "monospace"
            readOnly: true
            selectByMouse: true
        }
    }
}
