/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KEYBOARD_DAEMON_H_
#define KEYBOARD_DAEMON_H_

#include <QStringList>
#include <QTimer>
#include <KDEDModule>

#include "../keyboard_dbus.h"
#include "bindings.h"

#include "layout_list_models.h"
#include <fcitx_interface.h>

class XInputEventNotifier;
struct XkbRules;

class Q_DECL_EXPORT KeyboardDaemon : public KDEDModule {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KeyboardLayouts")

    LayoutListModels* m_layoutListModels;
    int m_currentLayoutIndex;

    KeyboardLayoutActionCollection* actionCollection;
    XInputEventNotifier* xEventNotifier;
    QTimer configureKeyboardTimer;
    QTimer configureMouseTimer;

    bool isXEventRegistered;

    FcitxQtInputMethodProxy m_fcitxDBusProxy;

    QString updateCurrentLayout(int newLayoutIndex);

    void registerListeners();
    void registerShortcut();
    void unregisterListeners();
    void unregisterShortcut();

private Q_SLOTS:
    void switchToNextLayout();
    void configureKeyboard();
    void configureMouse();
    void layoutChanged();
    void layoutMapChanged();
    bool setLayout(QAction* action);

public Q_SLOTS:
    Q_SCRIPTABLE bool setLayout(const QString& layout);
    Q_SCRIPTABLE QString getCurrentLayout();
    Q_SCRIPTABLE QStringList getLayoutsList();
    Q_SCRIPTABLE QString getLayoutDisplayName(const QString& layout);

Q_SIGNALS:
    Q_SCRIPTABLE void currentLayoutChanged(QString layout);
    Q_SCRIPTABLE void layoutListChanged();
    Q_SCRIPTABLE void configChanged();

public:
    KeyboardDaemon(QObject* parent, const QList<QVariant>&);
    ~KeyboardDaemon() override;
};

#endif /* KEYBOARD_DAEMON_H_ */
