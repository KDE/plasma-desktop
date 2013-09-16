/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOUCHPADCONFIG_H
#define TOUCHPADCONFIG_H

#include <KCModule>
#include <KConfigDialogManager>
#include <QScopedPointer>

#include "touchpadparameters.h"

#include "ui_pointermotion.h"
#include "ui_tap.h"
#include "ui_scroll.h"
#include "ui_sensitivity.h"

class TouchpadBackend;
class KMessageWidget;

class TouchpadConfig : public KCModule
{
    Q_OBJECT

public:
    explicit TouchpadConfig(QWidget *parent,
                            const QVariantList &args = QVariantList());

    virtual void save();
    virtual void load();

protected:
    virtual void showEvent(QShowEvent *ev);
    virtual void hideEvent(QHideEvent *);

private Q_SLOTS:
    void beginTesting();
    void endTesting();

private:
    bool compareConfigs(const TouchpadParameters &a,
                        const TouchpadParameters &b) const;

    TouchpadBackend *m_backend;
    TouchpadParameters m_config;
    QScopedPointer<TouchpadParameters> m_prevConfig;
    KConfigDialogManager *m_manager;
    bool m_configOutOfSync;

    KMessageWidget *m_errorMessage, *m_differentConfigsMessage;

    Ui::PointerMotionForm m_pointerMotion;
    Ui::TapForm m_tapping;
    Ui::ScrollForm m_scrolling;
    Ui::SensitivityForm m_sensitivity;
};

#endif // TOUCHPADCONFIG_H
