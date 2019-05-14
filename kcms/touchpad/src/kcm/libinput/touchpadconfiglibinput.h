/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
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

#ifndef TOUCHPADCONFIGLIBINPUT_H
#define TOUCHPADCONFIGLIBINPUT_H

#include "../touchpadconfigplugin.h"

class TouchpadBackend;
class QHideEvent;
class QQuickWidget;
class KMessageWidget;

class TouchpadConfigLibinput : public TouchpadConfigPlugin
{
    Q_OBJECT

public:
    explicit TouchpadConfigLibinput(TouchpadConfigContainer *parent,
                                    TouchpadBackend *backend,
                            const QVariantList &args = QVariantList());
    virtual ~TouchpadConfigLibinput() {}

    void load() override;
    void save() override;
    void defaults() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void hideEvent(QHideEvent *) override {}

private Q_SLOTS:
    void onChange();
    void onTouchpadAdded(bool success);
    void onTouchpadRemoved(int index);

private:
    void hideErrorMessage();

    QQuickWidget *m_view;
    KMessageWidget *m_errorMessage;

    bool m_initError;
};

#endif // TOUCHPADCONFIGLIBINPUT_H
