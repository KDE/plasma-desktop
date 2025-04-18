// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>

#include <qqmlregistration.h>

#include "flags.h"
#include "layoutsearchmodel.h"

struct FlagsForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(Flags)
    QML_SINGLETON
    QML_FOREIGN(Flags)
};

struct LayoutSearchModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(LayoutSearchModel)
    QML_FOREIGN(LayoutSearchModel)
};
