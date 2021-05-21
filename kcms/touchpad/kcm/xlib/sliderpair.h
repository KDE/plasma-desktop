/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SLIDERPAIR_H
#define SLIDERPAIR_H

#include <QObject>

class QSlider;

class SliderPair : public QObject
{
    Q_OBJECT
public:
    SliderPair(QSlider *minSlider, QSlider *maxSlider, QObject *parent = nullptr);

private Q_SLOTS:
    void adjustMinSlider();
    void adjustMaxSlider();

private:
    QSlider *m_minSlider, *m_maxSlider;
};

#endif // SLIDERPAIR_H
