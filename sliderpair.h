#ifndef SLIDERPAIR_H
#define SLIDERPAIR_H

#include <QSlider>

class SliderPair : public QObject
{
    Q_OBJECT
public:
    SliderPair(QSlider *minSlider, QSlider *maxSlider, QObject *parent = 0);

private Q_SLOTS:
    void adjustMinSlider();
    void adjustMaxSlider();

private:
    QSlider *m_minSlider, *m_maxSlider;
};

#endif // SLIDERPAIR_H
