/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

class InputDevice : public QObject
{
    Q_OBJECT

    //
    // general
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(bool supportsDisableEvents READ supportsDisableEvents CONSTANT FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)

    //
    // advanced
    Q_PROPERTY(Qt::MouseButtons supportedButtons READ supportedButtons CONSTANT FINAL)

    Q_PROPERTY(bool supportsLeftHanded READ supportsLeftHanded CONSTANT FINAL)
    Q_PROPERTY(bool leftHandedEnabledByDefault READ leftHandedEnabledByDefault CONSTANT FINAL)
    Q_PROPERTY(bool leftHanded READ isLeftHanded WRITE setLeftHanded NOTIFY leftHandedChanged FINAL)

    Q_PROPERTY(bool supportsMiddleEmulation READ supportsMiddleEmulation CONSTANT FINAL)
    Q_PROPERTY(bool middleEmulationEnabledByDefault READ middleEmulationEnabledByDefault CONSTANT FINAL)
    Q_PROPERTY(bool middleEmulation READ isMiddleEmulation WRITE setMiddleEmulation NOTIFY middleEmulationChanged FINAL)

    //
    // acceleration speed and profile
    Q_PROPERTY(bool supportsPointerAcceleration READ supportsPointerAcceleration CONSTANT FINAL)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration NOTIFY pointerAccelerationChanged FINAL)

    Q_PROPERTY(bool supportsPointerAccelerationProfileFlat READ supportsPointerAccelerationProfileFlat CONSTANT FINAL)
    Q_PROPERTY(bool defaultPointerAccelerationProfileFlat READ defaultPointerAccelerationProfileFlat CONSTANT FINAL)
    Q_PROPERTY(bool pointerAccelerationProfileFlat READ pointerAccelerationProfileFlat WRITE setPointerAccelerationProfileFlat NOTIFY
                   pointerAccelerationProfileFlatChanged FINAL)

    Q_PROPERTY(bool supportsPointerAccelerationProfileAdaptive READ supportsPointerAccelerationProfileAdaptive CONSTANT FINAL)
    Q_PROPERTY(bool defaultPointerAccelerationProfileAdaptive READ defaultPointerAccelerationProfileAdaptive CONSTANT FINAL)
    Q_PROPERTY(bool pointerAccelerationProfileAdaptive READ pointerAccelerationProfileAdaptive WRITE setPointerAccelerationProfileAdaptive NOTIFY
                   pointerAccelerationProfileAdaptiveChanged FINAL)

    //
    // scrolling
    Q_PROPERTY(bool supportsNaturalScroll READ supportsNaturalScroll CONSTANT FINAL)
    Q_PROPERTY(bool naturalScrollEnabledByDefault READ naturalScrollEnabledByDefault CONSTANT FINAL)
    Q_PROPERTY(bool naturalScroll READ isNaturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged FINAL)
    Q_PROPERTY(bool supportsScrollOnButtonDown READ supportsScrollOnButtonDown CONSTANT FINAL)
    Q_PROPERTY(bool scrollOnButtonDownEnabledByDefault READ scrollOnButtonDownEnabledByDefault CONSTANT FINAL)
    Q_PROPERTY(bool scrollOnButtonDown READ isScrollOnButtonDown WRITE setScrollOnButtonDown NOTIFY scrollOnButtonDownChanged FINAL)
    // The only unique property to the Wayland backend
    Q_PROPERTY(qreal scrollFactor READ scrollFactor WRITE setScrollFactor NOTIFY scrollFactorChanged FINAL)

public:
    using QObject::QObject;

    //
    // general
    virtual QString name() const = 0;

    virtual bool supportsDisableEvents() const = 0;

    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    //
    // advanced
    virtual Qt::MouseButtons supportedButtons() const = 0;

    virtual bool supportsLeftHanded() const = 0;

    virtual bool leftHandedEnabledByDefault() const = 0;

    virtual bool isLeftHanded() const = 0;
    virtual void setLeftHanded(bool set) = 0;

    virtual bool supportsMiddleEmulation() const = 0;

    virtual bool middleEmulationEnabledByDefault() const = 0;

    virtual bool isMiddleEmulation() const = 0;
    virtual void setMiddleEmulation(bool set) = 0;

    //
    // acceleration speed and profile
    virtual bool supportsPointerAcceleration() const = 0;

    virtual qreal pointerAcceleration() const = 0;
    virtual void setPointerAcceleration(qreal acceleration) = 0;

    virtual bool supportsPointerAccelerationProfileFlat() const = 0;

    virtual bool defaultPointerAccelerationProfileFlat() const = 0;

    virtual bool pointerAccelerationProfileFlat() const = 0;
    virtual void setPointerAccelerationProfileFlat(bool set) = 0;

    virtual bool supportsPointerAccelerationProfileAdaptive() const = 0;

    virtual bool defaultPointerAccelerationProfileAdaptive() const = 0;

    virtual bool pointerAccelerationProfileAdaptive() const = 0;
    virtual void setPointerAccelerationProfileAdaptive(bool set) = 0;

    //
    // scrolling
    virtual bool supportsNaturalScroll() const = 0;

    virtual bool naturalScrollEnabledByDefault() const = 0;

    virtual bool isNaturalScroll() const = 0;
    virtual void setNaturalScroll(bool set) = 0;

    virtual bool supportsScrollOnButtonDown() const = 0;

    virtual bool scrollOnButtonDownEnabledByDefault() const = 0;

    virtual bool isScrollOnButtonDown() const = 0;
    virtual void setScrollOnButtonDown(bool set) = 0;

    virtual qreal scrollFactor() const = 0;
    virtual void setScrollFactor(qreal set) = 0;

Q_SIGNALS:
    void needsSaveChanged();

    void leftHandedChanged();
    void pointerAccelerationChanged();
    void pointerAccelerationProfileFlatChanged();
    void pointerAccelerationProfileAdaptiveChanged();
    void enabledChanged();
    void middleEmulationChanged();
    void naturalScrollChanged();
    void scrollOnButtonDownChanged();
    // The only unique property to the Wayland backend
    void scrollFactorChanged();
};
