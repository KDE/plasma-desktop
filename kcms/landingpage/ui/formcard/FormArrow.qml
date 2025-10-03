/*
 * Copyright 2022 Devin Lin <devin@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

/**
 * @brief An arrow UI component used in Form delegates.
 *
 * This component can be used to decorate existing or custom Form delegates.
 * It is used, for instance, as the trailing property of FormButtonDelegate.
 *
 * Each FormArrow instance corresponds to a single arrow that may point
 * upwards, downwards, to the left or to the right.
 *
 * @since KirigamiAddons 0.11.0
 *
 * @inherit Kirigami.Icon
 */

Kirigami.Icon {
    /**
     * @brief The direction the FormArrow will point towards.
     *
     * Set this to any Qt::ArrowType enum value.
     *
     * default: `Qt.RightArrow`
     */
    property int direction: Qt.RightArrow

    source: {
        switch (direction) {
            case Qt.UpArrow:
                return "arrow-up-symbolic";
            case Qt.DownArrow:
                return "arrow-down-symbolic";
            case Qt.LeftArrow:
                return "arrow-left-symbolic";
            case Qt.RightArrow:
                return "arrow-right-symbolic";
        }
    }
    implicitWidth: 12
    implicitHeight: 12
}
