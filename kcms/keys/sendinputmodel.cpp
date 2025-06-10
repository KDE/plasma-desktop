/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sendinputmodel.h"

#include <QCollator>
#include <QMetaEnum>

#include <KConfigBase>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardShortcut>

#include "basemodel.h"
#include "kcmkeys_debug.h"

SendInputModel::SendInputModel(QObject *parent)
    : BaseModel(parent)
{
}

void SendInputModel::load()
{
    beginResetModel();
    m_components.clear();
    const ComponentType inputSectionName = ComponentType::SendInput;
    m_components.append(Component{"SendInput", i18n("Generate Key Presses"), inputSectionName, "preferences-desktop-keyboard-shortcut", {}, false, false});
    m_components.last().actions.append(Action{"GenerateKeySequence", i18n("Key Sequence"), {}, {}, {}});
    m_components.last().actions.append(Action{"GenerateTextSnippet", i18n("Text Snippet"), {}, {}, {}});
    endResetModel();
}

void SendInputModel::exportToConfig(KConfigBase &config)
{
}

void SendInputModel::importConfig(const KConfigBase &config)
{
}

void SendInputModel::save()
{
}

#include "moc_sendinputmodel.cpp"
