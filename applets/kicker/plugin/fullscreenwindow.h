/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
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

#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H

#include <Plasma/Theme>

#include <QQuickWindow>
#include <QQuickItem>

class FullScreenWindow : public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem* mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)
    Q_PROPERTY(QQuickItem* visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

    Q_CLASSINFO("DefaultProperty", "mainItem")

    public:
        FullScreenWindow(QQuickItem *parent = 0);
        ~FullScreenWindow();

        QQuickItem *mainItem() const;
        void setMainItem(QQuickItem *item);

        QQuickItem *visualParent() const;
        void setVisualParent(QQuickItem *item);

        Q_INVOKABLE void toggle();

    Q_SIGNALS:
        void mainItemChanged() const;
        void visualParentChanged() const;

    private Q_SLOTS:
        void updateTheme();
        void visualParentWindowChanged(QQuickWindow *window);
        void visualParentScreenChanged(QScreen *screen);

    protected:
        void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
        void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;

    private:
        QQuickItem *m_mainItem;
        QPointer<QQuickItem> m_visualParentItem;
        QPointer<QQuickWindow> m_visualParentWindow;
        Plasma::Theme m_theme;
};

#endif
