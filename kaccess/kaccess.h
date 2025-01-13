/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "kcmaccessibilityactivationgestures.h"
#include "kcmaccessibilitybell.h"
#include "kcmaccessibilitykeyboard.h"
#include "kcmaccessibilitykeyboardfilters.h"
#include "kcmaccessibilitymouse.h"
#include "kcmaccessibilityscreenreader.h"

#include <QAbstractNativeEventFilter>
#include <QColor>
#include <QLabel>
#include <QPaintEvent>
#include <QWidget>

#include <X11/Xlib.h>
#define explicit int_explicit // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#include <xcb/xkb.h>
#undef explicit
#include <fixx11h.h>

class KComboBox;

class KAccessApp : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit KAccessApp();
    ~KAccessApp();

    void newInstance();
    void setXkbOpcode(int opcode);
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

    bool isFailed() const
    {
        return m_error;
    }

protected:
    void readSettings();

    void xkbStateNotify();
    void xkbBellNotify(xcb_xkb_bell_notify_event_t *event);
    void xkbControlsNotify(xcb_xkb_controls_notify_event_t *event);

private Q_SLOTS:

    void notifyChanges();
    void applyChanges();
    void yesClicked();
    void noClicked();
    void dialogClosed();
    void toggleScreenReader();

private:
    void createDialogContents();
    void initMasks();
    void setScreenReaderEnabled(bool enabled);

    BellSettings m_bellSettings;
    KeyboardSettings m_keyboardSettings;
    KeyboardFiltersSettings m_keyboardFiltersSettings;
    MouseSettings m_mouseSettings;
    ScreenReaderSettings m_screenReaderSettings;
    ActivationGesturesSettings m_activationGesturesSettings;
    KConfig m_kdeglobals;

    int xkb_opcode;
    unsigned int features;
    unsigned int requestedFeatures;

    QDialog *dialog;
    QLabel *featuresLabel;
    KComboBox *showModeCombobox;

    int keys[8];
    int state;

    QAction *toggleScreenReaderAction;
    bool m_error;
};
