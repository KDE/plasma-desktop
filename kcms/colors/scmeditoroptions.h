/* ColorEdit widget for KDE Display color scheme setup module
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
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

#ifndef __SCMEDITOROPTIONS_H__
#define __SCMEDITOROPTIONS_H__

#include <KColorScheme>
#include <KSharedConfig>

#include <QFrame>
#include <QPalette>
#include <QWidget>

#include "ui_scmeditoroptions.h"

class SchemeEditorOptions : public QWidget, public Ui::ScmEditorOptions
{
    Q_OBJECT

public:
    SchemeEditorOptions(KSharedConfigPtr config, QWidget *parent = nullptr);
    void updateValues();

Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:

    // options slots
    void on_contrastSlider_valueChanged(int value);
    void on_shadeSortedColumn_stateChanged(int state);
    void on_inactiveSelectionEffect_stateChanged(int state);
    void on_useInactiveEffects_stateChanged(int state);

private:

    /** load options from global */
    void loadOptions();
    void setCommonForeground(KColorScheme::ForegroundRole role,
                             int stackIndex,
                             int buttonIndex);
    void setCommonDecoration(KColorScheme::DecorationRole role,
                             int stackIndex,
                             int buttonIndex);

    KSharedConfigPtr m_config;
    bool m_disableUpdates;

};

#endif
