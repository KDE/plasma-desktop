/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

var positions = []

var cellSize = {
    width: units.gridUnit,
    height: units.gridUnit
}

var defaultAppletSize = {
    width: cellSize.width * 6,
    height: cellSize.height * 6
}

var resultsFlow
var plasmoid;
var itemsConfig
var rotations = {}

//bookkeeping for the item groups
var itemGroups = {}

function restore()
{
    var appletsMap = {};
    var applet;
    var appletsLength = plasmoid.applets.length
    for (var i = 0; i < appletsLength; ++i) {
        applet = plasmoid.applets[i];
        appletsMap["Applet-" + applet.id] = applet;
    }

    itemsConfig = {};
    var configString = String(plasmoid.configuration.ItemsGeometries)
    //print("Config read from configfile: " + configString);
    //array, a cell for encoded item geometry
    var itemsStrings = configString.split(";")
    var itemsStringsLength = itemsStrings.length
    for (var i = 0; i < itemsStringsLength; ++i) {
        //[id, encoded geometry]
        var idConfig = itemsStrings[i].split(":")
        idConfig[0] = idConfig[0].replace("%3A", ":")
        //ignore if malformed or there isn't an applet with such id
        if (idConfig.length < 2 || appletsMap[idConfig[0]] === undefined) {
            continue;
        }

        //array [x, y, width, height, rotation]
        //print("Restoring applet " + idConfig[0] + " at " + idConfig[1]);
        var rect = idConfig[1].split(",")
        if (rect.length < 5) {
            continue
        }

        itemsConfig[idConfig[0]] = {
            x: rect[0],
            y: rect[1],
            width: rect[2],
            height: rect[3],
            rotation: parseFloat(rect[4])
        };
    }
}

function save()
{
    var configString = "";

    for (var _id in itemsConfig) {
        var rect = itemsConfig[_id];
        var idstring = _id.replace(":", "%3A");
        if (idstring != "undefined") {
            configString += idstring + ":" + rect.x + "," + rect.y + "," + rect.width + "," + rect.height + "," + rect.rotation + ";";
        }
    }

    if (plasmoid) {
        plasmoid.configuration.ItemsGeometries = configString;
    }
}

function resetPositions()
{
    positions = []
}

//returns the available size at a given position
function availableSpace(x, y, width, height)
{
    var row = Math.round(x/cellSize.width)
    var column = Math.round(y/cellSize.height)
    var rowsWidth = Math.ceil(width/cellSize.width)
    var columnsHeight = Math.ceil(height/cellSize.height)

    var availableSize = {
        width: 0,
        height: 0
    }

    if (x < 0 || y < 0) {
        return availableSize;
    }

    if (positions[row] === undefined || !positions[row][column]) {
        for (var w = 0; w < rowsWidth; ++w) {
            if (positions[row + w] && positions[row + w][column]) {
                break;
            } else {
                availableSize.width = w+1
            }
        }

        for (var h = 0; h < columnsHeight; ++h) {
            //occupied?
            var free = true

            //using availableSize.width instead of rowsWidth or the result will be 0
            var rowEnd = row + availableSize.width
            for (var i = row; i < rowEnd; ++i) {
                if (positions[i] && positions[i][column + h]) {
                    free = false
                    break
                }
            }

            if (free) {
                availableSize.height = h+1
            } else {
                //print("occupied"+row+" "+column+" "+h+" "+positions[row][column+h]+" "+availableSize.height)
                break
            }
        }
    }

    availableSize.width *= cellSize.width
    availableSize.height *= cellSize.height

    //don't make it overflow
    availableSize.width = Math.min(availableSize.width,
                                   (resultsFlow.width-row*cellSize.width))

    return availableSize
}

function setSpaceAvailable(x, y, width, height, available)
{
    var row = Math.round(x/cellSize.width)
    var column = Math.round(y/cellSize.height)
    var rowsWidth = Math.round(width/cellSize.width)
    var columnsHeight = Math.round(height/cellSize.height)

    var rowEnd = row + rowsWidth;
    for (var i = row; i < rowEnd; ++i) {
        if (!positions[i]) {
            positions[i] = []
        }
        var columnEnd = column + columnsHeight;
        for (var j = column; j < columnEnd; ++j) {
            positions[i][j] = !available
            //print("set "+i+" "+j+" "+!available)
        }
    }
}

function adjustItemSizeToAvailableScreenRegion(x, y, width, height)
{
    var pos = root.mapToItem(root.parent, x, y);
    pos = plasmoid.adjustToAvailableScreenRegion(pos.x, pos.y, width, height);
    pos = root.mapFromItem(root.parent, pos.x, pos.y);
    return pos;
}

