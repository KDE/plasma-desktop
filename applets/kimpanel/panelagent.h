/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
#ifndef PANELAGENT_H
#define PANELAGENT_H
#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

class PanelAgent: public QObject
{
    Q_OBJECT
public:
    PanelAgent(QObject *parent);
    virtual ~PanelAgent();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Enable(bool to_enable);
    void ShowHelp();
    void ShowPreedit(bool to_show);
    void ShowAux(bool to_show);
    void ShowLookupTable(bool to_show);
    void UpdateLookupTable(const QStringList &labels,
        const QStringList &candis,
        const QStringList &attrlists,
        int,int,int,bool);
    void UpdatePreeditText(const QString &text);
    void UpdateAux(const QString &text);
    void UpdateSpotLocation(int x,int y);
    void UpdateScreen(int screen_id);
Q_SIGNALS: // SIGNALS
    void MovePreeditCaret(int position);
    void ShowFactoryMenu();
    void ChangeFactory(int index);
    void SelectCandidate(int index);
    void LookupTablePageUp();
    void LookupTablePageDown();
    void Exit();
    void ReloadConfig();
};

#endif // PANEL_H
