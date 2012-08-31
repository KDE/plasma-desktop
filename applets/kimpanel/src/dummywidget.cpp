#include "dummywidget.h"
#include <QPainter>
#include <QStyleOption>

DummyWidget::DummyWidget(QGraphicsWidget* parent) : QGraphicsWidget(parent)
{
    setMinimumSize(0, 0);
    setMaximumSize(INT_MAX, 1);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DummyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->fillRect(option->rect, QBrush(Qt::transparent));
    // painter->fillRect(option->rect, QBrush(Qt::blue));
}
