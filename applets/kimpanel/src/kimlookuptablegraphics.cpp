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

#include "kimlookuptablegraphics.h"
#include "kimlabelgraphics.h"
#include "kimpanelagent.h"
#include "kimpanelsettings.h"

#include <plasma/paintutils.h>
#include <KIconLoader>
#include <KIconEffect>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QAction>
#include <QList>
#include <QPalette>
#include <QWidget>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QPainter>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#ifdef Q_WS_X11
#include <QX11Info>
#include <NETRootInfo>
#include <KWindowSystem>
#include <X11/Xlib.h>
#endif

KIMLookupTableGraphics::KIMLookupTableGraphics(PanelAgent *agent, QGraphicsItem *parent)
    :QGraphicsWidget(parent),
     m_preedit_caret(0),
     m_panel_agent(agent),
     m_auxVisible(false),
     m_preeditVisible(false),
     m_lookupTableVisible(false),
     m_tableOrientation(KIM::Settings::LookupTableHorizontal),
     m_orientVar(1)
{
    //setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    setContentsMargins(0,0,0,0);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical,this);
    m_upperLayout = new QGraphicsLinearLayout(m_layout);
    m_lowerLayout = new QGraphicsGridLayout(m_layout);

    m_layout->addItem(m_upperLayout);
    m_layout->addItem(m_lowerLayout);

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0,0,0,0);

    m_upperLayout->setSpacing(0);
    m_upperLayout->setContentsMargins(0,0,0,0);

    m_spacing = 16;
    m_lowerLayout->setContentsMargins(0,0,0,0);

    m_auxLabel = new KIMLabelGraphics(KIM::Auxiliary,this);
    m_preeditLabel = new KIMLabelGraphics(KIM::Preedit,this);
    m_preeditLabel->setDrawCursor(true);
    m_pageUpIcon = new Plasma::IconWidget(this);
    connect(m_pageUpIcon,SIGNAL(clicked()),this,SIGNAL(LookupTablePageUp()));
    m_pageUpIcon->setIcon(KIcon("arrow-left"));
    m_pageUpIcon->setMinimumSize(KIconLoader::SizeSmallMedium,KIconLoader::SizeSmall);
    m_pageUpIcon->setMaximumSize(KIconLoader::SizeSmallMedium,KIconLoader::SizeSmall);
    m_pageUpIcon->hide();
    m_pageDownIcon = new Plasma::IconWidget(this);
    connect(m_pageDownIcon,SIGNAL(clicked()),this,SIGNAL(LookupTablePageDown()));
    m_pageDownIcon->setIcon(KIcon("arrow-right"));
    m_pageDownIcon->setMinimumSize(KIconLoader::SizeSmallMedium,KIconLoader::SizeSmall);
    m_pageDownIcon->setMaximumSize(KIconLoader::SizeSmallMedium,KIconLoader::SizeSmall);
    m_pageDownIcon->hide();

    m_upperLayout->addItem(m_auxLabel);
    m_upperLayout->addItem(m_preeditLabel);
    m_upperLayout->addItem(m_pageUpIcon);
    m_upperLayout->addItem(m_pageDownIcon);

    m_auxLabel->hide();
    m_preeditLabel->hide();

    m_pageUpIcon->show();
    m_pageDownIcon->show();

    setLayout(m_layout);

    m_tableEntryMapper = new QSignalMapper(this);
    connect(m_tableEntryMapper,SIGNAL(mapped(int)),
            this,SIGNAL(SelectCandidate(int)));

    if (m_panel_agent) {
        connect(m_panel_agent,
            SIGNAL(updateLookupTable(const LookupTable &)),
            this,
            SLOT(updateLookupTable(const LookupTable &)));
        connect(m_panel_agent,
            SIGNAL(updatePreeditCaret(int)),
            this,
            SLOT(updatePreeditCaret(int)));
        connect(m_panel_agent,
            SIGNAL(updatePreeditText(const QString &,const QList<TextAttribute> &)),
            this,
            SLOT(updatePreeditText(const QString &,const QList<TextAttribute> &)));
        connect(m_panel_agent,
            SIGNAL(updateAux(const QString &,const QList<TextAttribute> &)),
            this,
            SLOT(updateAux(const QString &,const QList<TextAttribute> &)));
        connect(m_panel_agent,
            SIGNAL(showPreedit(bool)),
            this,
            SLOT(showPreedit(bool)));
        connect(m_panel_agent,
            SIGNAL(showAux(bool)),
            this,
            SLOT(showAux(bool)));
        connect(m_panel_agent,
            SIGNAL(showLookupTable(bool)),
            this,
            SLOT(showLookupTable(bool)));
        connect(this,
                SIGNAL(LookupTablePageUp()),
                m_panel_agent,
                SIGNAL(LookupTablePageUp()));
        connect(this,
                SIGNAL(LookupTablePageDown()),
                m_panel_agent,
                SIGNAL(LookupTablePageDown()));
        connect(this,
                SIGNAL(SelectCandidate(int)),
                m_panel_agent,
                SIGNAL(SelectCandidate(int)));
    }
}

