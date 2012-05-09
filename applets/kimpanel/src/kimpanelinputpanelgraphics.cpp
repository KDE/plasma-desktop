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

#include "kimpanelinputpanelgraphics.h"
#include "kimpanellabelgraphics.h"
#include "kimpanelsettings.h"

// KDE
#include <KIconLoader>
#include <KIcon>

// Plasma
#include <Plasma/IconWidget>

KimpanelInputPanelGraphics::KimpanelInputPanelGraphics(QGraphicsItem* parent, Qt::WindowFlags wFlags):
    QGraphicsWidget(parent, wFlags),
    m_layout(new QGraphicsLinearLayout(Qt::Vertical)),
    m_upperLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
    m_lowerLayout(new QGraphicsLinearLayout),
    m_auxLabel(new KimpanelLabelGraphics(Auxiliary, this)),
    m_preeditLabel(new KimpanelLabelGraphics(Preedit, this)),
    m_pageUpIcon(new Plasma::IconWidget(this)),
    m_pageDownIcon(new Plasma::IconWidget(this)),
    m_tableEntryMapper(new QSignalMapper(this)),
    m_lastVisible(false),
    m_reverse(false)
{
    setContentsMargins(0, 0, 0, 0);

    // Content inside this panel will rapidly changed
    setCacheMode(QGraphicsItem::NoCache);

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_upperLayout->setSpacing(0);
    m_upperLayout->setContentsMargins(0, 0, 0, 0);

    m_lowerLayout->setContentsMargins(0, 0, 0, 0);

    m_upperLayout->addItem(m_auxLabel);
    m_upperLayout->addItem(m_preeditLabel);

    m_pageUpIcon->setIcon(KIcon("arrow-left"));
    m_pageUpIcon->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_pageUpIcon->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_pageUpIcon->hide();

    m_pageDownIcon->setIcon(KIcon("arrow-right"));
    m_pageDownIcon->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_pageDownIcon->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_pageDownIcon->hide();

    m_upperLayout->addStretch(1);
    m_upperLayout->addItem(m_pageUpIcon);
    m_upperLayout->addItem(m_pageDownIcon);
    m_upperLayout->setAlignment(m_pageUpIcon, Qt::AlignTop | Qt::AlignRight);
    m_upperLayout->setAlignment(m_pageDownIcon, Qt::AlignTop | Qt::AlignRight);

    m_auxLabel->hide();
    m_preeditLabel->hide();
    m_preeditLabel->setDrawCursor(true);

    m_pageUpIcon->show();
    m_pageDownIcon->show();

    m_layout->addItem(m_upperLayout);
    m_layout->addItem(m_lowerLayout);

    setLayout(m_layout);

    connect(m_tableEntryMapper, SIGNAL(mapped(int)), this, SIGNAL(selectCandidate(int)));
    connect(m_pageUpIcon, SIGNAL(clicked()), this, SIGNAL(lookupTablePageUp()));
    connect(m_pageDownIcon, SIGNAL(clicked()), this, SIGNAL(lookupTablePageDown()));
}

KimpanelInputPanelGraphics::~KimpanelInputPanelGraphics()
{
}

QSizeF KimpanelInputPanelGraphics::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    if (which == Qt::MinimumSize || which == Qt::PreferredSize) {
        int width = (int) qMax(m_upperLayout->preferredWidth(), m_lowerLayout->preferredWidth());
        int height = (int)(m_upperLayout->preferredHeight() + m_lowerLayout->preferredHeight());

        QFontMetrics fm(KimpanelSettings::self()->font());
        int roundSize = fm.height() * 4;
        width = ((width / roundSize) * roundSize)
                + ((width % roundSize) ? roundSize : 0);
        return QSizeF(width, height);
    } else
        return QGraphicsWidget::sizeHint(which, constraint);
}


void KimpanelInputPanelGraphics::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QGraphicsWidget::paint(painter, option, widget);
}

void KimpanelInputPanelGraphics::setShowPreedit(bool show)
{
    preeditVisible = show;
    m_preeditLabel->setVisible(show);
    if (m_preeditLabel->preferredHeight() > 0) {
        QSizeF size(m_preeditLabel->preferredHeight(), m_preeditLabel->preferredHeight());
        m_pageUpIcon->setMinimumSize(size);
        m_pageUpIcon->setMaximumSize(size);
        m_pageDownIcon->setMinimumSize(size);
        m_pageDownIcon->setMaximumSize(size);
    }
    updateVisible();
}

void KimpanelInputPanelGraphics::setShowAux(bool show)
{
    auxVisible = show;
    m_auxLabel->setVisible(show);
    if (m_auxLabel->preferredHeight() > 0) {
        QSizeF size(m_auxLabel->preferredHeight(), m_auxLabel->preferredHeight());
        m_pageUpIcon->setMinimumSize(size);
        m_pageUpIcon->setMaximumSize(size);
        m_pageDownIcon->setMinimumSize(size);
        m_pageDownIcon->setMaximumSize(size);
    }
    updateVisible();
}

