/***************************************************************************
 *   Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company *
 *                      <info@kdab.com>                                    *
 *   Author: Andras Mantia <andras.mantia@kdab.com>                        *
 *           Work sponsored by the LiMux project of the city of Munich.    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#ifndef SCREENMAPPER_H
#define SCREENMAPPER_H

#include <QObject>
#include <QVariantHash>
#include <QVector>

#include "folderplugin_private_export.h"

class QTimer;

namespace Plasma {
    class Corona;
}

class FOLDERPLUGIN_TESTS_EXPORT ScreenMapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList screenMapping READ screenMapping WRITE setScreenMapping NOTIFY screenMappingChanged)

public:
    enum MappingSignalBehavior {
        DelayedSignal = 0,
        ImmediateSignal
    };

    static ScreenMapper *instance();
    ~ScreenMapper() override = default;

    QStringList screenMapping() const;
    void setScreenMapping(const QStringList &mapping);

    int screenForItem(const QString &name) const;
    void addMapping(const QString &name, int screen, MappingSignalBehavior behavior = ImmediateSignal);
    void removeFromMap(const QString &name);
    void setCorona(Plasma::Corona *corona);

    void addScreen(int screenId, const QString &path);
    void removeScreen(int screenId, const QString &path);
    int firstAvailableScreen(const QString &path) const;
    void removeItemFromDisabledScreen(const QString &name);

#ifdef BUILD_TESTING
    void cleanup();
#endif

Q_SIGNALS:
    void screenMappingChanged() const;
    void screensChanged() const;

private:
    ScreenMapper(QObject *parent = nullptr);

    QHash<QString, int> m_screenItemMap;
    QHash<int, QStringList> m_itemsOnDisabledScreensMap;
    QHash<QString, int> m_firstScreenForPath; // first available screen for a path
    QHash<QString, int> m_screensPerPath; // screen per registered path
    QVector<int> m_availableScreens;
    Plasma::Corona *m_corona = nullptr;
    QTimer *m_screenMappingChangedTimer = nullptr;
};

#endif // SCREENMAPPER_H
