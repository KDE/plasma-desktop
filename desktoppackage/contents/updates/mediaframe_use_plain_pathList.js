/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/**
    The old path list contains serialized JSON data, but only the "path" key is used.

    @see https://invent.kde.org/plasma/kdeplasma-addons/-/merge_requests/392
    @since 6.0
*/
const containments = desktops().concat(panels());

containments.forEach(containment => containment.widgets("org.kde.plasma.mediaframe").forEach(widget => {
    widget.currentConfigGroup = ["Paths"];
    let pathList = widget.readConfig("pathList", []);
    if (pathList.length > 0) {
        let newPathList = [];
        for (let jsonStr of pathList) {
            const parsedData = JSON.parse(jsonStr);
            newPathList.push(parsedData.path);
        }
        widget.writeConfig("pathList", newPathList);
    }
}));
