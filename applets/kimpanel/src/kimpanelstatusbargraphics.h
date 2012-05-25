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
#ifndef KIMPANEL_STATUSBAR_GRAPHICS_H
#define KIMPANEL_STATUSBAR_GRAPHICS_H

#include "kimpanel/kimpanelagenttype.h"

// Qt
#include <QGraphicsWidget>
#include <QTimer>

// Plasma
#include <Plasma/IconWidget>
#include "icongridlayout.h"

class QSignalMapper;
class IconGridLayout;

class DelayedSignalContainer : public QObject
{
    Q_OBJECT
public:
    DelayedSignalContainer(const QString& key, QObject* parent = 0) : QObject(parent) {
        this->key = key;
    }

    void launch(int time) {
        QTimer::singleShot(time, this, SLOT(delay()));
    }

signals:
    void notify(QString key);

private slots:
    void delay() {
        emit notify(key);
        this->deleteLater();
    }

private:
    QString key;
};

class KimpanelStatusBarGraphics : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit KimpanelStatusBarGraphics(QGraphicsItem *parent = 0);
    ~KimpanelStatusBarGraphics();
    void updateProperties(const QVariant& var);
    void execMenu(const QVariant &var);
    QList<QAction *> contextualActions() const;
    void setLayoutMode(IconGridLayout::Mode mode);
    void setPreferredIconSize(int size);
Q_SIGNALS:
    void triggerProperty(QString property);
    void configure();
    void reloadConfig();
    void exitIM();
    void startIM();
protected Q_SLOTS:
    void hiddenActionToggled();
    void delayedTriggerProperty(const QString& key);
private:
    void updateIcon();
    IconGridLayout* m_layout;
    QList< KimpanelProperty > m_props;
    QMap<QString, Plasma::IconWidget*> m_iconMap;
    Plasma::IconWidget* m_startIMIcon;

    QSignalMapper *m_propertyMapper;

    QMenu* m_filterMenu;
    QAction *m_filterAction;
    QAction* m_configureAction;
    QAction *m_reloadConfigAction;
    QAction *m_exitAction;
    QSet<QString> m_hiddenProperties;
    QSizeF m_preferredIconSize;
    Plasma::Svg* m_svg;
};

#endif // KIMPANEL_STATUSBAR_GRAPHICS_H

