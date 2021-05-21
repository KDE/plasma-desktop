/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef APP_H
#define APP_H
#include <ibus.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "panel.h"
#include <QAbstractNativeEventFilter>
#include <QByteArray>
#include <QGuiApplication>
#include <QMap>
#include <QPair>
class QDBusServiceWatcher;

class XcbEventFilter : public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long int *result) override;
};

class App : public QGuiApplication
{
    Q_OBJECT
public:
    typedef QPair<uint, uint> TriggerKey;
    App(int &argc, char *argv[]);
    ~App() override;
    void setTriggerKeys(QList<TriggerKey> triggersList);
    void setDoGrab(bool doGrab);
    bool keyboardGrabbed()
    {
        return m_keyboardGrabbed;
    }
    bool nativeEvent(xcb_generic_event_t *event);
    void nameAcquired();
    void nameLost();
    QByteArray normalizeIconName(const QByteArray &icon) const;
public Q_SLOTS:
    void init();
    void finalize();
    void clean();
    void grabKey();
    void ungrabKey();
    uint getPrimaryModifier(uint state);
    void keyRelease(const xcb_key_release_event_t *event);
    void accept();
    void ungrabXKeyboard();
    bool grabXKeyboard();

private:
    QScopedPointer<XcbEventFilter> m_eventFilter;
    bool m_init;
    IBusBus *m_bus;
    IBusPanelImpanel *m_impanel;
    QList<QPair<uint, uint>> m_triggersList;
    bool m_keyboardGrabbed;
    bool m_doGrab;
    xcb_key_symbols_t *m_syms;
    QMap<QByteArray, QByteArray> m_iconMap;
    QDBusServiceWatcher *m_watcher;
};

#endif // APP_H
