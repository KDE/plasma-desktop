/*
 *   Copyright (C) 2015 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H
#define UTILS_H

#include <QGuiApplication>

inline std::unique_ptr<QQuickView> createView(QWidget *parent)
{
    auto view = new QQuickView();
    view->setColor(QGuiApplication::palette().window().color());
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    auto container = QWidget::createWindowContainer(view, parent);
    container->setFocusPolicy(Qt::TabFocus);

    parent->layout()->addWidget(container);

    return std::unique_ptr<QQuickView>(view);
}

template <typename View>
inline bool setViewSource(View &view, const QString &file)
{
    QString sourceFile = QStringLiteral(KAMD_KCM_DATADIR) + file;

    if (QFile::exists(sourceFile)) {
        view->setSource(sourceFile);

        return true;

    } else {
        return false;
    }

}

#endif // UTILS_H

