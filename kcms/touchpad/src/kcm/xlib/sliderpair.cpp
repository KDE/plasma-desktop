/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "sliderpair.h"

#include <QSlider>

SliderPair::SliderPair(QSlider *minSlider, QSlider *maxSlider, QObject *parent)
    : QObject(parent), m_minSlider(minSlider), m_maxSlider(maxSlider)
{
    connect(m_minSlider, &QAbstractSlider::valueChanged, this, &SliderPair::adjustMaxSlider);
    connect(m_maxSlider, &QAbstractSlider::valueChanged, this, &SliderPair::adjustMinSlider);
}

void SliderPair::adjustMaxSlider()
{
    m_maxSlider->setValue(qMax(m_maxSlider->value(), m_minSlider->value()));
}

void SliderPair::adjustMinSlider()
{
    m_minSlider->setValue(qMin(m_maxSlider->value(), m_minSlider->value()));
}