KIMLookupTableGraphics::~KIMLookupTableGraphics()
{
}

void KIMLookupTableGraphics::updateLookupTable(const LookupTable &lookup_table)
{
    m_lookup_table = lookup_table;
    // workaround for layout bug
    for (int i =0; i<m_lowerLayout->columnCount(); i++) {
        m_lowerLayout->setColumnSpacing(i,0);
    }
    m_lowerLayout->updateGeometry();
    while (m_lowerLayout->count() > 0) {
        m_lowerLayout->removeAt(0);
    }
    foreach (KIMLabelGraphics *item, m_tableEntryLabels) {
        m_tableEntryMapper->removeMappings(item);
    }
    qDeleteAll(m_tableEntryLabels);
    m_tableEntryLabels.clear();
    int row = 0;
    int col = 0;
    int max_col = (lookup_table.entries.size() + m_orientVar - 1)/m_orientVar;
    KIMLabelGraphics *item;
    foreach (const LookupTable::Entry &entry, lookup_table.entries) {
        item = new KIMLabelGraphics(KIM::TableEntry,this);
        item->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        item->setLabel(entry.label);
        item->setText(entry.text);
        item->enableHoverEffect(true);
        m_lowerLayout->addItem(item,row,col);
        switch (m_tableOrientation) {
        case KIM::Settings::LookupTableHorizontal:
            col++;
            break;
        case KIM::Settings::LookupTableVertical:
            row++;
            break;
        case KIM::Settings::LookupTableFixedRows:
            col++;
            if (col >= max_col) {
                col = 0;
                row++;
            }
            break;
        case KIM::Settings::LookupTableFixedColumns:
            col++;
            if (col >= m_orientVar) {
                col = 0;
                row++;
            }
            break;
        }
        m_tableEntryMapper->setMapping(item,m_tableEntryLabels.size());
        connect(item,SIGNAL(clicked()),m_tableEntryMapper,SLOT(map()));
        m_tableEntryLabels << item;
    }
    for (int i =0; i<m_lowerLayout->columnCount()-1; i++) {
        m_lowerLayout->setColumnSpacing(i,m_spacing);
    }
    // update pageup/pagedown icons
    KIconEffect effect;
    QPixmap pixmap = KIcon("arrow-left").pixmap(KIconLoader::SizeSmall);
    if (!lookup_table.has_prev) {
        pixmap = effect.apply(pixmap,KIconLoader::Desktop,KIconLoader::DisabledState);
    } else {
        pixmap = effect.apply(pixmap,KIconLoader::Desktop,KIconLoader::DefaultState);
    }
    m_pageUpIcon->setIcon(pixmap);

    pixmap = KIcon("arrow-right").pixmap(KIconLoader::SizeSmall);
    if (!lookup_table.has_next) {
        pixmap = effect.apply(pixmap,KIconLoader::Desktop,KIconLoader::DisabledState);
    } else {
        pixmap = effect.apply(pixmap,KIconLoader::Desktop,KIconLoader::DefaultState);
    }
    m_pageDownIcon->setIcon(pixmap);

    m_lowerLayout->updateGeometry();
    resize(preferredSize());
    emit sizeChanged();
    if (lookup_table.entries.size() > 0) {
        showLookupTable(true);
    }
}

void KIMLookupTableGraphics::updatePreeditCaret(int pos)
{
    m_preedit_caret = pos;
    m_preeditLabel->setCursorPos(pos);
}

void KIMLookupTableGraphics::updatePreeditText(const QString &text,const QList<TextAttribute> &attrs)
{
    Q_UNUSED(attrs)

    m_preedit_text = text;
    m_preeditLabel->setText(text);
    m_upperLayout->updateGeometry();
    resize(preferredSize());
    emit sizeChanged();
    showPreedit(!text.isEmpty());
}

void KIMLookupTableGraphics::updateAux(const QString &text,const QList<TextAttribute> &attrs)
{
    Q_UNUSED(attrs)

    m_aux_text = text;
    m_auxLabel->setText(text);

    m_upperLayout->updateGeometry();
    resize(preferredSize());
    emit sizeChanged();
    showAux(!text.isEmpty());
}

void KIMLookupTableGraphics::showAux(bool to_show)
{
    m_auxVisible = to_show;
    m_auxLabel->setVisible(to_show);
    if (m_auxVisible || m_preeditVisible || m_lookupTableVisible) {
        emit visibleChanged(true);
    } else {
        emit visibleChanged(false);
    }
}

void KIMLookupTableGraphics::showPreedit(bool to_show)
{
    m_preeditVisible = to_show;
    m_preeditLabel->setVisible(to_show);
    if (m_auxVisible || m_preeditVisible || m_lookupTableVisible) {
        emit visibleChanged(true);
    } else {
        emit visibleChanged(false);
    }
}

void KIMLookupTableGraphics::showLookupTable(bool to_show)
{
    m_lookupTableVisible = to_show;
    if (m_auxVisible || m_preeditVisible || m_lookupTableVisible) {
        emit visibleChanged(true);
    } else {
        emit visibleChanged(false);
    }
}

void KIMLookupTableGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

// vim: sw=4 sts=4 et tw=100

