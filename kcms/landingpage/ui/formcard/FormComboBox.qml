/*
 * Copyright 2022 Devin Lin <devin@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components

/**
 * @brief A Form delegate that corresponds to a combobox.
 *
 * This component is used for individual settings that can have multiple
 * possible values shown in a vertical list, typically defined in a ::model.
 *
 * Many of its properties require familiarity with QtQuick.Controls.ComboBox.
 *
 * Use the inherited QtQuick.Controls.AbstractButton.text property to define
 * the main text of the combobox.
 *
 * If you need a purely on/off toggle, use a FormSwitchDelegate instead.
 *
 * If you need an on/off/tristate toggle, use a FormCheckDelegate instead.
 *
 * If you need multiple toggles instead of multiple values for the same
 * setting, consider using a FormRadioDelegate.
 *
 * @since KirigamiAddons 0.11.0
 *
 * @see QtQuick.Controls.AbstractButton
 * @see FormSwitchDelegate
 * @see FormCheckDelegate
 * @see FormRadioDelegate
 *
 */
QQC2.AbstractButton {
    id: controlRoot

    /**
     * @brief This signal is emitted when the item at @p index is activated
     * by the user.
     */
    signal activated(int index)

    /**
     * @brief This signal is emitted when the Return or Enter key is pressed
     * while an editable combo box is focused.
     *
     * @see editable
     */
    signal accepted()

    /**
     * @brief A label that contains secondary text that appears under the
     * inherited text property.
     *
     * This provides additional information shown in a faint gray color.
     *
     * This is supposed to be a short text and the API user should avoid
     * making it longer than two lines.
     */
    property string description: ""

    /**
     * @brief This property holds the value of the current item in the combobox.
     */
    property alias currentValue: combobox.currentValue

    /**
     * @brief This property holds the text of the current item in the combobox.
     *
     * @see displayText
     */
    property alias currentText: combobox.currentText

    /**
     * @brief This property holds the model providing data for the combobox.
     *
     * @see displayText
     * @see QtQuick.Controls.ComboBox.model
     * @see <a href="https://doc.qt.io/qt-6/qtquick-modelviewsdata-modelview.html">Models and Views in QtQuick</a>
     */
    property var model

    /**
     * @brief This property holds the `count` of the internal combobox.
     *
     * @see QtQuick.Controls.ComboBox.count
     * @since Kirigami Addons 1.4.0
     */
    property alias count: combobox.count

    /**
     * @brief This property holds the `textRole` of the internal combobox.
     *
     * @see QtQuick.Controls.ComboBox.textRole
     */
    property alias textRole: combobox.textRole

    /**
     * @brief This property holds the `valueRole` of the internal combobox.
     *
     * @see QtQuick.Controls.ComboBox.valueRole
     */
    property alias valueRole: combobox.valueRole

    /**
     * @brief This property holds the `currentIndex` of the internal combobox.
     *
     * default: `-1` when the ::model has no data, `0` otherwise
     *
     * @see QtQuick.Controls.ComboBox.currentIndex
     */
    property alias currentIndex: combobox.currentIndex

    /**
     * @brief This property holds the `highlightedIndex` of the internal combobox.
     *
     * @see QtQuick.Controls.ComboBox.highlightedIndex
     */
    property alias highlightedIndex: combobox.highlightedIndex

    /**
     * @brief This property holds the `displayText` of the internal combobox.
     *
     * This can be used to slightly modify the text to be displayed in the combobox, for instance, by adding a string with the ::currentText.
     *
     * @see QtQuick.Controls.ComboBox.displayText
     */
    property alias displayText: combobox.displayText

    /**
     * @brief This property holds the `editable` property of the internal combobox.
     *
     * This turns the combobox editable, allowing the user to specify
     * existing values or add new ones.
     *
     * Use this only when ::displayMode is set to
     * FormComboBox.ComboBox.
     *
     * @see QtQuick.Controls.ComboBox.editable
     */
    property alias editable: combobox.editable

    /**
     * @brief This property holds the `editText` property of the internal combobox.
     *
     * @see QtQuick.Controls.ComboBox.editText
     */
    property alias editText: combobox.editText

    /** @brief The enum used to determine the ::displayMode. **/
    enum DisplayMode {
        /**
         * A standard combobox component containing a vertical list of values.
         */
        ComboBox,
        /**
         * A button with similar appearance to a combobox that, when clicked,
         * shows a Kirigami.OverlaySheet at the middle of the window
         * containing a vertical list of values.
         */
        Dialog,
        /**
         * A button with similar appearance to a combobox that, when clicked,
         * shows a Kirigami.ScrollablePage in a new window containing a
         * vertical list of values.
         */
        Page
    }

    /**
     * @brief This property holds what display mode the delegate should show as.
     *
     * Set this property to the desired ::DisplayMode.
     *
     * default: `FormComboBox.ComboBox`
     *
     * @see DisplayMode
     */
    property int displayMode: Kirigami.Settings.isMobile ? FormComboBox.Dialog : FormComboBox.ComboBox

    /**
     * @brief The delegate component to use as entries in the combobox display mode.
     */
    property Component comboBoxDelegate: Delegates.RoundedItemDelegate {
        implicitWidth: ListView.view ? ListView.view.width : Kirigami.Units.gridUnit * 16
        text: controlRoot.textRole ? (Array.isArray(controlRoot.model) ? modelData[controlRoot.textRole] : model[controlRoot.textRole]) : modelData
        highlighted: controlRoot.highlightedIndex === index
    }

    /**
     * @brief The delegate component to use as entries for each value in the dialog and page display mode.
     */
    property Component dialogDelegate: Delegates.RoundedItemDelegate {
        implicitWidth: ListView.view ? ListView.view.width : Kirigami.Units.gridUnit * 16
        text: controlRoot.textRole ? (Array.isArray(controlRoot.model) ? modelData[controlRoot.textRole] : model[controlRoot.textRole]) : modelData

        Layout.topMargin: index == 0 ? Math.round(Kirigami.Units.smallSpacing / 2) : 0

        onClicked: {
            controlRoot.currentIndex = index;
            controlRoot.activated(index);
            controlRoot.closeDialog();
        }
    }

    /**
     * @brief This property holds the current status message type of
     * the text field.
     *
     * This consists of an inline message with a colorful background
     * and an appropriate icon.
     *
     * The status property will affect the color of ::statusMessage used.
     *
     * Accepted values:
     * - `Kirigami.MessageType.Information` (blue color)
     * - `Kirigami.MessageType.Positive` (green color)
     * - `Kirigami.MessageType.Warning` (orange color)
     * - `Kirigami.MessageType.Error` (red color)
     *
     * default: `Kirigami.MessageType.Information` if ::statusMessage is set,
     * nothing otherwise.
     *
     * @see Kirigami.MessageType
     * @since 1.5.0
     */
    property var status: Kirigami.MessageType.Information

    /**
     * @brief This property holds the current status message of
     * the text field.
     *
     * If this property is not set, no ::status will be shown.
     *
     * @since 1.5.0
     */
    property string statusMessage: ""

    /**
     * @brief Closes the dialog or layer.
     *
     * This function can be used when reimplementing the ::page or ::dialog.
     */
    function closeDialog() {
        if (_selectionPageItem) {
            _selectionPageItem.closeDialog();
            _selectionPageItem = null;
        }

        if (_selectionDialogItem) {
            _selectionDialogItem.close();
        }
    }

    property var _selectionPageItem: null
    property var _selectionDialogItem: null
    property real __indicatorMargin: controlRoot.indicator && controlRoot.indicator.visible && controlRoot.indicator.width > 0 ? controlRoot.spacing + indicator.width + controlRoot.spacing : 0

    leftPadding: horizontalPadding + (!controlRoot.mirrored ? 0 : __indicatorMargin)
    rightPadding: horizontalPadding + (controlRoot.mirrored ? 0 : __indicatorMargin)


    // use connections instead of onClicked on root, so that users can supply
    // their own behaviour.
    Connections {
        target: controlRoot
        function onClicked() {
            if (controlRoot.displayMode === FormComboBox.Dialog) {
                controlRoot._selectionDialogItem = controlRoot.dialog.createObject();
                controlRoot._selectionDialogItem.open();
            } else if (controlRoot.displayMode === FormComboBox.Page) {
                controlRoot._selectionPageItem = controlRoot.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(page)
            } else {
                combobox.popup.open();
                combobox.forceActiveFocus(Qt.PopupFocusReason);
            }
        }
    }

    /**
     * @brief The dialog component used for the combobox.
     *
     * This property allows to override the internal dialog
     * with a custom component.
     */
    property Component dialog: QQC2.Dialog {
        id: dialog

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        z: Kirigami.OverlayZStacking.z

        title: controlRoot.text
        implicitWidth: Math.min(parent.width - Kirigami.Units.gridUnit * 2, Kirigami.Units.gridUnit * 22)
        parent: controlRoot.QQC2.Overlay.overlay
        background: Components.DialogRoundedBackground {}

        implicitHeight: Math.min(
            Math.max(implicitBackgroundHeight + topInset + bottomInset,
                     contentHeight + topPadding + bottomPadding
                     + (implicitHeaderHeight > 0 ? implicitHeaderHeight + spacing : 0)
                     + (implicitFooterHeight > 0 ? implicitFooterHeight + spacing : 0)),
            parent.height - Kirigami.Units.gridUnit * 2)

        onClosed: destroy();

        modal: true
        focus: true
        padding: 0

        header: Kirigami.Heading {
            text: dialog.title
            elide: QQC2.Label.ElideRight
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
        }

        contentItem: ColumnLayout {
            spacing: 0

            Kirigami.Separator {
                visible: !listView.atYBeginning
                Layout.fillWidth: true
            }

            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.margins: 2

                Component.onCompleted: if (background) {
                    background.visible = false;
                }

                ListView {
                    id: listView

                    clip: true
                    model: controlRoot.model
                    delegate: controlRoot.dialogDelegate
                    currentIndex: controlRoot.currentIndex
                    onCurrentIndexChanged: controlRoot.currentIndex = currentIndex
                }
            }

            Kirigami.Separator {
                visible: controlRoot.editable
                Layout.fillWidth: true
            }

            QQC2.TextField {
                visible: controlRoot.editable
                onTextChanged: controlRoot.editText = text;
                Layout.fillWidth: true
            }
        }
    }

    /**
     * @brief The page component used for the combobox, if applicable.
     *
     * This property allows to override the internal
     * Kirigami.ScrollablePage with a custom component.
     */
    property Component page: Kirigami.ScrollablePage {
        title: controlRoot.text

        ListView {
            spacing: 0
            model: controlRoot.model
            delegate: controlRoot.dialogDelegate

            footer: QQC2.TextField {
                visible: controlRoot.editable
                onTextChanged: controlRoot.editText = text;
                Layout.fillWidth: true
            }
        }
    }

    function indexOfValue(value) {
        return combobox.indexOfValue(value);
    }

    focusPolicy: Qt.StrongFocus
    Accessible.description: description
    Accessible.onPressAction: controlRoot.clicked()

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            visible: controlRoot.displayMode === FormComboBox.Dialog || controlRoot.displayMode === FormComboBox.Page
            Item {
                Layout.fillWidth: true
            }
            QQC2.Label {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: Kirigami.Units.smallSpacing
                color: Kirigami.Theme.disabledTextColor
                text: controlRoot.displayText
            }

            FormArrow {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                direction: Qt.DownArrow
            }
        }

        QQC2.ComboBox {
            id: combobox
            focusPolicy: Qt.NoFocus // provided by parent
            model: controlRoot.model
            visible: controlRoot.displayMode == FormComboBox.ComboBox
            delegate: controlRoot.comboBoxDelegate
            currentIndex: controlRoot.currentIndex
            onActivated: index => controlRoot.activated(index)
            onAccepted: controlRoot.accepted()
            popup.contentItem.clip: true
            Layout.fillWidth: true
        }

        QQC2.Label {
            visible: controlRoot.description !== ""
            Layout.fillWidth: true
            text: controlRoot.description
            color: Kirigami.Theme.disabledTextColor
            wrapMode: Text.Wrap
            Accessible.ignored: true
        }

        Kirigami.InlineMessage {
            visible: controlRoot.statusMessage.length > 0
            Layout.topMargin: visible ? Kirigami.Units.smallSpacing : 0
            Layout.fillWidth: true
            text: controlRoot.statusMessage
            type: controlRoot.status
        }
    }
}

