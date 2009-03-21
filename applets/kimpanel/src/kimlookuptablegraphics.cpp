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

#include <plasma/paintutils.h>
#include <KIconLoader>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
//#include <QtCore>
//#include <QtGui>

#ifdef Q_WS_X11
#include <NETRootInfo>
#include <X11/Xlib.h>
#endif

KIMLookupTableGraphics::KIMLookupTableGraphics(PanelAgent *agent, QGraphicsItem *parent)
    :QGraphicsWidget(parent),
     m_preedit_caret(0),
     m_panel_agent(agent),
     m_auxVisible(false),
     m_preeditVisible(false),
     m_lookupTableVisible(false),
     m_tableOrientation(KIM::Horizontal),
     m_orientVar(1),
     m_cg(0),
     m_timerId(-1)
{
    m_layout = new KIM::SvgScriptLayout();

    KSharedConfigPtr config = KSharedConfig::openConfig("kimpanel");
    m_cg = new KConfigGroup(config,"LookupTable");
    m_tableOrientation = (KIM::LookupTableOrientation)m_cg->readEntry("Orientation",(int)KIM::Horizontal);
    if ((m_tableOrientation == KIM::FixedRows) || (m_tableOrientation == KIM::FixedColumns)) {
        m_orientVar = m_cg->readEntry("OrientationFixedValue",1);
        if (m_orientVar <= 0) {
            m_orientVar = 1;
        }
    }

    //setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    setContentsMargins(0,0,0,0);

    m_lookupTable = new QGraphicsWidget(this);
    m_tableLayout = new QGraphicsGridLayout();
    m_lookupTable->setLayout(m_tableLayout);

    m_spacing = QFontMetrics(qApp->font()).size(0,"XX").width();
    m_tableLayout->setContentsMargins(0,0,0,0);

    m_auxLabel = new KIMLabelGraphics(KIM::Auxiliary,this);
    m_auxLabel->hide();
    m_preeditLabel = new KIMLabelGraphics(KIM::Preedit,this);
    m_preeditLabel->setDrawCursor(true);
    m_preeditLabel->hide();
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

    m_auxLabel->hide();
    m_preeditLabel->hide();

    m_preeditLabel->show();
    m_pageUpIcon->show();
    m_pageDownIcon->show();

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

    themeUpdated();
    connect(KIM::Theme::defaultTheme(),SIGNAL(themeChanged()),
            this,SLOT(themeUpdated()));
}

KIMLookupTableGraphics::~KIMLookupTableGraphics()
{
}

QBitmap KIMLookupTableGraphics::mask() const
{
    return m_mask;
}

