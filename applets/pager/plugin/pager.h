/***************************************************************************
 *   Copyright (C) 2007 by Daniel Laidig <d.laidig@gmx.de>                 *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
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

#ifndef PAGER_H
#define PAGER_H

#include <QGraphicsSceneHoverEvent>
#include <QList>

#include <Plasma/Applet>

#include "model.h"
#include <config-X11.h>

class QDesktopWidget;

class KColorScheme;
class KWindowInfo;
class KCModuleProxy;

namespace Plasma
{
    class FrameSvg;
}

class Pager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* model READ model CONSTANT)
    Q_PROPERTY(int currentDesktop READ currentDesktop NOTIFY currentDesktopChanged)
    Q_PROPERTY(bool showWindowIcons READ showWindowIcons WRITE setShowWindowIcons NOTIFY showWindowIconsChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QSizeF size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QSize preferredSize READ preferredSize NOTIFY preferredSizeChanged)
    Q_PROPERTY(Pager::CurrentDesktopSelected currentDesktopSelected READ currentDesktopSelected WRITE setCurrentDesktopSelected NOTIFY currentDesktopSelectedChanged)
    Q_PROPERTY(Pager::DisplayedText displayedText READ displayedText WRITE setDisplayedText NOTIFY displayedTextChanged)

    public:
        enum CurrentDesktopSelected {
            DoNothing = 0,
            ShowDesktop,
            ShowDashboard
        };
        Q_ENUMS(CurrentDesktopSelected)

        enum DisplayedText {
            Number = 0,
            Name,
            None
        };
        Q_ENUMS(DisplayedText)
        
        Pager(QObject *parent = 0);
        ~Pager();

        QObject *model() const { return m_pagerModel; }

        int currentDesktop() const { return m_currentDesktop; }
        void setCurrentDesktop(int desktop);

        bool showWindowIcons() const { return m_showWindowIcons; }
        void setShowWindowIcons(bool show);

        Qt::Orientation orientation() const;
        void setOrientation(Qt::Orientation orientation);

        QSizeF size() const;
        void setSize(const QSizeF &size);

        QSize preferredSize() const;

        CurrentDesktopSelected currentDesktopSelected() const;
        void setCurrentDesktopSelected(CurrentDesktopSelected cur);

        DisplayedText displayedText() const;
        void setDisplayedText(DisplayedText disp);

        Q_INVOKABLE void moveWindow(int, double, double, int, int);
        Q_INVOKABLE void changeDesktop(int desktopId);

    Q_SIGNALS:
        void currentDesktopChanged();
        void showWindowIconsChanged();
        void orientationChanged();
        void sizeChanged();
        void preferredSizeChanged();
        void currentDesktopSelectedChanged();
        void displayedTextChanged();

    public Q_SLOTS:
        void recalculateGridSizes(int rows);
        void updateSizes();
        void recalculateWindowRects();
        void openVirtualDesktopsKCM();

    protected Q_SLOTS:
        void currentDesktopChanged(int desktop);
        void currentActivityChanged(const QString &activity);
        void desktopsSizeChanged();
        void numberOfDesktopsChanged(int num);
        void desktopNamesChanged();
        void windowChanged(WId id, const unsigned long *dirty);
        void startTimer();
        void startTimerFast();
#if HAVE_X11
        void slotAddDesktop();
        void slotRemoveDesktop();
#endif

    protected:
        void createMenu();
        QRect fixViewportPosition( const QRect& r );

    private:

        PagerModel *m_pagerModel;

        QTimer* m_timer;


        DisplayedText m_displayedText;
        CurrentDesktopSelected m_currentDesktopSelected;
        int m_rows;
        int m_columns;
        int m_desktopCount;
        int m_currentDesktop;
        QString m_currentActivity;
        qreal m_widthScaleFactor;
        qreal m_heightScaleFactor;
        QSizeF m_size;
        QSize m_preferredSize;
        Qt::Orientation m_orientation;

        bool m_showWindowIcons : 1;
        bool m_desktopDown : 1;
        bool m_validSizes : 1;

        QDesktopWidget *m_desktopWidget;
#if HAVE_X11
        bool m_isX11;
#endif
    };

#endif
