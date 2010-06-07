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

#ifndef KIM_STATUSBAR_H
#define KIM_STATUSBAR_H

#include <plasma/dialog.h>
#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <plasma/corona.h>

#include <KIconLoader>
#include <KConfigGroup>

#include <QToolButton>
#include <QPixmap>
#include <QBitmap>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include "kimagenttype.h"
#include "kimstatusbargraphics.h"
#include "kimpanelruntime_export.h"

class KIMStatusBarGraphics;

class KIMPANELRUNTIME_EXPORT KIMStatusBar : public QWidget
{
Q_OBJECT
public:
    explicit KIMStatusBar(Plasma::Corona *corona=0, QWidget *parent=0, const QList<QAction *> extra_actions = QList<QAction *>());
    ~KIMStatusBar();
    
//X     void setEnabled(bool to_enable);

    void setGraphicsWidget(KIMStatusBarGraphics *widget);
    KIMStatusBarGraphics *graphicsWidget() 
    {
        return m_widget;
    }

    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void triggerProperty(const QString &key);

private Q_SLOTS:
    void themeUpdated();
    void adjustSelf();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool event(QEvent *e);

private:
    Plasma::FrameSvg *m_background;
    QBitmap m_mask;

    Plasma::Corona *m_scene;
    QGraphicsView *m_view;
    KIMStatusBarGraphics *m_widget;

    QVBoxLayout *m_layout;

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

};

#endif // KIM_STATUSBAR_H
