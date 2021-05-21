/*
    SPDX-FileCopyrightText: 2020 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KIMPANEL_H
#define KIMPANEL_H

#include "kimpanelagent.h"
#include <QList>
#include <QObject>
#include <QRect>
#include <QVariantList>

class Kimpanel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString auxText MEMBER m_auxText NOTIFY auxTextChanged)
    Q_PROPERTY(bool auxVisible MEMBER m_auxVisible NOTIFY auxTextChanged)

    Q_PROPERTY(QString preeditText MEMBER m_preeditText NOTIFY preeditTextChanged)
    Q_PROPERTY(int caretPos MEMBER m_caretPos NOTIFY preeditTextChanged)
    Q_PROPERTY(bool preeditVisible MEMBER m_preeditVisible NOTIFY preeditTextChanged)

    Q_PROPERTY(QRect spotRect MEMBER m_spotRect NOTIFY spotRectChanged)

    Q_PROPERTY(bool lookupTableVisible MEMBER m_lookupTableVisible NOTIFY lookupTableChanged)
    Q_PROPERTY(int lookupTableCursor MEMBER m_lookupTableCursor NOTIFY lookupTableChanged)
    Q_PROPERTY(int lookupTableLayout MEMBER m_lookupTableLayout NOTIFY lookupTableChanged)
    Q_PROPERTY(bool hasPrev MEMBER m_hasPrev NOTIFY lookupTableChanged)
    Q_PROPERTY(bool hasNext MEMBER m_hasNext NOTIFY lookupTableChanged)
    Q_PROPERTY(QStringList labels MEMBER m_labels NOTIFY lookupTableChanged)
    Q_PROPERTY(QStringList texts MEMBER m_texts NOTIFY lookupTableChanged)

    Q_PROPERTY(QVariantList properties MEMBER m_props NOTIFY propertiesChanged)
public:
    Kimpanel(QObject *parent = nullptr);

    Q_INVOKABLE void lookupTablePageUp();
    Q_INVOKABLE void lookupTablePageDown();
    Q_INVOKABLE void movePreeditCaret(int position);
    Q_INVOKABLE void selectCandidate(int candidate);
    Q_INVOKABLE void triggerProperty(const QString &key);
    Q_INVOKABLE void reloadConfig();
    Q_INVOKABLE void configure();
    Q_INVOKABLE void exit();

Q_SIGNALS:
    void auxTextChanged();
    void preeditTextChanged();
    void lookupTableChanged();
    void spotRectChanged();
    void propertiesChanged();
    void menuTriggered(const QVariantList &props);

private Q_SLOTS:
    void updatePreeditText(const QString &text, const QList<TextAttribute> &attrList);
    void updateAux(const QString &text, const QList<TextAttribute> &attrList);
    void updatePreeditCaret(int pos);
    void updateLookupTable(const KimpanelLookupTable &lookupTable);
    void updateLookupTableFull(const KimpanelLookupTable &lookupTable, int cursor, int layout);
    void updateSpotLocation(int x, int y);
    void updateSpotRect(int x, int y, int w, int h);
    void showAux(bool visible);
    void showPreedit(bool visible);
    void showLookupTable(bool visible);
    void updateLookupTableCursor(int cursor);

    void updateProperty(const KimpanelProperty &property);
    void registerProperties(const QList<KimpanelProperty> &props);
    void execDialog(const KimpanelProperty &prop);
    void execMenu(const QList<KimpanelProperty> &prop_list);

private:
    PanelAgent *m_panelAgent;
    QString m_auxText;
    QString m_preeditText;
    int m_caretPos = 0;
    QRect m_spotRect;
    bool m_auxVisible = false;
    bool m_preeditVisible = false;
    bool m_lookupTableVisible = false;
    int m_lookupTableCursor = -1;
    int m_lookupTableLayout = 0;
    bool m_hasPrev = false;
    bool m_hasNext = false;
    QStringList m_labels;
    QStringList m_texts;

    QVariantList m_props;
};

#endif
