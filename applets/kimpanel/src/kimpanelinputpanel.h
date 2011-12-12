/***************************************************************************
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

#ifndef KIMPANEL_INPUTPANEL_H
#define KIMPANEL_INPUTPANEL_H

// Qt
#include <QWidget>

class QGraphicsScene;
namespace Plasma
{
class FrameSvg;
}

class QHBoxLayout;
class KimpanelInputPanelGraphics;
class QGraphicsView;
class KimpanelInputPanel : public QWidget
{
    Q_OBJECT
public:
    KimpanelInputPanel(QWidget* parent = 0);
    virtual ~KimpanelInputPanel();

    void setSpotLocation(int x, int y);
    void setSpotLocation(const QPoint& point);
    void setShowPreedit(bool show);
    void setShowAux(bool show);
    void setShowLookupTable(bool show);
    void setPreeditCaret(int pos);
    void setPreeditText(const QString& text,
                        const QString& attrs = QString());
    void setAuxText(const QString& text,
                    const QString& attrs = QString());
    void setLookupTable(const QStringList& labels,
                        const QStringList& candidates,
                        bool hasPrev,
                        bool hasNext,
                        const QStringList& attrs = QStringList()
                       );

Q_SIGNALS:
    void lookupTablePageUp();
    void lookupTablePageDown();
    void selectCandidate(int index);
protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);
private Q_SLOTS:
    void loadTheme();
    void maskBackground(bool);
    void updateVisible(bool);
    void updateSize();
private:
    QHBoxLayout* m_layout;
    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    KimpanelInputPanelGraphics* m_widget;
    Plasma::FrameSvg* m_backgroundSvg;
    QPoint m_pointPos;
    bool m_moving;
};

#endif // KIMPANEL_INPUTPANEL_H

