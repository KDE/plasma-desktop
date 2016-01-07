/***************************************************************************
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KIMPANEL_JOB_H
#define KIMPANEL_JOB_H

// Qt
#include <QMap>

// Plasma
#include <Plasma/ServiceJob>

class PanelAgent;
class KimpanelJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    KimpanelJob(PanelAgent* panelAgent, const QString& destination,
                const QString& operation, const QMap< QString, QVariant >& parameters, QObject* parent = 0);
    virtual void start();

private:
    PanelAgent* m_panelAgent;
};

#endif // KIMPANEL_JOB_H

