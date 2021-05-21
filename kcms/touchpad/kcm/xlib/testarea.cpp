/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testarea.h"

#include <QKeyEvent>
#include <QMouseEvent>

#include <KLocalizedString>
#include <Plasma/Theme>

TestArea::TestArea(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    m_ui.listWidget->addItem(new QListWidgetItem(QIcon::fromTheme("folder"), i18n("Drag me"), m_ui.listWidget));

    Plasma::Theme defaultTheme;
    QString wallpaper = defaultTheme.wallpaperPath();
    static const QString stylesheet("background-image: url(%1)");
    m_ui.scrollAreaWidgetContents->setStyleSheet(stylesheet.arg(wallpaper));
}

void TestArea::enterEvent(QEvent *e)
{
    Q_EMIT enter();
    QWidget::enterEvent(e);
}

void TestArea::leaveEvent(QEvent *e)
{
    Q_EMIT leave();
    QWidget::leaveEvent(e);
}
