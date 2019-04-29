/* KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "scmeditoroptions.h"

#include <QDebug>
#include <KConfigGroup>

SchemeEditorOptions::SchemeEditorOptions(KSharedConfigPtr config, QWidget *parent)
    : QWidget( parent )
    , m_config( config )
{
    setupUi(this);
    m_disableUpdates = false;
    loadOptions();
}

void SchemeEditorOptions::updateValues()
{
    loadOptions();
}

void SchemeEditorOptions::loadOptions()
{
    KConfigGroup generalGroup(KSharedConfig::openConfig(), "General");
    shadeSortedColumn->setChecked(generalGroup.readEntry("shadeSortColumn", true));

    KConfigGroup KDEgroup(m_config, "KDE");
    contrastSlider->setValue(KDEgroup.readEntry("contrast", KColorScheme::contrast()));

    KConfigGroup group(m_config, "ColorEffects:Inactive");
    useInactiveEffects->setChecked(group.readEntry("Enable", false));

    // NOTE: keep this in sync with kdelibs/kdeui/colors/kcolorscheme.cpp
    // NOTE: remove extra logic from updateFromOptions and on_useInactiveEffects_stateChanged when this changes!
    inactiveSelectionEffect->setChecked(group.readEntry("ChangeSelectionColor", group.readEntry("Enable", true)));
}

// Option slot
void SchemeEditorOptions::on_contrastSlider_valueChanged(int value)
{
    KConfigGroup group(m_config, "KDE");
    group.writeEntry("contrast", value);

    emit changed(true);
}

void SchemeEditorOptions::on_shadeSortedColumn_stateChanged(int state)
{
    if (m_disableUpdates)
        return;
    KConfigGroup group(m_config, "General");
    group.writeEntry("shadeSortColumn", bool(state != Qt::Unchecked));

    emit changed(true);
}

void SchemeEditorOptions::on_useInactiveEffects_stateChanged(int state)
{
    KConfigGroup group(m_config, "ColorEffects:Inactive");
    group.writeEntry("Enable", bool(state != Qt::Unchecked));

    m_disableUpdates = true;
    inactiveSelectionEffect->setChecked(group.readEntry("ChangeSelectionColor", bool(state != Qt::Unchecked)));
    m_disableUpdates = false;

    emit changed(true);
}

void SchemeEditorOptions::on_inactiveSelectionEffect_stateChanged(int state)
{
    if (m_disableUpdates)
    {
        // don't write the config as we are reading it!
        return;
    }

    KConfigGroup group(m_config, "ColorEffects:Inactive");
    group.writeEntry("ChangeSelectionColor", bool(state != Qt::Unchecked));

    emit changed(true);
}
