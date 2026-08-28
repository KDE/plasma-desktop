# SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

kcmutils_add_qml_kcm(kcm_spellchecking SOURCES
   spellchecking.cpp
   spellcheckingdata.cpp
)

target_link_libraries(kcm_spellchecking
   KF6::KCMUtils
   KF6::SonnetCore
   KF6::SonnetUi
)
