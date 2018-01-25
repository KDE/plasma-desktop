/***************************************************************************
 *   Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company *
 *                      <info@kdab.com>                                    *
 *   Author: Andras Mantia <andras.mantia@kdab.com>                        *
 *           Work sponsored by the LiMux project of the city of Munich.    *
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
#include "screenmapper.h"

#include <QScreen>
#include <QTimer>

#include <Plasma/Corona>
#include <KConfig>
#include <KConfigGroup>

ScreenMapper *ScreenMapper::instance()
{
    static ScreenMapper *s_instance = new ScreenMapper();
    return s_instance;
}

ScreenMapper::ScreenMapper(QObject *parent)
    : QObject(parent)
    , m_screenMappingChangedTimer(new QTimer(this))

{
    connect(m_screenMappingChangedTimer, &QTimer::timeout,
            this, &ScreenMapper::screenMappingChanged);

    connect(this, &ScreenMapper::screenMappingChanged, this, [this] {
       if (!m_corona)
           return;

       auto config = m_corona->config();
       KConfigGroup group(config, QLatin1String("ScreenMapping"));
       group.writeEntry(QLatin1String("screenMapping"), screenMapping());
       config->sync();
    });

    // used to compress screenMappingChanged signals when addMapping is called multiple times,
    // eg. from FolderModel::filterAcceptRows. The timer interval is an arbitrary number,
    // that doesn't delay too much the signal, but still compresses as much as possible
    m_screenMappingChangedTimer->setInterval(100);
    m_screenMappingChangedTimer->setSingleShot(true);
}

void ScreenMapper::removeScreen(int screenId, const QUrl &screenUrl)
{
    if (screenId < 0 || !m_availableScreens.contains(screenId))
        return;

    const auto screenPathWithScheme = screenUrl.url();
    // store the original location for the items
    auto it = m_screenItemMap.constBegin();
    while (it != m_screenItemMap.constEnd()) {
        const auto name = it.key();
        if (it.value() == screenId && name.url().startsWith(screenPathWithScheme)) {
            m_itemsOnDisabledScreensMap[screenId].append(name);
        }
        ++it;
    }

    m_availableScreens.removeAll(screenId);

    const auto newFirstScreen = std::min_element(m_availableScreens.constBegin(), m_availableScreens.constEnd());
    auto pathIt = m_screensPerPath.find(screenUrl);
    if (pathIt != m_screensPerPath.end() && pathIt.value() > 0) {
        int firstScreen = m_firstScreenForPath.value(screenUrl, -1);
        if (firstScreen == screenId) {
            m_firstScreenForPath[screenUrl] = (newFirstScreen == m_availableScreens.constEnd()) ? -1 : *newFirstScreen;
        }
        *pathIt = pathIt.value() - 1;
    } else if (screenUrl.isEmpty()) {
        // The screen got completely removed, not only its path changed.
        // If the removed screen was the first screen for a desktop path, the first screen for that path
        // needs to be updated.
        for (auto it = m_firstScreenForPath.begin(); it != m_firstScreenForPath.end(); ++it) {
            if (*it == screenId) {
                *it = *newFirstScreen;

                // we have now the path for the screen that was removed, so adjust it
                pathIt = m_screensPerPath.find(it.key());
                if (pathIt != m_screensPerPath.end()) {
                    *pathIt = pathIt.value() - 1;
                }
            }
        }
    }

    emit screensChanged();
}

void ScreenMapper::addScreen(int screenId, const QUrl &screenUrl)
{
    if (screenId < 0 || m_availableScreens.contains(screenId))
        return;

    const auto screenPathWithScheme = screenUrl.url();
    const bool isEmpty = (screenUrl.isEmpty() || screenUrl.path() == QLatin1String("/"));
    // restore the stored locations
    auto it = m_itemsOnDisabledScreensMap.find(screenId);
    if (it != m_itemsOnDisabledScreensMap.end()) {
        auto items = it.value();
        for (const auto &name: it.value()) {
            // add the items to the new screen, if they are on a disabled screen and their
            // location is below the new screen's path
            if (isEmpty || name.url().startsWith(screenPathWithScheme)) {
                addMapping(name, screenId, DelayedSignal);
                items.removeAll(name);
            }
        }
        if (items.isEmpty()) {
            m_itemsOnDisabledScreensMap.erase(it);
        } else {
            *it = items;
        }
    }

    m_availableScreens.append(screenId);

    // path is empty when a new screen appears that has no folderview base path associated with
    if (!screenUrl.isEmpty()) {
        auto it = m_screensPerPath.find(screenUrl);
        int firstScreen = m_firstScreenForPath.value(screenUrl, -1);
        if (firstScreen == -1 || screenId < firstScreen) {
            m_firstScreenForPath[screenUrl] = screenId;
        }
        if (it == m_screensPerPath.end()) {
            m_screensPerPath[screenUrl] = 1;
        } else {
            *it = it.value() + 1;
        }
    }

    emit screensChanged();
}

void ScreenMapper::addMapping(const QUrl &url, int screen, MappingSignalBehavior behavior)
{
    m_screenItemMap[url] = screen;
    if (behavior == DelayedSignal) {
        m_screenMappingChangedTimer->start();
    } else {
        emit screenMappingChanged();
    }
}

void ScreenMapper::removeFromMap(const QUrl &url)
{
    m_screenItemMap.remove(url);
    m_screenMappingChangedTimer->start();
}

int ScreenMapper::firstAvailableScreen(const QUrl &screenUrl) const
{
    return m_firstScreenForPath.value(screenUrl, -1);
}

void ScreenMapper::removeItemFromDisabledScreen(const QUrl &url)
{
    for (auto it = m_itemsOnDisabledScreensMap.begin();
         it != m_itemsOnDisabledScreensMap.end(); ++it) {
        auto urls = &(*it);
        urls->removeAll(url);
    }
}

#ifdef BUILD_TESTING
void ScreenMapper::cleanup()
{
    m_screenItemMap.clear();
    m_itemsOnDisabledScreensMap.clear();
    m_firstScreenForPath.clear();
    m_screensPerPath.clear();
    m_availableScreens.clear();
}
#endif

void ScreenMapper::setCorona(Plasma::Corona *corona)
{
    if (m_corona != corona) {
        Q_ASSERT(!m_corona);

        m_corona = corona;
        if (m_corona) {
            connect(m_corona, &Plasma::Corona::screenRemoved, this, [this] (int screenId) {
                removeScreen(screenId, {});
            });
            connect(m_corona, &Plasma::Corona::screenAdded, this, [this] (int screenId) {
                addScreen(screenId, {});
            });

            auto config = m_corona->config();
            KConfigGroup group(config, QLatin1String("ScreenMapping"));
            const QStringList mapping = group.readEntry(QLatin1String("screenMapping"), QStringList{});
            setScreenMapping(mapping);
        }
    }
}

QStringList ScreenMapper::screenMapping() const
{
    QStringList result;
    result.reserve(m_screenItemMap.count() * 2);
    auto it = m_screenItemMap.constBegin();
    while (it != m_screenItemMap.constEnd()) {
        result.append(it.key().toString());
        result.append(QString::number(it.value()));
        ++it;
    }

    return result;
}

void ScreenMapper::setScreenMapping(const QStringList &mapping)
{
    QHash<QUrl, int> newMap;
    const int count = mapping.count();
    newMap.reserve(count / 2);
    for (int i = 0; i < count - 1; i += 2) {
        if (i + 1 < count) {
            const QUrl url = QUrl::fromUserInput(mapping[i], {}, QUrl::AssumeLocalFile);
            newMap[url] = mapping[i + 1].toInt();
        }
    }

    if (m_screenItemMap != newMap) {
        m_screenItemMap = newMap;
        emit screenMappingChanged();
    }
}

int ScreenMapper::screenForItem(const QUrl &url) const
{
    int screen = m_screenItemMap.value(url, -1);
    if (!m_availableScreens.contains(screen))
        screen = -1;

    return screen;
}

QUrl ScreenMapper::stringToUrl(const QString &path)
{
    return QUrl::fromUserInput(path, {}, QUrl::AssumeLocalFile);
}
