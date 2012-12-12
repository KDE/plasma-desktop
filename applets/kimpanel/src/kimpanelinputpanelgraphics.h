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

#ifndef KIMPANEL_INPUTPANEL_GRAPHICS_H
#define KIMPANEL_INPUTPANEL_GRAPHICS_H

// Qt
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QSignalMapper>

class DummyWidget;
class KimpanelLabelGraphics;
namespace Plasma
{
class IconWidget;
}

class QBitmap;
class KimpanelInputPanelGraphics : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit KimpanelInputPanelGraphics(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~KimpanelInputPanelGraphics();

    // virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    void setShowPreedit(bool show);
    void setShowAux(bool show);
    void setShowLookupTable(bool show);
    void setLookupTableCursor(int cursor);
    void setLookupTableLayout(int layout);
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
    QSize roundSize();
    void setReverse(bool reverse, bool force = false);

public:
    void updateSize();
    void updateVisible();
    bool markedVisible();

Q_SIGNALS:
    void selectCandidate(int idx);
    void lookupTablePageUp();
    void lookupTablePageDown();

    void sizeChanged();
protected:
    virtual void resizeEvent(QGraphicsSceneResizeEvent* event);

private slots:
    void loadSettings();
    void updateDummyWidget();

private:
    void clearLookupTable();
    void updateLookupTable();
    void updateOrientation();
    QGraphicsLinearLayout *m_layout;
    QGraphicsLinearLayout *m_upperLayout;
    QGraphicsLinearLayout *m_lookupTableLayout;
    QGraphicsLinearLayout *m_pageButtonLayout;
    QGraphicsLinearLayout *m_lowerLayout;

    QSignalMapper mapper;

    QPoint m_pointPos;
    bool m_moving;

    bool preeditVisible;
    bool auxVisible;
    bool lookuptableVisible;

    QString m_text;
    int m_cursorPos;
    QString m_auxText;
    QStringList m_labels;
    QStringList m_candidates;
    bool m_hasPrev;
    bool m_hasNext;
    QColor m_candidateColor;
    QColor m_preEditColor;
    QColor m_labelColor;

    KimpanelLabelGraphics *m_auxLabel;
    KimpanelLabelGraphics *m_preeditLabel;
    Plasma::IconWidget *m_pageUpIcon;
    Plasma::IconWidget *m_pageDownIcon;
    DummyWidget* m_dummyWidget;
    QList<KimpanelLabelGraphics *> m_tableEntryLabels;

    QSignalMapper *m_tableEntryMapper;
    bool m_lastVisible;
    bool m_reverse;
    int m_lookupTableCursor;
    int m_fontHeight;
    bool m_useVertical;
    bool m_useReverse;
    Qt::Orientation m_orientaion;
    int m_lookupTableOrientation;
};

#endif // KIMPANEL_INPUTPANEL_GRAHICS_H

