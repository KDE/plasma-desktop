/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "evdev_settings.h"
#include "x11_evdev_backend.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusMessage>

#include "../migrationlib/kdelibs4config.h"

void EvdevSettings::apply(X11EvdevBackend *backend, bool force)
{
    if (!backend) {
        return;
    }

    backend->apply(force);
    handedNeedsApply = false;
}

void EvdevSettings::load(X11EvdevBackend *backend)
{
    KConfig config("kcminputrc");

    // TODO: what's a good threshold default value
    int threshold = 0;
    handed = Handed::Right;
    double accel = 1.0;
    if (backend) {
        auto handedOnServer = backend->handed();
        handedEnabled = handedOnServer != Handed::NotSupported;
        if (handedEnabled) {
            handed = handedOnServer;
        }
        accel = backend->accelRate();
        threshold = backend->threshold();
    }

    KConfigGroup group = config.group("Mouse");
    double a = group.readEntry("Acceleration", -1.0);
    if (a == -1)
        accelRate = accel;
    else
        accelRate = a;

    int t = group.readEntry("Threshold", -1);
    if (t == -1)
        thresholdMove = threshold;
    else
        thresholdMove = t;

    QString key = group.readEntry("MouseButtonMapping");
    if (key == "RightHanded")
        handed = Handed::Right;
    else if (key == "LeftHanded")
        handed = Handed::Left;
    reverseScrollPolarity = group.readEntry("ReverseScrollPolarity", false);

    handedNeedsApply = false;

    // SC/DC/AutoSelect/ChangeCursor
    group = config.group("KDE");
    doubleClickInterval = group.readEntry("DoubleClickInterval", 400);
    dragStartTime = group.readEntry("StartDragTime", 500);
    dragStartDist = group.readEntry("StartDragDist", 4);
    wheelScrollLines = group.readEntry("WheelScrollLines", 3);
}

// see KGlobalSettings::emitChange
enum ChangeType {
    PaletteChanged = 0,
    FontChanged,
    StyleChanged,
    SettingsChanged,
    IconChanged,
    CursorChanged,
    ToolbarStyleChanged,
    ClipboardConfigChanged,
    BlockShortcuts,
    NaturalSortingChanged
};
enum SettingsCategory {
    SETTINGS_MOUSE,
    SETTINGS_COMPLETION,
    SETTINGS_PATHS,
    SETTINGS_POPUPMENU,
    SETTINGS_QT,
    SETTINGS_SHORTCUTS,
    SETTINGS_LOCALE,
    SETTINGS_STYLE
};

static void emitChange(ChangeType changeType, int arg)
{
    // see KGlobalSettings::emitChange
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(static_cast<int>(changeType));
    args.append(arg);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

void EvdevSettings::save()
{
    KSharedConfig::Ptr kcminputProfile = KSharedConfig::openConfig("kcminputrc");
    KConfigGroup kcminputGroup(kcminputProfile, "Mouse");
    kcminputGroup.writeEntry("Acceleration", accelRate);
    kcminputGroup.writeEntry("Threshold", thresholdMove);
    if (handed == Handed::Right) {
        kcminputGroup.writeEntry("MouseButtonMapping", QString("RightHanded"));
    } else {
        kcminputGroup.writeEntry("MouseButtonMapping", QString("LeftHanded"));
    }
    kcminputGroup.writeEntry("ReverseScrollPolarity", reverseScrollPolarity);
    kcminputGroup.sync();

    KSharedConfig::Ptr profile = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup group(profile, "KDE");
    group.writeEntry("DoubleClickInterval", doubleClickInterval, KConfig::Persistent);
    group.writeEntry("StartDragTime", dragStartTime, KConfig::Persistent);
    group.writeEntry("StartDragDist", dragStartDist, KConfig::Persistent);
    group.writeEntry("WheelScrollLines", wheelScrollLines, KConfig::Persistent);

    group.sync();
    kcminputProfile->sync();

    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("Mouse"), "kcminputrc");
    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("KDE"), "kdeglobals");

    emitChange(SettingsChanged, SETTINGS_MOUSE);
}