function normalizeItemPosition(item)
{
    var x = Math.max(0, Math.round(item.x/cellSize.width)*cellSize.width)
    var y = Math.max(0, Math.round(item.y/cellSize.height)*cellSize.height)

    var width = Math.max(cellSize.width, Math.round(item.width/cellSize.width)*cellSize.width)
    var height = Math.max(cellSize.height, Math.round(item.height/cellSize.height)*cellSize.height)

    var pos = adjustItemSizeToAvailableScreenRegion(x, y, width, height);
    item.x = pos.x;
    item.y = pos.y;

    /*item.width = width
    item.height = height*/
}

function positionItem(item)
{
    var x = Math.max(0, Math.round(item.x/cellSize.width)*cellSize.width)
    var y = Math.max(0, Math.round(item.y/cellSize.height)*cellSize.height)

    var pos = adjustItemSizeToAvailableScreenRegion(x, y, item.width, item.height);
    x = pos.x;
    y = pos.y;

    var forwardX = x
    var forwardY = y
    var backX = x - cellSize.width
    var backY = y
    var avail

    while (forwardY < plasmoid.height) {
        //look forward
        var forwardAvail = availableSpace(forwardX, forwardY,
                                          Math.max(item.minimumWidth, item.width),
                                          Math.max(item.minimumHeight, item.height))
        //print("checking forward "+item.x+" "+item.y+" "+forwardX/cellSize.width+" "+forwardY/cellSize.height+" "+forwardAvail.width/cellSize.width+" "+forwardAvail.height/cellSize.height)

        //print("response: forwardAvail: "+forwardAvail.width+"x"+forwardAvail.height+" minimumSize: "+item.minimumWidth+"x"+item.minimumHeight+"\n\n")

        if (forwardAvail.width >= Math.max(item.minimumWidth, cellSize.width) &&
            forwardAvail.height >= Math.max(item.minimumHeight, cellSize.height)) {
            x = forwardX
            y = forwardY
            avail = forwardAvail
            break
        }
        forwardX += cellSize.width
        //new line
        if (forwardX+item.width > resultsFlow.width) {
            forwardX = 0
            forwardY += cellSize.height
            //forward positions exausted
            //scrolling now
            /*if (forwardY > resultsFlow.height) {
                break;
            }*/
        }

        //backwards positions exausted
        if (backY < 0) {
            continue
        }

        //look backwards
        var backAvail = availableSpace(backX, backY,
                                       Math.max(item.minimumWidth, item.width),
                                       Math.max(item.minimumHeight, item.height))
        //print("checking backwards "+backX/cellSize.width+" "+backY/cellSize.height+" "+backAvail.width/cellSize.width+" "+backAvail.height/cellSize.height)

        if (backAvail.width >= Math.max(item.minimumWidth, cellSize.width) &&
            backAvail.height >= Math.max(item.minimumHeight, cellSize.height)) {
            x = backX
            y = backY
            avail = backAvail
            break
        }
        backX -= cellSize.width
        if (backX < 0) {
            backX = resultsFlow.width - item.width
            backY -= cellSize.height
        }
    }

    var width = Math.max(cellSize.width*Math.max(1, Math.ceil(item.minimumWidth/cellSize.width)),
                         Math.round(Math.min(avail.width, item.width)/cellSize.width)*cellSize.width)
    var height = Math.max(cellSize.height*Math.max(1, Math.ceil(item.minimumHeight/cellSize.height)),
                          Math.round(Math.min(avail.height, item.height)/cellSize.height)*cellSize.height)

    setSpaceAvailable(x, y, width, height, false)
    //resultsFlow.height = Math.max(resultsFlow.height, y+cellSize.height)

    item.x = x
    item.y = y
    item.width = width
    item.height = height

    saveItem(item);
}

function removeApplet(applet) {
    // Keeps config in sync, deletion of items in the scene
    // are handled in AppletAppearance and main.qml
    delete itemsConfig["Applet-"+applet.id];
    delete itemGroups["Applet-"+applet.id];
}

function saveItem(item) {
    var rect = {
        x: item.x,
        y: item.y,
        width: item.width,
        height: item.height,
        rotation: item.rotation
    };

    //save only things that actually have a category (exclude the placeholder)
    if (item.category) {
        //print("-- Saving rotation : " + rect.rotation);
        itemsConfig[item.category] = rect
    }
}

function saveRotation(item) {
    if (item.category) {
        rotations[item.category] = item.rotation;
    }
}

function restoreRotation(item) {
    if (item.category && rotations[item.category]) {
        item.rotation = rotations[item.category];
    }
}
