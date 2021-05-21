/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQuick.Controls 2.5

Canvas {
    property QtObject shape
    property variant strokeSyle: "yellow"
    property variant fillStyle: "steelblue"
    // Only draw one outline. Outlines are tricky because xkb has no
    // concept of displaying text on top of the shapes. IOW: it
    // only thinks about blank keys. This means we have no layout
    // constraints for our text and as a result we'd have to figure out
    // where to put text so it doesn't conflict with any of the drawn
    // paths and is inside most/all of them. It's not worth the work...
    property int maxOutline: 1
    // use a round number, we divide this by two to balance spacing
    property real lineWidth: 4.0
    // scoot away from 0,0 to translation,translation
    property real translation: lineWidth / 2.0
    // reduce the scale to account for the scoot of translation
    property real scale: Math.min((width - lineWidth) / width, (height - lineWidth) / height)

    id: canvas
    width: shape.bounds.width
    height: shape.bounds.height
    onStrokeSyleChanged: requestPaint()
    onFillStyleChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        ctx.lineWidth = lineWidth
        ctx.strokeStyle = strokeSyle
        ctx.fillStyle = fillStyle
        // Transform the context a bunch. The way strokes work is that
        // they extend to the left and right of the path, so with a
        // lineWidth of 8 we'll need the entire scene scooted so
        // 0,0 becomes 4,4. Since that also would screw up our absolute
        // coordinates towards the right (e.g. our width bounds as reported
        // by XKB) we also need to then adjust the scale so the entire
        // coordinate system so the scene leaves sufficient space at all
        // borders.
        // i.e. we move the scene so the top and right strokes are fully
        // visible and then shrink it so the left and bottom are fully visible.
        ctx.translate(translation, translation)
        ctx.scale(scale, scale)

        ctx.beginPath()

        for (var i in shape.outlines) {
            if (maxOutline && i >= maxOutline) {
                break;
            }

            var outline = shape.outlines[i]

            if (outline.points.length === 1) { // rect from 0,0 to point
                var point = outline.points[0]

                ctx.roundedRect(0, 0,
                                point.x, point.y,
                                outline.corner_radius, outline.corner_radius)
            } else if (outline.points.length === 2) { // rect from p1 to p2
                var topLeftPoint = outline.points[0]
                var bottomRightPoint = outline.points[1]

                ctx.roundedRect(topLeftPoint.x, topLeftPoint.y,
                                bottomRightPoint.x, bottomRightPoint.y,
                                outline.corner_radius, outline.corner_radius)
            } else { // polygon point to point draw (used for doodads and non-rect keys such as Enter)
                for (var j in outline.points) {
                    var anyPoint = outline.points[j]
                    if (j < 1) { // move to first point
                        ctx.moveTo(anyPoint.x, anyPoint.y)
                    } else { // from there we draw point to point
                        ctx.lineTo(anyPoint.x, anyPoint.y)
                    }

                    // TODO: should probably draw short of target and then arcTo over the target so we get a round corner?
                    // Currently shapes are not rounded.
                }
            }
        }

        ctx.closePath()
        if (fillStyle) {
            ctx.fill()
        }
        ctx.stroke()
    }
}
