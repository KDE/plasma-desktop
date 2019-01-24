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

#ifndef __SCMEDITOREFFECTS_H__
#define __SCMEDITOREFFECTS_H__

#include <KColorScheme>
#include <KSharedConfig>

#include <QFrame>
#include <QPalette>
#include <QWidget>

#include "ui_scmeditoreffects.h"


class SchemeEditorEffects : public QWidget, public Ui::ScmEditorEffects
{
    Q_OBJECT

public:
    SchemeEditorEffects(const KSharedConfigPtr &config, QPalette::ColorGroup palette, QWidget *parent = nullptr);
    void updateValues();
    void updateFromEffectsPage();

Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:

    void on_intensityBox_currentIndexChanged(int index);

    void on_intensitySlider_valueChanged(int value);

    void on_colorBox_currentIndexChanged(int index);

    void on_colorSlider_valueChanged(int value);

    void on_colorButton_changed(const QColor& color);

    void on_contrastBox_currentIndexChanged(int index);

    void on_contrastSlider_valueChanged(int value);

private:
    QPalette::ColorGroup m_palette;
    KSharedConfigPtr m_config;
    bool m_disableUpdates;

};

#endif
