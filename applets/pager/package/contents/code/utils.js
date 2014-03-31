.pragma library

function contains(item, point) {
    return point.x >= item.x && point.x <= item.x + item.width
           && point.y >= item.y && point.y <= item.y + item.height;
}
