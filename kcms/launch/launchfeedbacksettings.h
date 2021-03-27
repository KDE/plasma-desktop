/**
 *  Copyright 2020 Cyril Rossi <cyril.rossi@enioka.com>
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
