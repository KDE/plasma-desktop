#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Guillermo Steren <gstkein@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# kconf update script to migrate users who previously disabled
# activateWhenTypingOnDesktop for KRunner. This script only flips
# users who explicitly set the key to "false" to "true" and
# leaves unset keys untouched.

# Minimal behavior:
# - Locate the user config file (prefer $HOME/.config/krunnerrc)
# - If the file exists and contains the key ActivateWhenTypingOnDesktop,
#   set that key to "true" (make a backup first). Do not create the key.


# Locate config file
if [ -f "$HOME/.config/krunnerrc" ]; then
    CONFIG_FILE="$HOME/.config/krunnerrc"
elif [ -n "${XDG_CONFIG_HOME:-}" ] && [ -f "${XDG_CONFIG_HOME}/krunnerrc" ]; then
    CONFIG_FILE="${XDG_CONFIG_HOME}/krunnerrc"
else
    # No existing user config found; nothing to do.
    exit 0
fi

# Only modify if the key exists in the file
if grep -q '^[[:space:]]*ActivateWhenTypingOnDesktop[[:space:]]*=' "$CONFIG_FILE"; then
    # Read the current value so we can react if it was explicitly "false"
    OLDVAL=$(awk -F= '/^[[:space:]]*ActivateWhenTypingOnDesktop[[:space:]]*=/{gsub(/^[[:space:]]+|[[:space:]]+$/,"",$2); print $2; exit}' "$CONFIG_FILE")

    cp -a "$CONFIG_FILE" "$CONFIG_FILE".bak 2>/dev/null || true
    # Remove the key entirely from the user's config (migrate to default behavior).
    sed -i '/^[[:space:]]*ActivateWhenTypingOnDesktop[[:space:]]*=.*/d' "$CONFIG_FILE"

    # If the user explicitly disabled activateWhenTypingOnDesktop, also enable
    # the containment/plasmoid option `useTypeAhead` where present.
    if [ "${OLDVAL:-}" = "false" ]; then
        SEARCH_DIR="${XDG_CONFIG_HOME:-$HOME/.config}"

        # Prefer the standard applets file in $HOME/.config if present
        candidates=()
        if [ -f "$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc" ]; then
            candidates+=("$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc")
        fi
        if [ -n "${XDG_CONFIG_HOME:-}" ] && [ -f "${XDG_CONFIG_HOME}/plasma-org.kde.plasma.desktop-appletsrc" ]; then
            candidates+=("${XDG_CONFIG_HOME}/plasma-org.kde.plasma.desktop-appletsrc")
        fi

        # If none of the standard locations exist, search the config dir for matching files
        if [ ${#candidates[@]} -eq 0 ]; then
            while IFS= read -r -d '' f; do
                candidates+=("$f")
            done < <(find "$SEARCH_DIR" -type f -name 'plasma-org.kde.plasma.desktop-appletsrc*' -print0 2>/dev/null)
        fi

        for f in "${candidates[@]:-}"
        do
            if [ -z "$f" ] || [ ! -f "$f" ]; then
                continue
            fi
            if grep -q '^[[:space:]]*useTypeAhead[[:space:]]*=' "$f" 2>/dev/null; then
                cp -a "$f" "$f".bak 2>/dev/null || true
                # Set any existing useTypeAhead assignments to true
                sed -i 's/^[[:space:]]*useTypeAhead[[:space:]]*=.*/useTypeAhead=true/g' "$f"
            fi
        done
    fi
fi

exit 0
