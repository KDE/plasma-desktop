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

#include "kimpanelruntime_export.h"
#include <QtCore/QObject>
#include <QStringList>
template<class T> class QList;
template<class Key, class Value> class QMap;

#include "kimagenttype.h"

class KIMPANELRUNTIME_EXPORT PanelAgent: public QObject
{
    Q_OBJECT

public:
    PanelAgent(QObject *parent);
    virtual ~PanelAgent();

    void created();
    void exit();
    void reloadConfig();
    void selectCandidate(int idx);
    void lookupTablePageUp();
    void lookupTablePageDown();
    void movePreeditCaret(int pos);

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void UpdateLookupTable(const QStringList &labels,
        const QStringList &candis,
        const QStringList &attrlists,
        bool has_prev,bool has_next);
    void UpdatePreeditText(const QString &text,const QString &attr);
    void UpdateAux(const QString &text,const QString &attr);
    void UpdateScreen(int screen_id);
    void UpdateProperty(const QString &prop);
    void RegisterProperties(const QStringList &props);
    void ExecDialog(const QString &prop);
    void ExecMenu(const QStringList &entries);

Q_SIGNALS: // SIGNALS
    void MovePreeditCaret(int position);
    void SelectCandidate(int index);
    void LookupTablePageUp();
    void LookupTablePageDown();
    void TriggerProperty(const QString &key);
    void PanelCreated();
    void Exit();
    void ReloadConfig();


    // signals to inform kimpanel
    void enable(bool to_enable);

    void updatePreeditCaret(int pos);
    void updatePreeditText(const QString &text,const QList<TextAttribute> &attr);
    void updateAux(const QString &text,const QList<TextAttribute> &attr);
    void updateProperty(const Property &prop);
    void updateLookupTable(const LookupTable &lookup_table);
    void updateSpotLocation(int x,int y);
    
    void registerProperties(const QList<Property> &props);
    
    void execDialog(const Property &prop);
    void execMenu(const QList<Property> &prop_list);

    void showPreedit(bool to_show);
    void showAux(bool to_show);
    void showLookupTable(bool to_show);

private:
    bool m_show_aux;
    bool m_show_preedit;
    bool m_show_lookup_table;
    int m_spot_x;
    int m_spot_y;
    QStringList cached_props;
};

#endif // PANEL_H
