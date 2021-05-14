#!/usr/bin/env python3
#
#  SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
#
#  SPDX-License-Identifier: LGPL-2.0-or-later
#

# 5.21.0 botched the migration by accidentally turning enabled to disabled and disabled to enabled.
# We have two choices here: we can either break key repeat for users that manually intervened
# and enabled key repeat by inverting the value, or we can enable key repeat for users that didn't
# have it enabled and fix users that had their key repeat disabled. Only one of those sentences
# contains the word "break" so we go with the one without the word "break" ;)
# Enabling key repeat for users who disabled it is less breaking than disabling key repeat for
# users who enabled it.

#
# And that brings us to the code.
#
# We'll need to import sys in order to read stdin, which is how we receive
# the old input file to look for it.
#
import sys

#
# After that, we'll need to declare ourselves a variable to hold the contents
# of the config file we read in from stdin.
#

content: str = sys.stdin.read()

#
# That brings us to checking for the presence of the old script's effects on the underlying system:
# kconfig files are annotated with a section containing all of the config scripts that have ran
# on them.
#
# They're of the form filename.upd:migration_name.
#
if "kcminputrc_repeat.upd:kcminputrc_migrate_repeat_value" in content:
    #
    # Now we check for the presence of a disabled key repeat in the config file contents. For 5.21.0, this
    # was changed from a 0-1-2 ternary to an English-language nothing/repeat/accent.
    # The config script for migrating ternary to English-language is why we're in this
    # mess in the first place. We migrated 0 (enabled) -> nothing and 2 (disabled) -> repeat.
    #
    if "KeyRepeat=nothing" in content:
        #
        # Now we print the command instructing the kconfig updater to delete this field.
        # A deleted field essentially means "use the default" which in this case means
        # "enable key repeat." We delete instead of reassigning to "repeat" because
        # we would prefer to have a deleted field act as our "default" value.
        #
        print("# DELETE [Keyboard]KeyRepeat")

    #
    # Since we created the Tmp group as a temporary way of conveying to this script what migrations
    # were applied to the config file, we want to clean it up, as it serves no other purpose.
    #
    # We instruct KConfig to delete the Tmp group in the config file.
    #
    print("# DELETEGROUP Tmp")
