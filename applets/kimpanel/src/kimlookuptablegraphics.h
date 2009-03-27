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
#ifndef KIM_LOOKUPTABLE_GRAPHICS_H
#define KIM_LOOKUPTABLE_GRAPHICS_H

#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/flashinglabel.h>
#include <plasma/widgets/iconwidget.h>

#include <KConfigGroup>

#include <QBitmap>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QSignalMapper>

#include "kimpanelruntime_export.h"
#include "kimpanelsettings.h"
#include "kimagenttype.h"

class PanelAgent;
class KIMLabelGraphics;

class KIMPANELRUNTIME_EXPORT KIMLookupTableGraphics: public QGraphicsWidget
{
Q_OBJECT
public:
    explicit KIMLookupTableGraphics(PanelAgent * = 0,QGraphicsItem *parent=0);
    ~KIMLookupTableGraphics();

Q_SIGNALS:
    void SelectCandidate(int idx);
    void LookupTablePageUp();
    void LookupTablePageDown();

    void sizeChanged();
    void visibleChanged(bool);
    
public Q_SLOTS:
    void updateLookupTable(const LookupTable &lookup_table);
    void updateAux(const QString &text,const QList<TextAttribute> &attrs);
    void updatePreeditCaret(int pos);
    void updatePreeditText(const QString &text,const QList<TextAttribute> &attrs);
    void showAux(bool to_show);
    void showPreedit(bool to_show);
    void showLookupTable(bool to_show);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    //Plasma m_background;
    QBitmap m_mask;

    QGraphicsLinearLayout *m_layout;
    QGraphicsLinearLayout *m_upperLayout;
    QGraphicsGridLayout *m_lowerLayout;

    QSignalMapper mapper;

    QString m_aux_text;
    QString m_preedit_text;
    int m_preedit_caret;

    LookupTable m_lookup_table;

    PanelAgent *m_panel_agent;

    bool m_auxVisible;
    bool m_preeditVisible;
    bool m_lookupTableVisible;

    KIMLabelGraphics *m_auxLabel;
    KIMLabelGraphics *m_preeditLabel;
    Plasma::IconWidget *m_pageUpIcon;
    Plasma::IconWidget *m_pageDownIcon;
    QList<KIMLabelGraphics *> m_tableEntryLabels;

    QSignalMapper *m_tableEntryMapper;

    int m_spacing;

    int m_tableOrientation;
    int m_orientVar;

    int m_timerId;
};

#endif // KIM_LOOKUPTABLE_GRAPHICS_H
