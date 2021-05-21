/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef __K_ACCESS_H__
#define __K_ACCESS_H__

#include <QAbstractNativeEventFilter>
#include <QColor>
#include <QLabel>
#include <QPaintEvent>
#include <QWidget>

#include <phonon/MediaObject>

#include <X11/Xlib.h>
#define explicit int_explicit // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#include <xcb/xkb.h>
#undef explicit
#include <fixx11h.h>

class QLabel;
class KComboBox;

class KAccessApp : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit KAccessApp();
    void newInstance();
    void setXkbOpcode(int opcode);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long int *result) override;

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

    void activeWindowChanged(WId wid);
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

    int xkb_opcode;
    unsigned int features;
    unsigned int requestedFeatures;

    bool _systemBell, _artsBell, _visibleBell, _visibleBellInvert;
    QColor _visibleBellColor;
    int _visibleBellPause;

    bool _gestures, _gestureConfirmation;
    bool _kNotifyModifiers, _kNotifyAccessX;

    QWidget *overlay;

    Phonon::MediaObject *_player;
    QString _currentPlayerSource;

    WId _activeWindow;

    QDialog *dialog;
    QLabel *featuresLabel;
    KComboBox *showModeCombobox;

    int keys[8];
    int state;

    QAction *toggleScreenReaderAction;
    bool m_error;
};

class VisualBell : public QWidget
{
    Q_OBJECT

public:
    VisualBell(int pause)
        : QWidget((QWidget *)nullptr, Qt::X11BypassWindowManagerHint)
        , _pause(pause)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int _pause;
};

#endif
