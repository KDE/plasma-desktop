#! /usr/bin/awk -f

# - Input:
# DisplayNames=\\0
# VariantList=\\0
# Options=\\0
#
# - Output:
# # DELETE DisplayNames
# # DELETE VariantList
# # DELETE Options

BEGIN { FS = "=" }

$2 == "\\\\0" && ($1 == "DisplayNames" || $1 == "VariantList" || $1 == "Options") {
	print "# DELETE", $1;
}
