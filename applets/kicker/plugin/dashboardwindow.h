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

#ifndef DASHBOARDWINDOW_H
#define DASHBOARDWINDOW_H

#include <Plasma/Theme>

#include <QQuickWindow>
#include <QQuickItem>

class DashboardWindow : public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem* mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)
    Q_PROPERTY(QQuickItem* visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)
    Q_PROPERTY(QQuickItem* keyEventProxy READ keyEventProxy WRITE setKeyEventProxy NOTIFY keyEventProxyChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

    Q_CLASSINFO("DefaultProperty", "mainItem")

    public:
        explicit DashboardWindow(QQuickItem *parent = nullptr);
        ~DashboardWindow() override;

        QQuickItem *mainItem() const;
        void setMainItem(QQuickItem *item);

        QQuickItem *visualParent() const;
        void setVisualParent(QQuickItem *item);

        QQuickItem *keyEventProxy() const;
        void setKeyEventProxy(QQuickItem *item);

        QColor backgroundColor() const;
        void setBackgroundColor(const QColor &color);

        Q_INVOKABLE void toggle();

    Q_SIGNALS:
        void mainItemChanged() const;
        void visualParentChanged() const;
        void keyEventProxyChanged() const;
        void backgroundColorChanged() const;
        void keyEscapePressed() const;

    private Q_SLOTS:
        void updateTheme();
        void visualParentWindowChanged(QQuickWindow *window);
        void visualParentScreenChanged(QScreen *screen);

    protected:
        bool event(QEvent *event) override;
        void keyPressEvent(QKeyEvent *e) override;

    private:
        QQuickItem *m_mainItem;
        QPointer<QQuickItem> m_visualParentItem;
        QPointer<QQuickWindow> m_visualParentWindow;
        QPointer<QQuickItem> m_keyEventProxy;
        Plasma::Theme m_theme;
};

#endif
