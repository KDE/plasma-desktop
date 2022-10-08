// ------------------------------
// The headed name is a noun followed by attributes.
// There are two formats:
//   "head, attr1 attr2..." (basic)
//   "head (attr11 attr12 ..., attr21 attr22 ..., ...)" (compound)
function headed_name_to_case (gcase, composed)
{
    var p = composed.lastIndexOf("("); // in case of (...) in the head
    if (p < 0) {
        // The basic format.
        // Assume there may be no comma or attributes, so check thoroughly.
        p = composed.lastIndexOf(","); // in case of comma in the head
        if (p < 0) {
            return getprop(composed, gcase);
        }
        var head = composed.substr(0, p);
        var final = getprop(head, gcase);
        var attrs = composed.substr(p + 1).split(" ").filter(Boolean);
        if (attrs.length > 0) {
            var gender = getprop(head, "_род");
            var number = ""
            if (hasprop(head, "_број")) {
                number = getprop(head, "_број");
            }
            for (var i = 0; i < attrs.length; i += 1) {
                attrs[i] = getprop(attrs[i], gcase + "-" + gender + number);
            }
            final += ", " + attrs.join(" ");
        }
        return final;
    }
    else {
        // The compound format.
        // Assume valid, skip checks.
        var head = composed.substr(0, p);
        var pp = composed.indexOf(")", p);
        var attrs = composed.substring(p + 1, pp).split(",")
        var gender = getprop(head, "_род");
        var number = ""
        if (hasprop(head, "_број")) {
            number = getprop(head, "_број");
        }
        for (var i = 0; i < attrs.length; i += 1) {
            var subattrs = attrs[i].split(" ").filter(Boolean);
            for (var j = 0; j < subattrs.length; j += 1) {
                subattrs[j] = getprop(subattrs[j], gcase + "-" + gender + number);
            }
            attrs[i] = subattrs.join(" ");
        }
        return getprop(head, gcase) + " (" + attrs.join(", ") + ")";
    }
}
Ts.setcall("главорепи", headed_name_to_case);
