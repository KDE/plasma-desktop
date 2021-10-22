/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var layout;
var root;
var plasmoid;
var lastSpacer;
var marginHighlights;


function restore() {
    var configString = String(plasmoid.configuration.AppletOrder)

    //array, a cell for encoded item order
    var itemsArray = configString.split(";");

    //map applet id->order in panel
    var idsOrder = new Object();
    //map order in panel -> applet pointer
    var appletsOrder = new Object();

    for (var i = 0; i < itemsArray.length; i++) {
        //property name: applet id
        //property value: order
        idsOrder[itemsArray[i]] = i;
    }

    for (var i = 0; i < plasmoid.applets.length; ++i) {
        if (idsOrder[plasmoid.applets[i].id] !== undefined) {
            appletsOrder[idsOrder[plasmoid.applets[i].id]] = plasmoid.applets[i];
        //ones that weren't saved in AppletOrder go to the end
        } else {
            appletsOrder["unordered"+i] = plasmoid.applets[i];
        }
    }

    //finally, restore the applets in the correct order
    for (var i in appletsOrder) {
        root.addApplet(appletsOrder[i], -1, -1)
    }
    //rewrite, so if in the orders there were now invalid ids or if some were missing creates a correct list instead
    save();
}

function save() {
    var ids = new Array();
    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];

        if (child.applet) {
            ids.push(child.applet.id);
        }
    }
    plasmoid.configuration.AppletOrder = ids.join(';');
    updateMargins();
}

function removeApplet (applet) {
    for (var i = layout.children.length - 1; i >= 0; --i) {
        var child = layout.children[i];
        if (child.applet === applet) {
            // This makes sure the child is not in the layout.children anymore
            // even while it's being destroyed.
            child.parent = root;
            child.destroy();
        }
    }
}

//insert item2 before item1
function insertBefore(item1, item2) {
    if (item1 === item2) {
        return;
    }
    var removed = new Array();

    var child;

    var i;
    for (i = layout.children.length - 1; i >= 0; --i) {
        child = layout.children[i];
        removed.push(child);
        child.parent = root;

        if (child === item1) {
            break;
        }
    }

    item2.parent = layout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = layout;
    }
    return i;
}

//insert item2 after item1
function insertAfter(item1, item2) {
    if (item1 === item2) {
        return;
    }
    var removed = new Array();

    var child;

    var i;
    for (i = layout.children.length - 1; i >= 0; --i) {
        child = layout.children[i];
        //never ever insert after lastSpacer
        if (child === item1) {
            //Already in position, do nothing
            if (layout.children[i+1] === item2) {
                return;
            }
            break;
        }

        removed.push(child);
        child.parent = root;
    }

    item2.parent = layout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = layout;
    }
    return i;
}

function insertAtIndex(item, position) {
    if (position < 0 || position >= layout.children.length) {
        return;
    }

    //never ever insert after lastSpacer
    if (layout.children[position] === lastSpacer) {
        --position;
    }

    var removedItems = new Array();

    for (var i = position; i < layout.children.length; ++i) {
        var child = layout.children[position];
        child.parent = root;
        removedItems.push(child);
    }

    item.parent = layout;
    for (var i in removedItems) {
        removedItems[i].parent = layout;
    }
}

function insertAtCoordinates(item, x, y) {
    if (root.isHorizontal) {
        y = layout.height / 2;
    } else {
        x = layout.width / 2;
    }
    var child = layout.childAt(x, y);

    //if we got a place inside the space between 2 applets, we have to find it manually
    if (!child) {
        if (root.isHorizontal) {
            for (var i = 0; i < layout.children.length; ++i) {
                var candidate = layout.children[i];
                // It must be at on the same or a higner x position as the candidate
                if (x >= candidate.x) {
                    // Taking the candidate with & rowSpacing into account, it must be smaller
                    const totalXofCondidate= candidate.x + candidate.width + layout.rowSpacing * 2
                    if (x <= totalXofCondidate) {
                        child = candidate;
                        break;
                    }
                }
            }
        } else {
            for (var i = 0; i < layout.children.length; ++i) {
                var candidate = layout.children[i];
                if (y >= candidate.x && y < candidate.y + candidate.height + layout.columnSpacing) {
                    child = candidate;
                    break;
                }
            }
        }
    }
    //already in position
    if (child === item) {
        return;
    }
    if (!child) {
        child = layout.children[0];
    }
    item.parent = root;

    //PlasmaCore.Types.Vertical = 3
    if ((plasmoid.formFactor === 3 && y < child.y + child.height/2) ||
        (plasmoid.formFactor !== 3 && x < child.x + child.width/2)) {
        return insertBefore(child, item);
    } else {
        return insertAfter(child, item);
    }
}

function updateMargins() {
    for (var i = 0; i < marginHighlights.length; ++i) {
        marginHighlights[i].destroy();
    }
    marginHighlights = [];
    var inThickArea = false;
    var startApplet = undefined;
    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];
        if (child.dragging) {child = child.dragging}
        if (child.applet) {
            child.inThickArea = inThickArea;
            if ((child.applet.constraintHints & PlasmaCore.Types.MarginAreasSeparator) == PlasmaCore.Types.MarginAreasSeparator) {
                var marginRect = rectHighlightEl.createObject(root, {
                    startApplet: startApplet,
                    endApplet: child,
                    thickArea: inThickArea
                });
                marginHighlights.push(marginRect);
                var startApplet = child;
                inThickArea = !inThickArea;
            }
        }
    }
    if (marginHighlights.length == 0) return;
    var marginRect = rectHighlightEl.createObject(root, {
        startApplet: startApplet,
        endApplet: undefined,
        thickArea: inThickArea
    });
    marginHighlights.push(marginRect);
}
