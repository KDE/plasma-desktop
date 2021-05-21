/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testbutton.h"

#include <QMouseEvent>
#include <QTimer>

#include <KLocalizedString>

TestButton::TestButton(QWidget *parent)
    : QPushButton(parent)
    , m_firstClick(true)
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
