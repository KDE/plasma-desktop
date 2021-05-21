/*
    SPDX-FileCopyrightText: 2009 Wang Hoi <zealot.hoi@gmail.com>
    SPDX-FileCopyrightText: 2011 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KIMPANEL_AGENT_H
#define KIMPANEL_AGENT_H

#include "kimpanelagenttype.h"

// Qt
#include <QDBusConnection>
#include <QDBusContext>
#include <QObject>
#include <QStringList>

class QDBusServiceWatcher;
class Impanel2Adaptor;
class ImpanelAdaptor;

class PanelAgent : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    PanelAgent(QObject *parent);
    ~PanelAgent() override;

    void configure();
    void created();
    void exit();
    void reloadConfig();
    void selectCandidate(int idx);
    void lookupTablePageUp();
    void lookupTablePageDown();
    void movePreeditCaret(int pos);
    void triggerProperty(const QString &key);

public Q_SLOTS: // METHODS
    void UpdateLookupTable(const QStringList &labels, const QStringList &candis, const QStringList &attrlists, bool has_prev, bool has_next);
    void UpdatePreeditText(const QString &text, const QString &attr);
    void UpdateAux(const QString &text, const QString &attr);
    void UpdateScreen(int screen_id);
    void UpdateProperty(const QString &prop);
    void RegisterProperties(const QStringList &props);
    void ExecDialog(const QString &prop);
    void ExecMenu(const QStringList &entries);
    void SetSpotRect(int x, int y, int w, int h);
    void SetLookupTable(const QStringList &labels, const QStringList &candis, const QStringList &attrlists, bool hasPrev, bool hasNext, int cursor, int layout);
    void serviceUnregistered(const QString &service);

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
    void PanelRegistered();
    void Exit();
    void ReloadConfig();

    // signals to inform kimpanel
    void enable(bool to_enable);

    void updatePreeditCaret(int pos);
    void updatePreeditText(const QString &text, const QList<TextAttribute> &attr);
    void updateAux(const QString &text, const QList<TextAttribute> &attr);
    void updateProperty(const KimpanelProperty &prop);
    void updateLookupTable(const KimpanelLookupTable &lookup_table);
    void updateLookupTableFull(const KimpanelLookupTable &lookup_table, int cursor, int layout);
    void updateSpotLocation(int x, int y);
    void updateSpotRect(int x, int y, int w, int h);

    void registerProperties(const QList<KimpanelProperty> &props);

    void execDialog(const KimpanelProperty &prop);
    void execMenu(const QList<KimpanelProperty> &prop_list);

    void showPreedit(bool to_show);
    void showAux(bool to_show);
    void showLookupTable(bool to_show);
    void updateLookupTableCursor(int pos);

private:
    QString m_currentService;
    QStringList m_cachedProps;
    ImpanelAdaptor *m_adaptor;
    Impanel2Adaptor *m_adaptor2;
    QDBusServiceWatcher *m_watcher;
    QDBusConnection m_connection;
    static int m_connectionIndex;
};

#endif // KIMPANEL_AGENT_H
