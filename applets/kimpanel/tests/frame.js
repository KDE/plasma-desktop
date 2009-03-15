function do_frame_layout(width,height) 
{
    topleft.width = left.width;
    topleft.height = top.height;
    topright.width = right.width;
    topright.height = top.height;
    bottomleft.width = left.width;
    bottomleft.height = bottom.height;
    bottomright.width = right.width;
    bottomright.height = bottom.height; 

    left.height = height - topleft.height - bottomleft.height;
    right.height = height - topright.height - bottomright.height;

    top.width = width - topleft.width - topright.width;
    bottom.width = width - bottomleft.width - bottomright.width;

    center.width = width - left.width - right.width;
    center.height = height - top.height - bottom.height;

    topleft.x = 0;
    topleft.y = 0;
    top.x = topleft.width;
    top.y = 0;
    topright.x = top.x + top.width;
    topright.y = 0;

    left.x = 0;
    left.y = topleft.height;
    center.x = left.width;
    center.y = top.height;
    right.x = center.x + center.width;
    right.y = topright.height;

    bottomleft.x = 0;
    bottomleft.y = left.y + left.height;
    bottom.x = bottomleft.width;
    bottom.y = center.y + center.height;
    bottomright.x = bottom.x + bottom.width;
    bottomright.y = right.y + right.height;
}

function do_layout(width,height,elem)
{
    root = Svg.Element("root");
    left = Svg.Element("left");
    top = Svg.Element("top");
    right = Svg.Element("right");
    bottom = Svg.Element("bottom");
    topleft = Svg.Element("topleft");
    topright = Svg.Element("topright");
    bottomleft = Svg.Element("bottomleft");
    bottomright = Svg.Element("bottomright");
    center = Svg.Element("center");
    cat = Svg.Element("cat");

    if (elem == "root") {
        root.width = width;
        root.height = height;
        do_frame_layout(width,height);
        cat.x = root.width-cat.width;
        cat.y = 0;
	cat.layer = 1;
    } else if (elem == "center") {
        root.width = width + left.width + right.width;
        root.height = height + top.height + bottom.height;
        do_frame_layout(root.width,root.height);
        cat.x = root.width-cat.width;
        cat.y = 0;
	cat.layer = 1;
    }
}

