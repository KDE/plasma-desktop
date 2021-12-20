#! /usr/bin/awk -f

# - Input:
# Options=foo
#
# - Output:
# ResetOldOptions=true

BEGIN { FS = "=" }

$1 == "Options" && $2 != "" {
	print "ResetOldOptions=true"
}
