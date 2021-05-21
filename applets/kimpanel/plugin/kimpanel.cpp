/*
    SPDX-FileCopyrightText: 2020 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "kimpanel.h"

Kimpanel::Kimpanel(QObject *parent)
    : QObject(parent)
    , m_panelAgent(new PanelAgent(this))
{
    connect(m_panelAgent, &PanelAgent::updateAux, this, &Kimpanel::updateAux);
    connect(m_panelAgent, &PanelAgent::updatePreeditText, this, &Kimpanel::updatePreeditText);
    connect(m_panelAgent, &PanelAgent::updatePreeditCaret, this, &Kimpanel::updatePreeditCaret);
    connect(m_panelAgent, &PanelAgent::updateLookupTable, this, &Kimpanel::updateLookupTable);
    connect(m_panelAgent, &PanelAgent::updateSpotLocation, this, &Kimpanel::updateSpotLocation);
    connect(m_panelAgent, &PanelAgent::updateSpotRect, this, &Kimpanel::updateSpotRect);
    connect(m_panelAgent, &PanelAgent::showAux, this, &Kimpanel::showAux);
    connect(m_panelAgent, &PanelAgent::showPreedit, this, &Kimpanel::showPreedit);
    connect(m_panelAgent, &PanelAgent::showLookupTable, this, &Kimpanel::showLookupTable);
    connect(m_panelAgent, &PanelAgent::updateLookupTableCursor, this, &Kimpanel::updateLookupTableCursor);
    connect(m_panelAgent, &PanelAgent::updateLookupTableFull, this, &Kimpanel::updateLookupTableFull);

    connect(m_panelAgent, &PanelAgent::updateProperty, this, &Kimpanel::updateProperty);
    connect(m_panelAgent, &PanelAgent::registerProperties, this, &Kimpanel::registerProperties);
    connect(m_panelAgent, &PanelAgent::execMenu, this, &Kimpanel::execMenu);
    connect(m_panelAgent, &PanelAgent::execDialog, this, &Kimpanel::execDialog);
}

void Kimpanel::updateAux(const QString &text, const QList<TextAttribute> &attrList)
{
    Q_UNUSED(attrList);
    if (m_auxText != text) {
        m_auxText = text;
        Q_EMIT auxTextChanged();
    }
}

void Kimpanel::updatePreeditText(const QString &text, const QList<TextAttribute> &attrList)
{
    Q_UNUSED(attrList);
    if (m_preeditText != text) {
        m_preeditText = text;
        Q_EMIT preeditTextChanged();
    }
}

void Kimpanel::updatePreeditCaret(int pos)
{
    if (m_caretPos != pos) {
        m_caretPos = pos;
        Q_EMIT preeditTextChanged();
    }
}

void Kimpanel::updateLookupTable(const KimpanelLookupTable &lookupTable)
{
    m_labels.clear();
    m_texts.clear();
    Q_FOREACH (const KimpanelLookupTable::Entry &entry, lookupTable.entries) {
        m_labels << entry.label;
        m_texts << entry.text;
    }
    m_hasPrev = lookupTable.has_prev;
    m_hasNext = lookupTable.has_next;
    Q_EMIT lookupTableChanged();
}

void Kimpanel::updateSpotLocation(int x, int y)
{
    updateSpotRect(x, y, 0, 0);
}

void Kimpanel::updateSpotRect(int x, int y, int w, int h)
{
    m_spotRect = QRect(x, y, w, h);
    Q_EMIT spotRectChanged();
}

void Kimpanel::showAux(bool visible)
{
    if (m_auxVisible != visible) {
        m_auxVisible = visible;
        Q_EMIT auxTextChanged();
    }
}

void Kimpanel::showPreedit(bool visible)
{
    if (m_preeditVisible != visible) {
        m_preeditVisible = visible;
        Q_EMIT preeditTextChanged();
    }
}

void Kimpanel::showLookupTable(bool visible)
{
    if (m_lookupTableVisible != visible) {
        m_lookupTableVisible = visible;
        Q_EMIT lookupTableChanged();
    }
}

void Kimpanel::updateLookupTableCursor(int cursor)
{
    if (m_lookupTableCursor != cursor) {
        m_lookupTableCursor = cursor;
        Q_EMIT lookupTableChanged();
    }
}

void Kimpanel::updateLookupTableFull(const KimpanelLookupTable &lookupTable, int cursor, int layout)
{
    m_labels.clear();
    m_texts.clear();
    Q_FOREACH (const KimpanelLookupTable::Entry &entry, lookupTable.entries) {
        m_labels << entry.label;
        m_texts << entry.text;
    }
    m_hasPrev = lookupTable.has_prev;
    m_hasNext = lookupTable.has_next;
    m_lookupTableCursor = cursor;
    m_lookupTableLayout = layout;
    Q_EMIT lookupTableChanged();
}

void Kimpanel::updateProperty(const KimpanelProperty &property)
{
    for (auto &prop : m_props) {
        if (prop.toMap()[QStringLiteral("key")] == property.key) {
            prop = property.toMap();
            Q_EMIT propertiesChanged();
            break;
        }
    }
}

void Kimpanel::registerProperties(const QList<KimpanelProperty> &props)
{
    m_props.clear();
    for (const auto &prop : props) {
        m_props << prop.toMap();
    }
    Q_EMIT propertiesChanged();
}

void Kimpanel::execMenu(const QList<KimpanelProperty> &props)
{
    QVariantList menuProps;
    for (const auto &prop : props) {
        menuProps << prop.toMap();
    }
    Q_EMIT menuTriggered(menuProps);
}

void Kimpanel::execDialog(const KimpanelProperty &prop)
{
    Q_UNUSED(prop)
}

void Kimpanel::configure()
{
    m_panelAgent->configure();
}

void Kimpanel::exit()
{
    m_panelAgent->exit();
}

void Kimpanel::lookupTablePageDown()
{
    m_panelAgent->lookupTablePageDown();
}

void Kimpanel::lookupTablePageUp()
{
    m_panelAgent->lookupTablePageUp();
}

void Kimpanel::movePreeditCaret(int position)
{
    m_panelAgent->movePreeditCaret(position);
}

void Kimpanel::reloadConfig()
{
    m_panelAgent->reloadConfig();
}

void Kimpanel::selectCandidate(int candidate)
{
    m_panelAgent->selectCandidate(candidate);
}

void Kimpanel::triggerProperty(const QString &key)
{
    m_panelAgent->triggerProperty(key);
}
