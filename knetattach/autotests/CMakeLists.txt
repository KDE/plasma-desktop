# SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

set(knetattachtest_SRCS knetattachtest.cpp ../knetattach.cpp)
ki18n_wrap_ui(knetattachtest_SRCS ../knetattach.ui)

ecm_add_test(${knetattachtest_SRCS}
    TEST_NAME knetattachtest
    LINK_LIBRARIES Qt::Test KF6::KIOWidgets KF6::I18n KF6::Codecs)
target_include_directories(knetattachtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
