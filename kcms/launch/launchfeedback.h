/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
 *  Copyright (C) 2017 Eike Hein <hein@kde.org>
 *  Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>
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
 */

#ifndef LAUNCHFEEDBACK_H
#define LAUNCHFEEDBACK_H

#include <KQuickAddons/ManagedConfigModule>

class LaunchFeedbackData;
class LaunchFeedbackSettings;

class LaunchFeedback : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(LaunchFeedbackSettings *launchFeedbackSettings READ launchFeedbackSettings CONSTANT)

public:
    enum class CursorFeedbackType {
        None,
        Static,
        Blinking,
        Bouncing,
    };
    Q_ENUM(CursorFeedbackType)

    explicit LaunchFeedback(QObject *parent = nullptr, const QVariantList &list = QVariantList());
    ~LaunchFeedback() override;

    LaunchFeedbackSettings *launchFeedbackSettings() const;

private:
    LaunchFeedbackData *m_data;
};

#endif
