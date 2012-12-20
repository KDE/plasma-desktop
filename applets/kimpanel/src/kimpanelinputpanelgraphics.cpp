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
#include "dummywidget.h"

// KDE
#include <KIconLoader>
#include <KIcon>

// Plasma
#include <Plasma/IconWidget>

enum KimpanelOrientation {
    KimpanelNotSet = 0,
    KimpanelVertical = 1,
    KimpanelHorizontal = 2
};

KimpanelInputPanelGraphics::KimpanelInputPanelGraphics(QGraphicsItem* parent, Qt::WindowFlags wFlags):
    QGraphicsWidget(parent, wFlags),
    m_layout(new QGraphicsLinearLayout(Qt::Vertical)),
    m_upperLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
    m_lookupTableLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
    m_pageButtonLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
    m_lowerLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
    m_auxLabel(new KimpanelLabelGraphics(Auxiliary, this)),
    m_preeditLabel(new KimpanelLabelGraphics(Preedit, this)),
    m_pageUpIcon(new Plasma::IconWidget(this)),
    m_pageDownIcon(new Plasma::IconWidget(this)),
    m_dummyWidget(new DummyWidget(this)),
    m_tableEntryMapper(new QSignalMapper(this)),
    m_lastVisible(false),
    m_reverse(false),
    m_lookupTableCursor(-1),
    m_fontHeight(0),
    m_useVertical(false),
    m_useReverse(false),
    m_orientaion(Qt::Horizontal)
{
    setContentsMargins(0, 0, 0, 0);

    // Content inside this panel will rapidly changed
    setCacheMode(QGraphicsItem::NoCache);

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_upperLayout->setSpacing(0);
    m_upperLayout->setContentsMargins(0, 0, 0, 0);
    m_upperLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_upperLayout->addItem(m_auxLabel);
    m_upperLayout->addItem(m_preeditLabel);
    m_upperLayout->addItem(m_dummyWidget);

    m_lowerLayout->setSpacing(0);
    m_lowerLayout->setContentsMargins(0, 0, 0, 0);
    m_lowerLayout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_pageUpIcon->setIcon("arrow-left");
    m_pageUpIcon->setMinimumSize(0, 0);
    m_pageUpIcon->setMaximumSize(0, 0);
    m_pageUpIcon->hide();

    m_pageDownIcon->setIcon("arrow-right");
    m_pageDownIcon->setMinimumSize(0, 0);
    m_pageDownIcon->setMaximumSize(0, 0);
    m_pageDownIcon->hide();

    m_lowerLayout->addItem(m_lookupTableLayout);
    m_lowerLayout->addItem(m_pageButtonLayout);
    m_lowerLayout->setAlignment(m_pageButtonLayout, Qt::AlignVCenter);

    m_lookupTableLayout->setSpacing(0);
    m_lookupTableLayout->setContentsMargins(0, 0, 0, 0);
    m_lookupTableLayout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_pageButtonLayout->setSpacing(0);
    m_pageButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_pageButtonLayout->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_pageButtonLayout->addItem(m_pageUpIcon);
    m_pageButtonLayout->addItem(m_pageDownIcon);

    m_auxLabel->show();
    m_preeditLabel->show();
    m_preeditLabel->setDrawCursor(true);

    m_layout->addItem(m_upperLayout);
    m_layout->addItem(m_lowerLayout);

    setLayout(m_layout);

    loadSettings();

    connect(m_tableEntryMapper, SIGNAL(mapped(int)), this, SIGNAL(selectCandidate(int)));
    connect(m_pageUpIcon, SIGNAL(clicked()), this, SIGNAL(lookupTablePageUp()));
    connect(m_pageDownIcon, SIGNAL(clicked()), this, SIGNAL(lookupTablePageDown()));
    connect(KimpanelSettings::self(), SIGNAL(configChanged()), this, SLOT(loadSettings()));
    connect(m_preeditLabel, SIGNAL(sizeChanged()), this, SLOT(updateDummyWidget()));
    connect(m_auxLabel, SIGNAL(sizeChanged()), this, SLOT(updateDummyWidget()));
}

