/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "kcmaccessibilitybell.h"
#include "kcmaccessibilityscreenreader.h"

#include <QAbstractNativeEventFilter>
#include <QAction>

#include <X11/Xlib.h>
#define explicit int_explicit // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#include <xcb/xkb.h>
#undef explicit
#include <fixx11h.h>

class KAccessApp : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit KAccessApp();

    void newInstance();
    void setXkbOpcode(int opcode);
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

protected:
    void readSettings();

    void xkbBellNotify(xcb_xkb_bell_notify_event_t *event);

private Q_SLOTS:
    void toggleScreenReader();

private:
    void setScreenReaderEnabled(bool enabled);

    BellSettings m_bellSettings;
    ScreenReaderSettings m_screenReaderSettings;
    KConfig m_kdeglobals;

    int xkb_opcode;

    QAction *toggleScreenReaderAction;
};
