// Plasma's FrameSvg compatible layout


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

function calc_lookuptable_root()
{
    // show pageup/down icon in lookuptable area when no preedit text and auxilary text;
    if (aux_area.width + preedit_area.width < 1) {
        root.width = left.width + right.width + lookuptable_area.width + pageup_area.width + pagedown_area.width;

        max_content_height = 0;
        if (max_content_height < pageup_area.height) {
            max_content_height = pageup_area.height;
        }
        if (max_content_height < pagedown_area.height) {
            max_content_height = pagedown_area.height;
        }
        if (max_content_height < lookuptable_area.height) {
            max_content_height = lookuptable_area.height;
        }
        root.height = top.height + bottom.height + max_content_height;
    } else {
        max_content_width = aux_area.width + preedit_area.width + pageup_area.width + pagedown_area.width;
        if (max_content_width < lookuptable_area.width) {
            max_content_width = lookuptable_area.width;
        }
        root.width = left.width + right.width + max_content_width;

        max_content_height_upper = 0;
        max_content_height_lower = 0;
        // assume pageup_area.height == pagedown_area.height
        if (max_content_height_upper < aux_area.height) {
            max_content_height_upper = aux_area.height;
        }
        if (max_content_height_upper < aux_area.height) {
            max_content_height_upper = aux_area.height;
        }
        if (max_content_height_upper < pageup_area.height) {
            max_content_height_upper = pageup_area.height;
        }
        if (max_content_height_upper < pagedown_area.height) {
            max_content_height_upper = pagedown_area.height;
        }
        max_content_height_lower = lookuptable_area.height;
        root.height = top.height + bottom.height + max_content_height_upper + max_content_height_lower;
    }
}

function map_lookuptable_content()
{
    // show pageup/down icon in lookuptable area when no preedit text and auxilary text;
    if (aux_area.width + preedit_area.width < 1) {
        lookuptable_area.x = left.width;
        lookuptable_area.y = top.height;
        pageup_area.x = lookuptable_area.x + lookuptable_area.width;
        pageup_area.y = lookuptable_area.y;
        pagedown_area.x = pageup_area.x + pageup_area.width;
        pagedown_area.y = pageup_area.y;
    } else {
        aux_area.x = left.width;
        aux_area.y = top.height;
        preedit_area.x = aux_area.x + aux_area.width;
        preedit_area.y = aux_area.y;
        lookuptable_area.x = left.width;
        lookuptable_area.y = aux_area.y + aux_area.height;
        pagedown_area.x = right.x - pagedown_area.width;
        pagedown_area.y = preedit_area.y;
        pageup_area.x = pagedown_area.x - pageup_area.width;
        pageup_area.y = preedit_area.y;
    }
}

function do_layout(_width,_height,elem)
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

// fake element as place holder
    statusbar = Svg.Element("statusbar");
    aux_area = Svg.Element("aux_area");
    preedit_area = Svg.Element("preedit_area");
    pageup_area = Svg.Element("pageup_area");
    pagedown_area = Svg.Element("pagedown_area");
    lookuptable_area = Svg.Element("lookuptable_area");

    if (elem == "statusbar") {
        if (_width < 16) {
            _width = 16;
        }
        if (_height < 16) {
            _height = 16;
        }
        root.width = _width + left.width + right.width;
        root.height = _height + top.height + bottom.height;
        do_frame_layout(root.width,root.height);

        statusbar.x = center.x;
        statusbar.y = center.y;
        statusbar.width = center.width;
        statusbar.height = center.height;
    }

    if (elem == "aux_area") {
        aux_area.width = _width;
        aux_area.height = _height;

        calc_lookuptable_root();
        do_frame_layout(root.width,root.height);
        map_lookuptable_content();

    }
    if (elem == "preedit_area") {
        preedit_area.width = _width;
        preedit_area.height = _height;

        calc_lookuptable_root();
        do_frame_layout(root.width,root.height);
        map_lookuptable_content();
    }

    if (elem == "pageup_area") {
        pageup_area.width = _width;
        pageup_area.height = _height;

        calc_lookuptable_root();
        do_frame_layout(root.width,root.height);
        map_lookuptable_content();
    }
    if (elem == "pagedown_area") {
        pagedown_area.width = _width;
        pagedown_area.height = _height;

        calc_lookuptable_root();
        do_frame_layout(root.width,root.height);
        map_lookuptable_content();
    }

    if (elem == "lookuptable_area") {
        lookuptable_area.width = _width;
        lookuptable_area.height = _height;

        calc_lookuptable_root();
        do_frame_layout(root.width,root.height);
        map_lookuptable_content();
    }
}

