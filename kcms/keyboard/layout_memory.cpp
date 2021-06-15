/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layout_memory.h"
#include "debug.h"

#include <QX11Info>

#include <KWindowSystem>

#include "xkb_helper.h"

LayoutMemory::LayoutMemory(const KeyboardConfig &keyboardConfig_)
    : prevLayoutList(X11Helper::getLayoutsList())
    , keyboardConfig(keyboardConfig_)
{
    registerListeners();
}

LayoutMemory::~LayoutMemory()
{
    unregisterListeners();
}

void LayoutMemory::configChanged()
{
    //	this->layoutMap.clear();	// if needed this will be done on layoutMapChanged event
    unregisterListeners();
    registerListeners();
}

void LayoutMemory::registerListeners()
{
    if (keyboardConfig.switchingPolicy == KeyboardConfig::SWITCH_POLICY_WINDOW || keyboardConfig.switchingPolicy == KeyboardConfig::SWITCH_POLICY_APPLICATION) {
        connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &LayoutMemory::windowChanged);
        //		connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
    }
    if (keyboardConfig.switchingPolicy == KeyboardConfig::SWITCH_POLICY_DESKTOP) {
        connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &LayoutMemory::desktopChanged);
    }
}

void LayoutMemory::unregisterListeners()
{
    disconnect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &LayoutMemory::windowChanged);
    disconnect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &LayoutMemory::desktopChanged);
    //	disconnect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
}

QString LayoutMemory::getCurrentMapKey()
{
    switch (keyboardConfig.switchingPolicy) {
    case KeyboardConfig::SWITCH_POLICY_WINDOW: {
        WId wid = KWindowSystem::self()->activeWindow();
        KWindowInfo winInfo(wid, NET::WMWindowType);
        NET::WindowType windowType = winInfo.windowType(NET::NormalMask | NET::DesktopMask | NET::DialogMask);
        qCDebug(KCM_KEYBOARD, ) << "window type" << windowType;

        // we ignore desktop type so that our keybaord layout applet on desktop could change layout properly
        if (windowType == NET::Desktop)
            return previousLayoutMapKey;
        if (windowType != NET::Unknown && windowType != NET::Normal && windowType != NET::Dialog)
            return QString();

        return QString::number(wid);
    }
    case KeyboardConfig::SWITCH_POLICY_APPLICATION: {
        WId wid = KWindowSystem::self()->activeWindow();
        KWindowInfo winInfo(wid, NET::WMWindowType, NET::WM2WindowClass);
        NET::WindowType windowType = winInfo.windowType(NET::NormalMask | NET::DesktopMask | NET::DialogMask);
        qCDebug(KCM_KEYBOARD, ) << "window type" << windowType;

        // we ignore desktop type so that our keybaord layout applet on desktop could change layout properly
        if (windowType == NET::Desktop)
            return previousLayoutMapKey;
        if (windowType != NET::Unknown && windowType != NET::Normal && windowType != NET::Dialog)
            return QString();

        // shall we use pid or window class ??? - class seems better (see e.g. https://bugs.kde.org/show_bug.cgi?id=245507)
        // for window class shall we use class.class or class.name? (seem class.class is a bit better - more app-oriented)
        qCDebug(KCM_KEYBOARD, ) << "New active window with class.class: " << winInfo.windowClassClass();
        return QString(winInfo.windowClassClass());
        //		NETWinInfo winInfoForPid( QX11Info::display(), wid, QX11Info::appRootWindow(), NET::WMPid);
        //		return QString::number(winInfoForPid.pid());
    }
    case KeyboardConfig::SWITCH_POLICY_DESKTOP:
        return QString::number(KWindowSystem::self()->currentDesktop());
    default:
        return QString();
    }
}

static bool isExtraSubset(const QList<LayoutUnit> &allLayouts, const QList<LayoutUnit> &newList)
{
    if (allLayouts.first() != newList.first())
        return false;
    for (const LayoutUnit &layoutUnit : newList) {
        if (!allLayouts.contains(layoutUnit))
            return false;
    }
    return true;
}

void LayoutMemory::layoutMapChanged()
{
    QList<LayoutUnit> newLayoutList(X11Helper::getLayoutsList());

    if (prevLayoutList == newLayoutList)
        return;

    qCDebug(KCM_KEYBOARD, ) << "Layout map change: " << LayoutSet::toString(prevLayoutList) << "-->" << LayoutSet::toString(newLayoutList);
    prevLayoutList = newLayoutList;

    // TODO: need more thinking here on how to support external map resetting
    if (keyboardConfig.configureLayouts && isExtraSubset(keyboardConfig.layouts, newLayoutList)) {
        qCDebug(KCM_KEYBOARD, ) << "Layout map change for extra layout";
        layoutChanged(); // to remember new map for active "window"
    } else {
        if (newLayoutList != keyboardConfig.getDefaultLayouts()) {
            qCDebug(KCM_KEYBOARD, ) << "Layout map change from external source: clearing layout memory";
            layoutMap.clear();
        }
    }
}

void LayoutMemory::layoutChanged()
{
    QString layoutMapKey = getCurrentMapKey();
    if (layoutMapKey.isEmpty())
        return;

    layoutMap[layoutMapKey] = X11Helper::getCurrentLayouts();
}

void LayoutMemory::setCurrentLayoutFromMap()
{
    QString layoutMapKey = getCurrentMapKey();
    if (layoutMapKey.isEmpty())
        return;

    if (!layoutMap.contains(layoutMapKey)) {
        //		qCDebug(KCM_KEYBOARD, ) << "new key for layout map" << layoutMapKey;

        if (!X11Helper::isDefaultLayout()) {
            //			qCDebug(KCM_KEYBOARD, ) << "setting default layout for container key" << layoutMapKey;
            if (keyboardConfig.configureLayouts && X11Helper::getLayoutsList() != keyboardConfig.getDefaultLayouts()) {
                XkbHelper::initializeKeyboardLayouts(keyboardConfig.getDefaultLayouts());
            }
            X11Helper::setDefaultLayout();
        }
    } else {
        LayoutSet layoutFromMap = layoutMap[layoutMapKey];
        qCDebug(KCM_KEYBOARD, ) << "Setting layout map item" << layoutFromMap.currentLayout.toString() << "for container key" << layoutMapKey;

        LayoutSet currentLayouts = X11Helper::getCurrentLayouts();
        if (layoutFromMap.layouts != currentLayouts.layouts) {
            if (keyboardConfig.configureLayouts) {
                XkbHelper::initializeKeyboardLayouts(layoutFromMap.layouts);
            }
            X11Helper::setLayout(layoutFromMap.currentLayout);
        } else if (layoutFromMap.currentLayout != currentLayouts.currentLayout) {
            X11Helper::setLayout(layoutFromMap.currentLayout);
        }
    }

    previousLayoutMapKey = layoutMapKey;
}

void LayoutMemory::windowChanged(WId /*wId*/)
{
    setCurrentLayoutFromMap();
}

void LayoutMemory::desktopChanged(int /*desktop*/)
{
    setCurrentLayoutFromMap();
}
