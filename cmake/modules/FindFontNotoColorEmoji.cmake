# SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

find_file(noto_font share/fonts/google-noto-color-emoji-fonts/Noto-COLRv1.ttf)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FontNotoColorEmoji
    FOUND_VAR
        FontNotoColorEmoji_FOUND
    REQUIRED_VARS
        noto_font
)

unset(noto_font)
