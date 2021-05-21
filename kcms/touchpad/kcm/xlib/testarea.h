/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TESTAREA_H
#define TESTAREA_H

#include "ui_testarea.h"

class TestArea : public QWidget
{
    Q_OBJECT
public:
    explicit TestArea(QWidget *);

Q_SIGNALS:
    void enter();
    void leave();

protected:
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    Ui::TestArea m_ui;
};

#endif // TESTAREA_H
