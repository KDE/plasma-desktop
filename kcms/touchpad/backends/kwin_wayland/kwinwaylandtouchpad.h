/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <backends/libinputcommon.h>

class QDBusInterface;

class KWinWaylandTouchpad : public LibinputCommon
{
    Q_OBJECT

    Q_PROPERTY(qreal scrollFactor READ scrollFactor WRITE setScrollFactor NOTIFY scrollFactorChanged)

public:
    KWinWaylandTouchpad(QString dbusName);
    ~KWinWaylandTouchpad() override;

    bool init();

    bool getConfig();
    bool getDefaultConfig();
    bool applyConfig();
    bool isChangedConfig() const;

    //
    // general
    QString name() const override
    {
        return m_name.val;
    }
    QString sysName() const
    {
        return m_sysName.val;
    }
    bool supportsDisableEvents() const override
    {
        return m_supportsDisableEvents.val;
    }
    void setEnabled(bool enabled) override
    {
        m_enabled.set(enabled);
    }
    bool isEnabled() const override
    {
        return m_enabled.val;
    }
    //
    // advanced
    bool supportsLeftHanded() const override
    {
        return m_supportsLeftHanded.val;
    }
    bool supportsDisableEventsOnExternalMouse() const override
    {
        return m_supportsDisableEventsOnExternalMouse.val;
    }
    bool supportsDisableWhileTyping() const override
    {
        return m_supportsDisableWhileTyping.val;
    }
    bool supportsMiddleEmulation() const override
    {
        return m_supportsMiddleEmulation.val;
    }
    //
    // tapping
    void setLmrTapButtonMap(bool set) override
    {
        m_lmrTapButtonMap.set(set);
    }
    //
    // acceleration speed and profile
    bool supportsPointerAcceleration() const override
    {
        return m_supportsPointerAcceleration.val;
    }
    bool supportsPointerAccelerationProfileFlat() const override
    {
        return m_supportsPointerAccelerationProfileFlat.val;
    }
    bool supportsPointerAccelerationProfileAdaptive() const override
    {
        return m_supportsPointerAccelerationProfileAdaptive.val;
    }
    bool supportsPointerAccelerationProfileCustom() const override
    {
        return m_supportsPointerAccelerationProfileCustom.val;
    }
    //
    // scrolling
    bool supportsNaturalScroll() const override
    {
        return m_supportsNaturalScroll.val;
    }
    bool supportsHorizontalScrolling() const override
    {
        return false;
    }
    bool supportsScrollTwoFinger() const override
    {
        return m_supportsScrollTwoFinger.val;
    }
    bool supportsScrollEdge() const override
    {
        return m_supportsScrollEdge.val;
    }
    bool supportsScrollOnButtonDown() const override
    {
        return m_supportsScrollOnButtonDown.val;
    }

    //
    // Scroll Factor
    bool supportsScrollFactor() const override
    {
        return true;
    }
    qreal scrollFactor() const
    {
        return m_scrollFactor.val;
    }
    void setScrollFactor(qreal factor)
    {
        return m_scrollFactor.set(factor);
    }

    //
    // Click method
    bool supportsClickMethodAreas() const override
    {
        return m_supportsClickMethodAreas.val;
    }
    bool supportsClickMethodClickfinger() const override
    {
        return m_supportsClickMethodClickfinger.val;
    }

private:
    template<typename T>
    bool valueLoader(Prop<T> &prop);

    template<typename T>
    QString valueWriter(const Prop<T> &prop);
    //
    // general
    Prop<QString> m_name = Prop<QString>(this, "name", QString());
    Prop<QString> m_sysName = Prop<QString>(this, "sysName", QString());

    //
    // advanced
    Prop<bool> m_supportsLeftHanded = PropBool(this, "supportsLeftHanded");
    Prop<bool> m_supportsDisableWhileTyping = PropBool(this, "supportsDisableWhileTyping");
    Prop<bool> m_supportsMiddleEmulation = PropBool(this, "supportsMiddleEmulation");

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration = PropBool(this, "supportsPointerAcceleration");

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll = PropBool(this, "supportsNaturalScroll");
    Prop<qreal> m_scrollFactor = Prop<qreal>(this, "scrollFactor", 0, &LibinputCommon::scrollFactorChanged);

    QDBusInterface *m_iface;
};
