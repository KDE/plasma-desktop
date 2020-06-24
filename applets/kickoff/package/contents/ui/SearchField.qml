/**
 * SPDX-FileCopyrightText: (C) 2014 Weng Xuetian <wengxt@gmail.com>
 * SPDX-FileCopyrightText: (C) 2013-2017 Eike Hein <hein@kde.org>                   *
 * SPDX-FileCopyrightText: (C) 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.4
import org.kde.plasma.components 3.0 as PlasmaComponents3

PlasmaComponents3.TextField {
    id: searchField

    anchors.fill: parent

    placeholderText: i18n("Search...")

    onTextChanged: runnerModel.query = text;

    Keys.onPressed: {
        /*if (event.key == Qt.Key_Down) {
            event.accepted = true;
            pageList.currentItem.itemGrid.tryActivate(0, 0);
        } else if (event.key == Qt.Key_Right) {
            if (cursorPosition == length) {
                event.accepted = true;

                if (pageList.currentItem.itemGrid.currentIndex == -1) {
                    pageList.currentItem.itemGrid.tryActivate(0, 0);
                } else {
                    pageList.currentItem.itemGrid.tryActivate(0, 1);
                }
            }
        } else if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
            if (text != "" && pageList.currentItem.itemGrid.count > 0) {
                event.accepted = true;
                pageList.currentItem.itemGrid.tryActivate(0, 0);
                pageList.currentItem.itemGrid.model.trigger(0, "", null);
                root.visible = false;
            }
        } else if (event.key == Qt.Key_Tab) {
            event.accepted = true;
            systemFavoritesGrid.tryActivate(0, 0);
        } else if (event.key == Qt.Key_Backtab) {
            event.accepted = true;

            if (!searching) {
                filterList.forceActiveFocus();
            } else {
                systemFavoritesGrid.tryActivate(0, 0);
            }
        }*/
    }

    function backspace() {
        if (!root.visible) {
            return;
        }

        focus = true;
        text = text.slice(0, -1);
    }

    function appendText(newText) {
        if (!root.visible) {
            return;
        }

        focus = true;
        text = text + newText;
    }
}
