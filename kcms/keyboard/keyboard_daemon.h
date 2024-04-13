/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KDEDModule>
#include <QStringList>
#include <optional>

#include "bindings.h"
#include "keyboard_dbus.h"
#include "layout_memory.h"
#include "layoutnames.h"

class XInputEventNotifier;
class KeyboardConfig;
class KeyboardSettings;

class Q_DECL_EXPORT KeyboardDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KeyboardLayouts")

    KeyboardSettings *keyboardSettings;
    KeyboardConfig *keyboardConfig;
    KeyboardLayoutActionCollection *actionCollection;
    XInputEventNotifier *xEventNotifier;
    LayoutMemory layoutMemory;
    std::optional<uint> lastUsedLayout;

    void registerListeners();
    void registerShortcut();
    void unregisterListeners();
    void unregisterShortcut();
    void setLastUsedLayoutValue(uint newValue);

private Q_SLOTS:
    void configureKeyboard();
    void configureInput();
    void layoutChangedSlot();
    void layoutMapChanged();
    bool setLayout(QAction *action);

public Q_SLOTS:
    Q_SCRIPTABLE void switchToNextLayout();
    Q_SCRIPTABLE void switchToPreviousLayout();
    Q_SCRIPTABLE bool setLayout(uint index);
    Q_SCRIPTABLE uint getLayout() const;
    Q_SCRIPTABLE QList<LayoutNames> getLayoutsList() const;

Q_SIGNALS:
    Q_SCRIPTABLE void layoutChanged(uint index);
    Q_SCRIPTABLE void layoutListChanged();

public:
    KeyboardDaemon(QObject *parent, const QList<QVariant> &);
    ~KeyboardDaemon() override;
};
