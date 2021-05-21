/*
    SPDX-FileCopyrightText: 2001 Rik Hemsley (rikkus) <rik@kde.org>
    SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
