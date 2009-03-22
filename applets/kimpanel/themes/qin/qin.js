hbox_splitter = 50;
hbox_rightRel = 116;

lookuptable_leftMargin = 24;
lookuptable_topMargin = 20;
lookuptable_rightMargin = hbox_rightRel;
lookuptable_bottomMargin = 2;

function do_horizontal_layout(width,height) 
{
    hboxcenter.x = hboxleft.width;
    hboxright.x = root.width - hboxright.width;
    hboxcenter.y = 19; 
    hboxcenter.width = hboxright.x - hboxcenter.x;
}

function calc_lookuptable_root()
{
    root.width = hboxleft.width + hboxright.width;
    if (root.width < lookuptable_leftMargin + lookuptable_area.width + lookuptable_rightMargin) {
        root.width = lookuptable_leftMargin + lookuptable_area.width + lookuptable_rightMargin;
    }
    root.height = hboxright.height;
}

function map_lookuptable_content()
{
    aux_area.x = lookuptable_leftMargin;
    aux_area.y = hbox_splitter - aux_area.height - 2;

    preedit_area.x = aux_area.x + aux_area.width;
    preedit_area.y = aux_area.y;

    pageup_area.x = root.width - hbox_rightRel - pageup_area.width - pagedown_area.width;
    pageup_area.y = aux_area.y;
    
    pagedown_area.x = pageup_area.x + pageup_area.width;
    pagedown_area.y = aux_area.y;

    lookuptable_area.x = lookuptable_leftMargin;
    lookuptable_area.y = hbox_splitter + 2;
}

function do_layout(_width,_height,elem)
{
    root = Svg.Element("root");

    // fake element as place holder
    statusbar = Svg.Element("statusbar");
    aux_area = Svg.Element("aux_area");
    preedit_area = Svg.Element("preedit_area");
    pageup_area = Svg.Element("pageup_area");
    pagedown_area = Svg.Element("pagedown_area");
    lookuptable_area = Svg.Element("lookuptable_area");

    if (elem == "statusbar") {
        hbar = Svg.Element("hbar");
        //vbar = Svg.Element("vbar");

        if (_width < 16) {
            _width = 16;
        }
        if (_height < 16) {
            _height = 16;
        }

        root.width = hbar.width;
        root.height = hbar.height;

        statusbar.x = 12;
        statusbar.y = 84;
        statusbar.width = 120;
        statusbar.height = 36;

        return;
    }


    hboxleft = Svg.Element("hboxleft");
    hboxcenter = Svg.Element("hboxcenter");
    hboxright = Svg.Element("hboxright");
    //vbox = Svg.Element("vbox");

    if (elem == "aux_area") {
        aux_area.width = _width;
        aux_area.height = _height;

        calc_lookuptable_root();
        do_horizontal_layout(root.width,root.height);
        map_lookuptable_content();

    }
    if (elem == "preedit_area") {
        preedit_area.width = _width;
        preedit_area.height = _height;

        calc_lookuptable_root();
        do_horizontal_layout(root.width,root.height);
        map_lookuptable_content();
    }

    if (elem == "pageup_area") {
        pageup_area.width = _width;
        pageup_area.height = _height;

        calc_lookuptable_root();
        do_horizontal_layout(root.width,root.height);
        map_lookuptable_content();
    }
    if (elem == "pagedown_area") {
        pagedown_area.width = _width;
        pagedown_area.height = _height;

        calc_lookuptable_root();
        do_horizontal_layout(root.width,root.height);
        map_lookuptable_content();
    }

    if (elem == "lookuptable_area") {
        lookuptable_area.width = _width;
        lookuptable_area.height = _height;

        calc_lookuptable_root();
        do_horizontal_layout(root.width,root.height);
        map_lookuptable_content();
    }
}

