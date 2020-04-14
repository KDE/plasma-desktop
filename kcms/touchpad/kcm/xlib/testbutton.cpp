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

#include "testbutton.h"

#include <QMouseEvent>
#include <QTimer>

#include <KLocalizedString>

TestButton::TestButton(QWidget *parent)
    : QPushButton(parent), m_firstClick(true)
{
}

void TestButton::mousePressEvent(QMouseEvent *e)
{
    if (m_firstClick) {
        m_originalText = text();
        m_firstClick = false;
    }

    switch (e->button()) {
    case Qt::LeftButton:
        setText(i18nc("Mouse button", "Left button"));
        break;
    case Qt::RightButton:
        setText(i18nc("Mouse button", "Right button"));
        break;
    case Qt::MiddleButton:
        setText(i18nc("Mouse button", "Middle button"));
        break;
    default:
        break;
    }

    QTimer::singleShot(500, this, &TestButton::resetText);

    QPushButton::mousePressEvent(e);
}

void TestButton::resetText()
{
    setText(m_originalText);
}
