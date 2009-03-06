#ifndef KIMPANELLAYOUT_H
#define KIMPANELLAYOUT_H

#include <plasma/applet.h>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsLayoutItem>
#include <QList>
#include <KIcon>
#include <KDialog>

class KIMPanelLayout : public QGraphicsLayout
{
public:
    KIMPanelLayout(QGraphicsLayoutItem *parent);
    void addItem(Plasma::IconWidget *icon);
    void addItems(const QList<Plasma::IconWidget *> &icons);
    void setItems(const QList<Plasma::IconWidget *> &icons);

    Plasma::IconWidget* itemAt(int i) const;
    void removeAt(int i);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    int count() const 
    {
        return m_icons.size();
    }

    void setGeometry(const QRectF &rect);
//X     void updateGeometry();

private:
    void smartLayout(const QRectF &rect);

private:
    QList<Plasma::IconWidget *> m_icons;

    QRectF m_cachedGeometry;

};

#endif // KIMPANELLAYOUT_H
