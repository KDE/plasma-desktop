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
#ifndef KIMPANELAPPLET_H
#define KIMPANELAPPLET_H

#include <plasma/applet.h>
#include <plasma/dialog.h>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsLayoutItem>
#include <QList>

#include "kimpanelagent.h"
#include "kimstatusbar.h"
#include "kimstatusbargraphics.h"
#include "kimlookuptable.h"

class KIMPanelApplet : public Plasma::Applet
{
Q_OBJECT
public:
    KIMPanelApplet(QObject *parent, const QVariantList &args);
    ~KIMPanelApplet();

    /**
     * Returns hints about the geometry of the figure
     * @return Hints about proportionality of the applet
     */
//X     QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    /**
     * Returns info about if we want to expand or not
     * For now, we never need to expand
     */
    Qt::Orientations expandingDirections() const { return 0; }

    /**
     * List of actions to add in context menu
     * @return List of QAction pointers
     */
    virtual QList<QAction*> contextualActions();

Q_SIGNALS:
    void triggerProperty(const QString &key);
    void sizeHintChanged(Qt::SizeHint);

public Q_SLOTS:
    void createConfigurationInterface(KConfigDialog *parent);

protected:
    /**
     * Overloaded method to save the state on exit
     */
    void saveState(KConfigGroup &config) const;

    /**
     * Called when something in the geometry has changed
     */
    void constraintsEvent(Plasma::Constraints constraints);

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);

protected Q_SLOTS:
    /**
     * Called when the user has clicked OK or apply in the Config interface
     */
    void configAccepted();
    void themeUpdated();

private Q_SLOTS:
    void adjustSelf();
    void toggleCollapse(bool b);

private:
    void init();

    /**
     * Removes all items from a BoxLayout
     * @param layout Layout to clear
     */
    void clearLayout(QGraphicsLayout *layout);

    /**
     * Saves icons into plasma applet config file
     */
    void saveConfig() {}

    QGraphicsLinearLayout *m_layout;

    KIMStatusBar *m_statusbar;
    KIMStatusBarGraphics *m_statusbarGraphics;

    KIMLookupTable *m_lookup_table;

    Plasma::FrameSvg *m_background;

    Plasma::IconWidget *m_logoIcon;

    int m_preferIconWidth;

    PanelAgent *m_panel_agent;
};

#endif
