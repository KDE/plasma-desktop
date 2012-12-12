/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
#ifndef KIMPANEL_AGENT_H
#define KIMPANEL_AGENT_H

#include "kimpanel/kimpanelagenttype.h"

// Qt
#include <QObject>
#include <QStringList>
#include <QDBusContext>

class QDBusServiceWatcher;
class Impanel2Adaptor;
class ImpanelAdaptor;

class PanelAgent: public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    PanelAgent(QObject *parent);
    virtual ~PanelAgent();

    void configure();
    void created();
    void exit();
    void reloadConfig();
    void selectCandidate(int idx);
    void lookupTablePageUp();
    void lookupTablePageDown();
    void movePreeditCaret(int pos);
    void triggerProperty(const QString& key);

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void UpdateLookupTable(const QStringList &labels,
                           const QStringList &candis,
                           const QStringList &attrlists,
                           bool has_prev, bool has_next);
    void UpdatePreeditText(const QString &text, const QString &attr);
    void UpdateAux(const QString &text, const QString &attr);
    void UpdateScreen(int screen_id);
    void UpdateProperty(const QString &prop);
    void RegisterProperties(const QStringList &props);
    void ExecDialog(const QString &prop);
    void ExecMenu(const QStringList &entries);
    void SetSpotRect(int x, int y, int w, int h);
    void SetLookupTable(const QStringList &labels,
                        const QStringList &candis,
                        const QStringList &attrlists,
                        bool hasPrev, bool hasNext, int cursor, int layout);
    void serviceUnregistered(const QString& service);

Q_SIGNALS:
    // signals that from kimpanel
    void Configure();
    void MovePreeditCaret(int position);
    void SelectCandidate(int index);
    void LookupTablePageUp();
    void LookupTablePageDown();
    void TriggerProperty(const QString &key);
    void PanelCreated();
    void PanelCreated2();
    void Exit();
    void ReloadConfig();

    // signals to inform kimpanel
    void enable(bool to_enable);

    void updatePreeditCaret(int pos);
    void updatePreeditText(const QString &text, const QList<TextAttribute> &attr);
    void updateAux(const QString &text, const QList<TextAttribute> &attr);
    void updateProperty(const KimpanelProperty &prop);
    void updateLookupTable(const KimpanelLookupTable &lookup_table);
    void updateLookupTableFull(const KimpanelLookupTable& lookup_table, int cursor, int layout);
    void updateSpotLocation(int x, int y);
    void updateSpotRect(int x, int y, int w ,int h);

    void registerProperties(const QList<KimpanelProperty> &props);

    void execDialog(const KimpanelProperty &prop);
    void execMenu(const QList<KimpanelProperty> &prop_list);

    void showPreedit(bool to_show);
    void showAux(bool to_show);
    void showLookupTable(bool to_show);
    void updateLookupTableCursor(int pos);

private:
    bool m_show_aux;
    bool m_show_preedit;
    bool m_show_lookup_table;
    int m_spot_x;
    int m_spot_y;
    QString m_currentService;
    QStringList cached_props;
    ImpanelAdaptor* adaptor;
    Impanel2Adaptor* adaptor2;
    QDBusServiceWatcher* watcher;
};

#endif // KIMPANEL_AGENT_H

