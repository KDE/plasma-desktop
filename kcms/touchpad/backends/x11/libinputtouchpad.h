/*
    SPDX-FileCopyrightText: 2019 Atul Bisht <atulbisht26@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIBINPUTTOUCHPAD_H
#define LIBINPUTTOUCHPAD_H

#include "backends/libinputcommon.h"
#include "xlibtouchpad.h"

#include <KConfigGroup>
#include <KSharedConfig>

class LibinputTouchpad : public LibinputCommon, public XlibTouchpad
{
    Q_OBJECT

public:
    LibinputTouchpad(Display *display, int deviceId);
    ~LibinputTouchpad() override
    {
    }

    bool getConfig() override;
    bool applyConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() override;

    int touchpadOff() override;
    XcbAtom &touchpadOffAtom() override;

private:
    template<typename T>
    bool valueLoader(Prop<T> &prop);

    template<typename T>
    QString valueWriter(const Prop<T> &prop);

    KSharedConfigPtr m_config;

    //
    // general
    QString name() const override
    {
        return m_name;
    }
    bool supportsDisableEvents() const override
    {
        return m_supportsDisableEvents.avail && m_supportsDisableEvents.val;
    }
    bool isEnabled() const override
    {
        return !m_enabled.val;
    }
    void setEnabled(bool set) override
    {
        m_enabled.set(!set);
    }
    //
    // Tapping
    void setLmrTapButtonMap(bool set) override
    {
        m_lrmTapButtonMap.set(!set);
        m_lmrTapButtonMap.set(set);
    }
    //
    // advanced
    bool supportsLeftHanded() const override
    {
        return m_leftHanded.avail;
    }
    bool supportsDisableEventsOnExternalMouse() const override
    {
        return m_supportsDisableEventsOnExternalMouse.avail && m_supportsDisableEventsOnExternalMouse.val;
    }
    bool supportsDisableWhileTyping() const override
    {
        return m_disableWhileTyping.avail;
    }
    bool supportsMiddleEmulation() const override
    {
        return m_middleEmulation.avail;
    }
    //
    // acceleration speed and profile
    bool supportsPointerAcceleration() const override
    {
        return m_pointerAcceleration.avail;
    }
    bool supportsPointerAccelerationProfileFlat() const override
    {
        return m_supportsPointerAccelerationProfileFlat.avail && m_supportsPointerAccelerationProfileFlat.val;
    }
    bool supportsPointerAccelerationProfileAdaptive() const override
    {
        return m_supportsPointerAccelerationProfileAdaptive.avail && m_supportsPointerAccelerationProfileAdaptive.val;
    }
    //
    // scrolling
    bool supportsNaturalScroll() const override
    {
        return m_naturalScroll.avail;
    }
    bool supportsHorizontalScrolling() const override
    {
        return true;
    }
    bool supportsScrollTwoFinger() const override
    {
        return m_supportsScrollTwoFinger.avail && m_supportsScrollTwoFinger.val;
    }
    bool supportsScrollEdge() const override
    {
        return m_supportsScrollEdge.avail && m_supportsScrollEdge.val;
    }
    bool supportsScrollOnButtonDown() const override
    {
        return m_supportsScrollOnButtonDown.avail && m_supportsScrollOnButtonDown.val;
    }
    //
    // click method
    bool supportsClickMethodAreas() const override
    {
        return m_supportsClickMethodAreas.avail && m_supportsClickMethodAreas.val;
    }
    bool supportsClickMethodClickfinger() const override
    {
        return m_supportsClickMethodClickfinger.avail && m_supportsClickMethodClickfinger.val;
    }

    bool supportsScrollFactor() const override
    {
        return false;
    }

    // Tapping
    Prop<bool> m_lrmTapButtonMapEnabledByDefault = Prop<bool>("lrmTapButtonMapEnabledByDefault");
    Prop<bool> m_lrmTapButtonMap = Prop<bool>("lrmTapButtonMap");
    //
    // advanced
    Prop<bool> m_disableEventsOnExternalMouse = Prop<bool>("disableEventsOnExternalMouse");
    Prop<bool> m_disableEventsOnExternalMouseDefault = Prop<bool>("disableEventsOnExternalMouseDefault");

    QString m_name;
};

#endif // LIBINPUTTOUCHPAD_H
