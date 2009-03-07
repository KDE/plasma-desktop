#include "kimpanellayout.h"
#include <kiconloader.h>

KIMPanelLayout::KIMPanelLayout(QGraphicsLayoutItem *parent) : QGraphicsLayout(parent) 
{
}

void KIMPanelLayout::addItem(Plasma::IconWidget *icon)
{
    m_icons << icon;
//    QGraphicsGridLayout::addItem(icon,0,count());
    updateGeometry();
    //smartLayout(m_cachedGeometry);
}

void KIMPanelLayout::addItems(const QList<Plasma::IconWidget *> &icons)
{
    m_icons << icons;
    updateGeometry();
    //smartLayout(m_cachedGeometry);
}

void KIMPanelLayout::setItems(const QList<Plasma::IconWidget *> &icons)
{
    m_icons = icons;
    updateGeometry();
    //smartLayout(m_cachedGeometry);
}

Plasma::IconWidget* KIMPanelLayout::itemAt(int i) const
{
    return m_icons.at(i);
}

void KIMPanelLayout::removeAt(int i)
{
    m_icons.removeAt(i);
//X     smartLayout(m_cachedGeometry);
    updateGeometry();
}

QSizeF KIMPanelLayout::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF sizeHint;
    switch (which) {
    case Qt::MinimumSize:
//X         return QSizeF(left + right + KIconLoader::SizeSmall,
//X             top + bottom + KIconLoader::SizeSmall);
        sizeHint = QSizeF(KIconLoader::SizeSmall,KIconLoader::SizeSmall);
        break;
    case Qt::PreferredSize:
        sizeHint = m_cachedGeometry.size();
        break;
    case Qt::MaximumSize:
        return QSizeF(-1.0,-1.0);
        break;
    default:
        ;
    }
    kDebug() << which << sizeHint << constraint;
    return sizeHint;
}

void KIMPanelLayout::setGeometry(const QRectF &rect)
{
    smartLayout(rect);
    m_cachedGeometry = rect;
    kDebug() << rect;
}

#if 0
void KIMPanelLayout::updateGeometry()
{
    kDebug() << "";
    QGraphicsGridLayout::updateGeometry();
}
#endif

void KIMPanelLayout::smartLayout(const QRectF &rect)
{
    if (m_icons.size() > 0) {
        qreal w;
        qreal h;
        int row;
        int best_row;
        int best_col = 1;
        int max_w = -1;
        for (int col = 1; col <= m_icons.size(); col++) {
            w = rect.width()/col;
            row = (m_icons.size() + col - 1) / col;
            h = rect.height()/row;

            if (qMin(w,h) >= max_w) {
                max_w = qMin(w,h);
                best_col = col;
                best_row = row;
            }
        }
//X         kDebug() << rect << "Best:" <<  best_row << best_col << max_w;
        // do the layout
        int r = 0;
        int c = 0;
        foreach (Plasma::IconWidget *icon, m_icons) {
            QRectF new_geometry;
            new_geometry.setWidth(max_w);
            new_geometry.setHeight(max_w);
            new_geometry.moveTop(r*max_w + rect.top());
            new_geometry.moveLeft(c*max_w + rect.left());
            icon->setGeometry(new_geometry);
            c++;
            if (c>=best_col) {
                c=0;
                r++;
            }
        }
    }
}

// vim: sw=4 sts=4 et tw=100