void KimpanelInputPanelGraphics::setShowLookupTable(bool show)
{
    lookuptableVisible = show;
    if (!show)
        clearLookupTable();
    else
        updateLookupTable();
    updateVisible();
}

void KimpanelInputPanelGraphics::setPreeditCaret(int pos)
{
    if (m_cursorPos != pos) {
        m_cursorPos = pos;
        m_preeditLabel->setCursorPos(pos);
        updateSize();
    }
}

void KimpanelInputPanelGraphics::setPreeditText(const QString& text,
        const QString& attrs)
{
    Q_UNUSED(attrs);
    if (m_text == text)
        return;
    m_text = text;
    m_preeditLabel->setText("", text);

    updateSize();
}

void KimpanelInputPanelGraphics::setAuxText(const QString& text,
        const QString& attrs)
{
    Q_UNUSED(attrs);
    if (m_auxText == text)
        return;
    m_auxText = text;
    m_auxLabel->setText("", text);

    updateSize();
}

void KimpanelInputPanelGraphics::clearLookupTable()
{
    while (m_lowerLayout->count() > 0) {
        m_lowerLayout->removeAt(0);
    }
    foreach(KimpanelLabelGraphics * item, m_tableEntryLabels) {
        m_tableEntryMapper->removeMappings(item);
    }
}

void KimpanelInputPanelGraphics::setLookupTable(const QStringList& labels,
        const QStringList& candidates,
        bool hasPrev,
        bool hasNext,
        const QStringList& attrs
)
{
    Q_UNUSED(attrs);
    if (m_labels == labels && m_candidates == candidates
            && m_hasNext == hasPrev && m_hasPrev == hasPrev)
        return;
    m_labels = labels;
    m_candidates = candidates;
    m_hasPrev = hasPrev;
    m_hasNext = hasNext;

    updateLookupTable();
}

void KimpanelInputPanelGraphics::updateLookupTable()
{
    if (KimpanelSettings::self()->verticalPreeditBar())
        m_lowerLayout->setOrientation(Qt::Vertical);
    else
        m_lowerLayout->setOrientation(Qt::Horizontal);

    clearLookupTable();

    int length = qMin(m_labels.size(), m_candidates.size());
    for (int i = 0; i < length; i ++) {
        KimpanelLabelGraphics* item = NULL;
        if (m_tableEntryLabels.length() < i + 1) {
            item = new KimpanelLabelGraphics(TableEntry, this);
            item->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            connect(item, SIGNAL(clicked()), m_tableEntryMapper, SLOT(map()));
            m_tableEntryLabels << item;
        }
        item = m_tableEntryLabels[i];
        item->show();
        item->setText(m_labels[i], m_candidates[i]);
        m_tableEntryMapper->setMapping(item, i);
    }
    if (m_reverse) {
        for (int i = length - 1; i >= 0; i--)
            m_lowerLayout->addItem(m_tableEntryLabels[i]);
    }
    else {
        for (int i = 0; i < length; i ++)
            m_lowerLayout->addItem(m_tableEntryLabels[i]);
    }
    for (int i = length; i < m_tableEntryLabels.length(); i ++) {
        KimpanelLabelGraphics* item = m_tableEntryLabels[i];
        item->hide();
    }

    m_pageUpIcon->setEnabled(m_hasPrev);
    m_pageDownIcon->setEnabled(m_hasNext);
    updateSize();
}

void KimpanelInputPanelGraphics::updateVisible()
{
    if (m_lastVisible != (preeditVisible || auxVisible || lookuptableVisible)) {
        m_lastVisible = (preeditVisible || auxVisible || lookuptableVisible);
        emit visibleChanged(m_lastVisible);
    }
}

void KimpanelInputPanelGraphics::updateSize()
{
    m_upperLayout->invalidate();
    m_lowerLayout->invalidate();
    m_layout->invalidate();

    resize(preferredSize());
    emit sizeChanged();
    update();
}

void KimpanelInputPanelGraphics::setReverse(bool reverse)
{
    reverse = reverse && KimpanelSettings::self()->useReverse() && KimpanelSettings::self()->verticalPreeditBar();
    if (m_reverse != reverse) {
        m_reverse = reverse;
        while(m_layout->count() > 0)
            m_layout->removeAt(0);

        if (m_reverse && KimpanelSettings::self()->useReverse()) {
            m_layout->addItem(m_lowerLayout);
            m_layout->addItem(m_upperLayout);
        }
        else {
            m_layout->addItem(m_upperLayout);
            m_layout->addItem(m_lowerLayout);
        }
        updateSize();
    }
}

