/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls

Canvas {
    property QtObject shape
    property variant strokeStyle: "yellow"
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
    onStrokeStyleChanged: requestPaint()
    onFillStyleChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        ctx.lineWidth = lineWidth
        ctx.strokeStyle = strokeStyle
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

        // Account for offset bounds (positive or negative!)
        ctx.translate(-shape.bounds.x, -shape.bounds.y)

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

                // roundedRect() takes position and size
                ctx.roundedRect(topLeftPoint.x, topLeftPoint.y,
                                bottomRightPoint.x - topLeftPoint.x, bottomRightPoint.y - topLeftPoint.y,
                                outline.corner_radius, outline.corner_radius)
            } else { // polygon point to point draw (used for doodads and non-rect keys such as Enter)
                // Move to the start of the first arc
                const firstPoint = outline.points[0]
                const lastPoint = outline.points[outline.points.length - 1]
                // Increase the distance by one to avoid artifacts
                const arcStart = substract(firstPoint, direction(lastPoint, firstPoint, outline.corner_radius + 1))

                ctx.moveTo(arcStart.x, arcStart.y)

                for (let j = 0; j < outline.points.length; j++) {
                    const point = outline.points[j]  // P
                    const nextPoint = outline.points[(j + 1) % outline.points.length]  // N
                    const arcEnd = substract(point, direction(nextPoint, point, outline.corner_radius));  // P - radius · dir(N -> P)

                    // arcTo() first draws a straight line from the current point to the start of the arc, so no need for `lineTo()`
                    ctx.arcTo(point.x, point.y,
                              arcEnd.x, arcEnd.y,
                              outline.corner_radius)
                }
            }
        }

        ctx.closePath()
        if (fillStyle) {
            ctx.fill()
        }
        ctx.stroke()
    }

    // Vector functions to operate with QPoints
    function substract(P: point, Q : point) : point {
        return Qt.point(P.x - Q.x, P.y - Q.y)
    }

    function multiply(P : point, factor : real) : point {
        return Qt.point(P.x * factor, P.y * factor)
    }

    // A scaled vector (modulus `scale`) in the direction P -> Q
    // Ex. if the points are in the same horizontal, Q after P, it will return (scale, 0)
    function direction(P : point, Q : point, scale : real) : point {
        const vector = substract(Q, P);
        const modulus = Math.sqrt(vector.x * vector.x + vector.y * vector.y);
        const scaled = multiply(vector, scale/modulus);
        return scaled;
    }
}
