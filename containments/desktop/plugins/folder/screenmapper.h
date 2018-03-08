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

public:
    enum MappingSignalBehavior {
        DelayedSignal = 0,
        ImmediateSignal
    };

    static ScreenMapper *instance();
    ~ScreenMapper() override = default;

    QStringList screenMapping() const;
    void setScreenMapping(const QStringList &mapping);

    int screenForItem(const QUrl &url) const;
    void addMapping(const QUrl &url, int screen, MappingSignalBehavior behavior = ImmediateSignal);
    void removeFromMap(const QUrl &url);
    void setCorona(Plasma::Corona *corona);

    void addScreen(int screenId, const QUrl &screenUrl);
    void removeScreen(int screenId, const QUrl &screenUrl);
    int firstAvailableScreen(const QUrl &screenUrl) const;
    void removeItemFromDisabledScreen(const QUrl &url);

    bool sharedDesktops() const { return m_sharedDesktops; }
    void setSharedDesktop(bool sharedDesktops);

#ifdef BUILD_TESTING
    void cleanup();
#endif

    static QUrl stringToUrl(const QString &path);

Q_SIGNALS:
    void screenMappingChanged() const;
    void screensChanged() const;

private:
    void saveDisabledScreensMap() const;
    void readDisabledScreensMap();

    ScreenMapper(QObject *parent = nullptr);

    QHash<QUrl, int> m_screenItemMap;
    QHash<int, QVector<QUrl> > m_itemsOnDisabledScreensMap;
    QHash<QUrl, QVector<int> > m_screensPerPath; // screens per registered path
    QVector<int> m_availableScreens;
    Plasma::Corona *m_corona = nullptr;
    QTimer *m_screenMappingChangedTimer = nullptr;
    bool m_sharedDesktops = false; // all screens share the same desktops, disabling the screen mapping
};

#endif // SCREENMAPPER_H