KimpanelInputPanelGraphics::~KimpanelInputPanelGraphics()
{
}

void KimpanelInputPanelGraphics::updateOrientation()
{
    Qt::Orientation newOrientation = m_useVertical ? Qt::Vertical : Qt::Horizontal;

    if (m_lookupTableOrientation == KimpanelVertical)
        newOrientation = Qt::Vertical;
    else if (m_lookupTableOrientation == KimpanelHorizontal)
        newOrientation = Qt::Horizontal;

    if (m_orientaion != newOrientation) {
        m_orientaion = newOrientation;

        m_lookupTableLayout->setOrientation(m_orientaion);
        m_lowerLayout->setOrientation(m_orientaion);
    }
}

void KimpanelInputPanelGraphics::loadSettings()
{
    QFontMetrics fm(KimpanelSettings::self()->font());
    m_fontHeight = fm.height();
    m_useVertical = KimpanelSettings::self()->verticalPreeditBar();

    updateOrientation();

    m_useReverse = KimpanelSettings::self()->useReverse();
    setReverse(m_reverse, true);
}


QSize KimpanelInputPanelGraphics::roundSize()
{
    QSize size = minimumSize().toSize();

    int roundSize = m_fontHeight * 4;
    int width = ((size.width() / roundSize) * roundSize)
            + ((size.width() % roundSize) ? roundSize : 0);
    return QSize(width, size.height());
}

void KimpanelInputPanelGraphics::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    emit sizeChanged();
}

void KimpanelInputPanelGraphics::setShowPreedit(bool show)
{
    preeditVisible = show;
    m_preeditLabel->setVisible(show);
}

void KimpanelInputPanelGraphics::setShowAux(bool show)
{
    auxVisible = show;
    m_auxLabel->setVisible(show);
}

void KimpanelInputPanelGraphics::setShowLookupTable(bool show)
{
    lookuptableVisible = show;
    if (!show)
        clearLookupTable();
    else
        updateLookupTable();
}

void KimpanelInputPanelGraphics::setLookupTableCursor(int cursor)
{
    m_lookupTableCursor = cursor;
}

void KimpanelInputPanelGraphics::setLookupTableLayout(int layout)
{
    m_lookupTableOrientation = layout;

    updateOrientation();
}

void KimpanelInputPanelGraphics::setPreeditCaret(int pos)
{
    if (m_cursorPos != pos) {
        m_cursorPos = pos;
        m_preeditLabel->setCursorPos(pos);
    }
}

void KimpanelInputPanelGraphics::setPreeditText(const QString& text,
        const QString& attrs)
{
    Q_UNUSED(attrs);
    if (m_text == text)
        return;
    m_text = text;
    m_preeditLabel->setText(QString(), text);
}

void KimpanelInputPanelGraphics::setAuxText(const QString& text,
        const QString& attrs)
{
    Q_UNUSED(attrs);
    if (m_auxText == text)
        return;
    m_auxText = text;
    m_auxLabel->setText(QString(), text);
}

