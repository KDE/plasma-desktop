/*
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAUNCHFEEDBACKSETTINGS_H
#define LAUNCHFEEDBACKSETTINGS_H

#include "launchfeedbacksettingsbase.h"

class LaunchFeedbackSettingsStore;

class LaunchFeedbackSettings : public LaunchFeedbackSettingsBase
{
    Q_OBJECT

    Q_PROPERTY(bool busyCursorDisabled READ busyCursorDisabled WRITE setBusyCursorDisabled NOTIFY busyCursorDisabledChanged)
    Q_PROPERTY(bool busyCursorStatic READ busyCursorStatic WRITE setBusyCursorStatic NOTIFY busyCursorStaticChanged)
    Q_PROPERTY(bool busyCursorBlinking READ busyCursorBlinking WRITE setBusyCursorBlinking NOTIFY busyCursorBlinkingChanged)
    Q_PROPERTY(bool busyCursorBouncing READ busyCursorBouncing WRITE setBusyCursorBouncing NOTIFY busyCursorBouncingChanged)

public:
    LaunchFeedbackSettings(QObject *parent = nullptr);

    bool busyCursorDisabled() const;
    bool busyCursorStatic() const;
    bool busyCursorBlinking() const;
    bool busyCursorBouncing() const;

public Q_SLOTS:
    void setBusyCursorDisabled(bool busyCursorDisabled);
    void setBusyCursorStatic(bool busyCursorStatic);
    void setBusyCursorBlinking(bool busyCursorBlinking);
    void setBusyCursorBouncing(bool busyCursorBouncing);

    void setSelectedBusyCursor(QString selectedBusyCursor);

Q_SIGNALS:
    void busyCursorDisabledChanged();
    void busyCursorStaticChanged();
    void busyCursorBlinkingChanged();
    void busyCursorBouncingChanged();

private:
    LaunchFeedbackSettingsStore *m_settingsStore;

    using NotifySignalType = void (LaunchFeedbackSettings::*)();
    void addItemInternal(const QByteArray &propertyName, const QVariant &defaultValue, NotifySignalType notifySignal);
};

#endif // LAUNCHFEEDBACKSETTINGS_H
