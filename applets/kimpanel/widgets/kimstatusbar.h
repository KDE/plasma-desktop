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
#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include <plasma/dialog.h>
#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <kiconloader.h>
#include <QtGui>

#include "kimagenttype.h"
#include "kimstatusbargraphics.h"
#include "kimsvgscriptlayout.h"
#include "kimtheme.h"

class KConfigGroup;
class QDesktopWidget;
class KIMStatusBarGraphics;

class KIMStatusBar : public QWidget
{
Q_OBJECT
public:
    KIMStatusBar(QWidget *parent=0, const QList<QAction *> extra_actions = QList<QAction *>());
    ~KIMStatusBar();
    
    void setEnabled(bool to_enable);

    void setGraphicsWidget(KIMStatusBarGraphics *widget);
    KIMStatusBarGraphics *graphicsWidget() 
    {
        return m_widget;
    }

    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void triggerProperty(const QString &key);

public Q_SLOTS:
//X     void updateAux(const QString &text,const QList<TextAttribute> &attrs);
    void updateProperty(const Property &prop);
    void registerProperties(const QList<Property> &props);

private Q_SLOTS:
    void themeUpdated();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);
    bool event(QEvent *e);

private:
    void updateResizeCorners();
    Plasma::Dialog::ResizeCorner resizeCorner(const QPoint &p);
    void generateBackground();

private:
    KIM::SvgScriptLayout *m_layout;
    QPixmap m_background;
    QBitmap m_mask;

    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    KIMStatusBarGraphics *m_widget;

    QList<Plasma::Svg *> m_properties_svg;
    QList<Property> m_props;

    QString m_button_stylesheet;

    bool m_dragging;
    bool m_moved;
    QPoint m_init_pos;

    QMap<QString,QToolButton *> prop_map;
    QSignalMapper prop_mapper;

    KConfigGroup *m_config;
    QDesktopWidget *m_desktop;

    int m_timer_id;
    QList<Property> m_pending_reg_properties;

    QList<QAction *> m_extraActions;

    Plasma::Dialog::ResizeCorners m_resizeCorners;
    Plasma::Dialog::ResizeCorner m_resizeStartCorner;
    QMap<Plasma::Dialog::ResizeCorner, QRect> m_resizeAreas;
};

#endif // STATUSBARWIDGET_H