void KimpanelInputPanelGraphics::clearLookupTable()
{
    while (m_lookupTableLayout->count() > 0) {
        m_lookupTableLayout->removeAt(0);
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
        if (i == m_lookupTableCursor)
            item->setHighLight(true);
        else
            item->setHighLight(false);
        m_tableEntryMapper->setMapping(item, i);
    }
    if (m_reverse && m_useVertical) {
        for (int i = length - 1; i >= 0; i--)
            m_lookupTableLayout->addItem(m_tableEntryLabels[i]);
    }
    else {
        for (int i = 0; i < length; i ++)
            m_lookupTableLayout->addItem(m_tableEntryLabels[i]);
    }
    for (int i = length; i < m_tableEntryLabels.length(); i ++) {
        KimpanelLabelGraphics* item = m_tableEntryLabels[i];
        item->hide();
    }

    m_pageUpIcon->setEnabled(m_hasPrev);
    m_pageDownIcon->setEnabled(m_hasNext);
    bool iconVisible = (m_hasPrev || m_hasNext);
    m_pageUpIcon->setVisible(iconVisible);
    m_pageDownIcon->setVisible(iconVisible);
    if (iconVisible) {
        m_pageUpIcon->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        m_pageUpIcon->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        m_pageDownIcon->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        m_pageDownIcon->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    }
    else {
        m_pageUpIcon->setMinimumSize(0, 0);
        m_pageUpIcon->setMaximumSize(0, 0);
        m_pageDownIcon->setMinimumSize(0, 0);
        m_pageDownIcon->setMaximumSize(0, 0);
    }
}

void KimpanelInputPanelGraphics::updateVisible()
{
    if (m_lastVisible != (preeditVisible || auxVisible || lookuptableVisible)) {
        m_lastVisible = (preeditVisible || auxVisible || lookuptableVisible);
    }
}

void KimpanelInputPanelGraphics::updateDummyWidget()
{
    m_dummyWidget->setMinimumHeight(qMax(m_preeditLabel->minimumHeight(), m_auxLabel->minimumHeight()));
    m_dummyWidget->setMaximumHeight(qMax(m_preeditLabel->minimumHeight(), m_auxLabel->minimumHeight()));
}

bool KimpanelInputPanelGraphics::markedVisible()
{
    return m_lastVisible;
}

void KimpanelInputPanelGraphics::updateSize()
{
    m_pageButtonLayout->invalidate();
    m_lookupTableLayout->invalidate();
    m_lowerLayout->invalidate();
    m_upperLayout->invalidate();
    m_layout->invalidate();
    resize(roundSize());
    update();
    // qDebug() << roundSize();
}

void KimpanelInputPanelGraphics::setReverse(bool reverse, bool force)
{
    reverse = reverse && m_useReverse;
    if (m_reverse != reverse || force) {
        m_reverse = reverse;
        while(m_layout->count() > 0)
            m_layout->removeAt(0);

        if (m_reverse && m_useReverse) {
            m_layout->addItem(m_lowerLayout);
            m_layout->addItem(m_upperLayout);
        }
        else {
            m_layout->addItem(m_upperLayout);
            m_layout->addItem(m_lowerLayout);
        }

        if (m_useVertical) {
            while(m_lowerLayout->count() > 0)
                m_lowerLayout->removeAt(0);

            if (m_reverse && m_useReverse) {
                m_lowerLayout->addItem(m_pageButtonLayout);
                m_lowerLayout->setAlignment(m_pageButtonLayout, Qt::AlignVCenter | Qt::AlignLeft);
                m_lowerLayout->addItem(m_lookupTableLayout);
                m_lowerLayout->setAlignment(m_lookupTableLayout, Qt::AlignLeft);
            }
            else {
                m_lowerLayout->addItem(m_lookupTableLayout);
                m_lowerLayout->setAlignment(m_lookupTableLayout, Qt::AlignLeft);
                m_lowerLayout->addItem(m_pageButtonLayout);
                m_lowerLayout->setAlignment(m_pageButtonLayout, Qt::AlignVCenter | Qt::AlignLeft);
            }
        }
        else {
            while(m_lowerLayout->count() > 0)
                m_lowerLayout->removeAt(0);
            m_lowerLayout->addItem(m_lookupTableLayout);
            m_lowerLayout->setAlignment(m_lookupTableLayout, Qt::AlignLeft);
            m_lowerLayout->addItem(m_pageButtonLayout);
            m_lowerLayout->setAlignment(m_pageButtonLayout, Qt::AlignVCenter | Qt::AlignLeft);
        }

        if (lookuptableVisible)
            updateLookupTable();
    }
}

