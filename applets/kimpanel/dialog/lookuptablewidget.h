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
#ifndef LOOKUPTABLEWIDGET_H
#define LOOKUPTABLEWIDGET_H

#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <QtGui>

#include "kimpaneltype.h"

class LookupTableWidget : public QWidget
{
Q_OBJECT
public:
    LookupTableWidget(QWidget *parent=0);
    ~LookupTableWidget();
    
//X     void setEnabled(bool to_enable);

Q_SIGNALS:
    void SelectCandidate(int idx);
    void LookupTablePageUp();
    void LookupTablePageDown();
    
public Q_SLOTS:
    void themeUpdated();
    void updateSpotLocation(int x,int y);
    void updateLookupTable(const LookupTable &lookup_table);
    void updateAux(const QString &text,const QList<TextAttribute> &attrs);
    void updatePreeditCaret(int pos);
    void updatePreeditText(const QString &text,const QList<TextAttribute> &attrs);
    void showAux(bool to_show);
    void showPreedit(bool to_show);
    void showLookupTable(bool to_show);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool event(QEvent *e);

private:
    Plasma::FrameSvg *m_background_svg;
//X     Plasma::Svg *m_background_svg;
//X     Plasma::Svg *m_factory_svg;
//X     Plasma::Svg *m_config_svg;
//X     Plasma::Svg *m_help_svg;

//X     QList<Plasma::Svg *> m_properties_svg;
//X     QList<Property> m_properties;
//X 
//X     QString m_button_stylesheet;

    QVBoxLayout *m_layout;

//X     QWidget *m_button_container;
//X 
//X     QToolButton *m_factory_button;
//X     QToolButton *m_config_button;
//X     QToolButton *m_help_button;

    bool m_dragging;
    QPoint m_init_pos;

//X     QMap<QString,QToolButton *> prop_map;
    QSignalMapper mapper;
    
    QString m_aux_text;
    QString m_preedit_text;
    int m_preedit_caret;
    LookupTable m_lookup_table;

    QLabel *m_aux_label;
    QLabel *m_preedit_label;
    QLabel *m_candis_label;

    QDesktopWidget m_desktop;
};

#endif // LOOKUPTABLEWIDGET_H
