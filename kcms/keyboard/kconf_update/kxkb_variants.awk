#! /usr/bin/awk -f

# - Input:
# LayoutList=us,us(intl),ru,gb(dvorak),gb
#
# - Output:
# LayoutList=us,us,ru,gb,gb
# VariantList=,intl,,dvorak
# # DELETE LayoutList

BEGIN { OFS = FS = "," }

/^LayoutList=/ {
	layouts = $0
	gsub(/\([^()]*\)/, "", layouts)

	for (f = 1; f <= NF; ++f) {
		match($f, /\((.*)\)/, variant)
		$f = variant[1]
	}
	sub(/,*$/, "")

	print layouts
	if (length())
		print "VariantList=" $0
	print "# DELETE LayoutList"
}
