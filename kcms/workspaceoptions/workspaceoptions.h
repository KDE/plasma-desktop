/*
 *  Copyright (C) 2018 <furkantokac34@gmail.com>
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
#ifndef _KCM_WORKSPACEOPTIONS_H
#define _KCM_WORKSPACEOPTIONS_H

#include <KQuickAddons/ConfigModule>

class KCMWorkspaceOptions : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(bool toolTip READ getToolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(bool visualFeedback READ getVisualFeedback WRITE setVisualFeedback NOTIFY visualFeedbackChanged)
    Q_PROPERTY(bool singleClick READ getSingleClick WRITE setSingleClick NOTIFY singleClickChanged)

public:
    KCMWorkspaceOptions(QObject* parent, const QVariantList& args);
    ~KCMWorkspaceOptions() {}

    // QML Properties
    bool getToolTip() const;
    void setToolTip(bool state);

    bool getVisualFeedback() const;
    void setVisualFeedback(bool state);

    bool getSingleClick() const;
    void setSingleClick(bool state);

public Q_SLOTS:
    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void toolTipChanged();
    void visualFeedbackChanged();
    void singleClickChanged();

private:
    void handleNeedsSave();

    // QML variables
    bool m_ostateToolTip;   // Original state
    bool m_stateToolTip;    // Current state

    bool m_ostateVisualFeedback;
    bool m_stateVisualFeedback;

    bool m_ostateSingleClick;
    bool m_stateSingleClick;
};

#endif  // _KCM_WORKSPACEOPTIONS_H
