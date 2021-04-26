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

#include <KDEDModule>
#include <QStringList>

#include "bindings.h"
#include "keyboard_dbus.h"
#include "layout_memory.h"
#include "layoutnames.h"

class XInputEventNotifier;
class KeyboardConfig;
struct Rules;

class Q_DECL_EXPORT KeyboardDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KeyboardLayouts")

    KeyboardConfig keyboardConfig;
    KeyboardLayoutActionCollection *actionCollection;
    XInputEventNotifier *xEventNotifier;
    LayoutMemory layoutMemory;
    const Rules *rules;

    void registerListeners();
    void registerShortcut();
    void unregisterListeners();
    void unregisterShortcut();

private Q_SLOTS:
    void configureKeyboard();
    void configureMouse();
    void layoutChangedSlot();
    void layoutMapChanged();
    bool setLayout(QAction *action);

public Q_SLOTS:
    Q_SCRIPTABLE void switchToNextLayout();
    Q_SCRIPTABLE void switchToPreviousLayout();
    Q_SCRIPTABLE bool setLayout(uint index);
    Q_SCRIPTABLE uint getLayout() const;
    Q_SCRIPTABLE QVector<LayoutNames> getLayoutsList() const;

Q_SIGNALS:
    Q_SCRIPTABLE void layoutChanged(uint index);
    Q_SCRIPTABLE void layoutListChanged();

public:
    KeyboardDaemon(QObject *parent, const QList<QVariant> &);
    ~KeyboardDaemon() override;
};

#endif /* KEYBOARD_DAEMON_H_ */