void KIMLookupTableGraphics::updateLookupTable(const LookupTable &lookup_table)
{
    m_lookup_table = lookup_table;
    // workaround for layout bug
    for (int i =0; i<m_tableLayout->columnCount(); i++) {
        m_tableLayout->setColumnSpacing(i,0);
    }
    m_tableLayout->updateGeometry();
    while (m_tableLayout->count() > 0) {
        m_tableLayout->removeAt(0);
    }
    foreach (KIMLabelGraphics *item, m_tableEntryLabels) {
        m_tableEntryMapper->removeMappings(item);
    }
    qDeleteAll(m_tableEntryLabels);
    m_tableEntryLabels.clear();
    int row = 0;
    int col = 0;
    int max_col = (lookup_table.entries.size() + m_orientVar - 1)/m_orientVar;
    foreach (const LookupTable::Entry &entry, lookup_table.entries) {
        KIMLabelGraphics *item = new KIMLabelGraphics(KIM::TableEntry,m_lookupTable);
        item->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        item->setLabel(entry.label);
        item->setText(entry.text);
        item->enableHoverEffect(true);
        m_tableLayout->addItem(item,row,col);
        switch (m_tableOrientation) {
        case KIM::Horizontal:
            col++;
            break;
        case KIM::Vertical:
            row++;
            break;
        case KIM::FixedRows:
            col++;
            if (col >= max_col) {
                col = 0;
                row++;
            }
            break;
        case KIM::FixedColumns:
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
    for (int i =0; i<m_tableLayout->columnCount()-1; i++) {
        m_tableLayout->setColumnSpacing(i,m_spacing);
    }
    m_tableLayout->updateGeometry();
    reLayout();
#if 0
    if (lookup_table.entries.size() > 0) {
        showLookupTable(true);
    }
#endif
}

void KIMLookupTableGraphics::updatePreeditCaret(int pos)
{
    m_preedit_caret = pos;
    m_preeditLabel->setCursorPos(pos);

    reLayout();
}

void KIMLookupTableGraphics::updatePreeditText(const QString &text,const QList<TextAttribute> &attrs)
{
    Q_UNUSED(attrs);

    m_preedit_text = text;
    m_preeditLabel->setText(text);

    reLayout();
}

void KIMLookupTableGraphics::updateAux(const QString &text,const QList<TextAttribute> &attrs)
{
    Q_UNUSED(attrs);

    m_aux_text = text;
    m_auxLabel->setText(text);

    reLayout();

    //showAux(!text.isEmpty());
}

void KIMLookupTableGraphics::showAux(bool to_show)
{
    m_auxVisible = to_show;
    reLayout();
}

void KIMLookupTableGraphics::showPreedit(bool to_show)
{
    m_preeditVisible = to_show;
    reLayout();
}

void KIMLookupTableGraphics::showLookupTable(bool to_show)
{
    m_lookupTableVisible = to_show;
    reLayout();
}

void KIMLookupTableGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->drawPixmap(m_layout->elementRect(),m_background,m_layout->elementRect());
}

void KIMLookupTableGraphics::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timerId) {
        killTimer(m_timerId);
        m_timerId = -1;

        m_layout->doLayout(m_auxVisible ? m_auxLabel->size() : QSizeF(0,0),
                "aux_area");
        m_layout->doLayout(m_preeditVisible ? m_preeditLabel->size() : QSizeF(0,0),
                "preedit_area");
        m_layout->doLayout(m_pageUpIcon->size(),"pageup_area");
        m_layout->doLayout(m_pageDownIcon->size(),"pagedown_area");
        m_layout->doLayout(m_lookupTableVisible ? m_tableLayout->preferredSize() : QSizeF(0,0),
                "lookuptable_area");
        generateBackground();
        setPreferredSize(m_layout->elementSize());
        resize(preferredSize());
        m_auxLabel->setGeometry(m_layout->elementRect("aux_area"));
        m_preeditLabel->setGeometry(m_layout->elementRect("preedit_area"));
        m_pageUpIcon->setGeometry(m_layout->elementRect("pageup_area"));
        m_pageDownIcon->setGeometry(m_layout->elementRect("pagedown_area"));
        m_lookupTable->setGeometry(m_layout->elementRect("lookuptable_area"));
        m_auxLabel->setVisible(m_auxVisible);
        m_preeditLabel->setVisible(m_preeditVisible);
        m_lookupTable->setVisible(m_lookupTableVisible);
        emit sizeChanged();
        if (m_auxVisible || m_preeditVisible || m_lookupTableVisible) {
            emit visibleChanged(true);
        } else {
            emit visibleChanged(false);
        }
        update();
    } else {
        QObject::timerEvent(e);
    }

}

void KIMLookupTableGraphics::themeUpdated()
{
    KIM::Theme *theme = KIM::Theme::defaultTheme();
//X     kDebug() << theme->lookuptableImagePath();
    m_layout->setImagePath(theme->lookuptableImagePath());
    m_layout->setScript(theme->lookuptableLayoutPath());
    m_layout->themeUpdated();
    reLayout();
}

void KIMLookupTableGraphics::reLayout()
{
    if (m_timerId == -1) {
        m_timerId = startTimer(50);
    }
}

void KIMLookupTableGraphics::generateBackground()
{
    m_background = QPixmap(m_layout->elementSize().toSize());
    m_background.fill(Qt::transparent);
    QPainter p(&m_background);
    m_layout->paint(&p);
    p.end();
    m_mask = m_background.mask();
    p.begin(&m_mask);
    p.setBrush(Qt::black);
    p.drawRect(m_layout->elementRect("aux_area"));
    p.drawRect(m_layout->elementRect("preedit_area"));
    p.drawRect(m_layout->elementRect("pagedown_area"));
    p.drawRect(m_layout->elementRect("pageup_area"));
    p.drawRect(m_layout->elementRect("lookuptable_area"));
}

#include "kimlookuptablegraphics.moc"
// vim: sw=4 sts=4 et tw=100
