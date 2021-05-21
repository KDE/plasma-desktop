/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TOUCHPADCONFIGLIBINPUT_H
#define TOUCHPADCONFIGLIBINPUT_H

#include "../touchpadconfigplugin.h"

class TouchpadBackend;
class QHideEvent;
class QQuickWidget;

class TouchpadConfigLibinput : public TouchpadConfigPlugin
{
    Q_OBJECT

public:
    explicit TouchpadConfigLibinput(TouchpadConfigContainer *parent, TouchpadBackend *backend, const QVariantList &args = QVariantList());
    virtual ~TouchpadConfigLibinput()
    {
    }

    void load() override;
    void save() override;
    void defaults() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void hideEvent(QHideEvent *) override
    {
    }

Q_SIGNALS:
    void showMessage(const QString message, int type = 3 /*Kirigami.MessageType.Error*/);

private Q_SLOTS:
    void onChange();
    void onTouchpadAdded(bool success);
    void onTouchpadRemoved(int index);

private:
    void hideErrorMessage();

    QQuickWidget *m_view;

    bool m_initError;
};

#endif // TOUCHPADCONFIGLIBINPUT_H
