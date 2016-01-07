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

#ifndef KIMPANEL_DATAENGINE_H
#define KIMPANEL_DATAENGINE_H

#include <Plasma/DataEngine>

#define INPUTPANEL_SOURCE_NAME "inputpanel"
#define STATUSBAR_SOURCE_NAME "statusbar"

class KimpanelService;
class PanelAgent;
/**
 * This engine provides access to kimpanel
 *
 */
class KimpanelEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    KimpanelEngine(QObject* parent, const QVariantList& args);
    virtual Plasma::Service* serviceForSource(const QString& source);
    virtual void init();

private:
    PanelAgent* m_panelAgent;
};

#endif // KIMPANEL_DATAENGINE_H
