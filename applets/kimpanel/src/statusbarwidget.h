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

#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/framesvg.h>
#include <kiconloader.h>
#include <QtGui>

#include "kimpaneltype.h"

class KConfigGroup;
class QDesktopWidget;

class StatusBarWidget : public QWidget
{
Q_OBJECT
public:
    StatusBarWidget(QWidget *parent=0);
    ~StatusBarWidget();
    
    void setEnabled(bool to_enable);

Q_SIGNALS:
    void triggerProperty(const QString &key);

public Q_SLOTS:
    void updateAux(const QString &text,const QList<TextAttribute> &attrs);
    void updateProperty(const Property &prop);
    void registerProperties(const QList<Property> &props);
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
    Plasma::FrameSvg *m_background_svg;

    QList<Plasma::Svg *> m_properties_svg;
    QList<Property> m_properties;

    QString m_button_stylesheet;

    QBoxLayout *m_layout;

    bool m_dragging;
    QPoint m_init_pos;

    QMap<QString,QToolButton *> prop_map;
    QSignalMapper prop_mapper;

    KConfigGroup *m_config;
    QDesktopWidget *m_desktop;

    int m_timer_id;
    QList<Property> m_pending_reg_properties;
};

#endif // STATUSBARWIDGET_H
