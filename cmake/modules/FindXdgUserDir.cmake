# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Méven Car meven.car@enioka.com

find_program(XdgUserDir_EXEC xdg-user-dir)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XdgUserDir
    FOUND_VAR
        XdgUserDir_FOUND
    REQUIRED_VARS
        XdgUserDir_EXEC
)
