/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "screenmapper.h"

#include <QScreen>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <Plasma/Corona>

ScreenMapper *ScreenMapper::instance()
{
    static ScreenMapper *s_instance = new ScreenMapper();
    return s_instance;
}

ScreenMapper::ScreenMapper(QObject *parent)
    : QObject(parent)
    , m_screenMappingChangedTimer(new QTimer(this))

{
    connect(m_screenMappingChangedTimer, &QTimer::timeout, this, &ScreenMapper::screenMappingChanged);

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
    QVector<QUrl> urlsToRemoveFromMapping;
    while (it != m_screenItemMap.constEnd()) {
        const auto name = it.key();
        if (it.value() == screenId && name.url().startsWith(screenPathWithScheme)) {
            bool found = false;
            for (const auto &disabledUrls : qAsConst(m_itemsOnDisabledScreensMap)) {
                found = disabledUrls.contains(name);
                if (found)
                    break;
            }
            if (!found)
                m_itemsOnDisabledScreensMap[screenId].append(name);
            urlsToRemoveFromMapping.append(name);
        }
        ++it;
    }

    saveDisabledScreensMap();

    for (const auto &url : urlsToRemoveFromMapping)
        removeFromMap(url);

    m_availableScreens.removeAll(screenId);

    auto pathIt = m_screensPerPath.find(screenUrl);
    if (pathIt != m_screensPerPath.end() && pathIt.value().size() > 0) {
        // remove the screen for a certain url, used when switching the URL for a screen
        pathIt->removeAll(screenId);
    } else if (screenUrl.isEmpty()) {
        // the screen was indeed removed so all references to it needs to be cleaned up
        for (auto &pathIt : m_screensPerPath) {
            pathIt.removeAll(screenId);
        }
    }

    Q_EMIT screensChanged();
}

void ScreenMapper::addScreen(int screenId, const QUrl &screenUrl)
{
    if (screenId < 0 || m_availableScreens.contains(screenId))
        return;

    const auto screenPathWithScheme = screenUrl.url();
    // restore the stored locations
    auto it = m_itemsOnDisabledScreensMap.find(screenId);
    if (it != m_itemsOnDisabledScreensMap.end()) {
        auto items = it.value();
        for (const auto &name : it.value()) {
            // add the items to the new screen, if they are on a disabled screen and their
            // location is below the new screen's path
            if (name.url().startsWith(screenPathWithScheme)) {
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
    saveDisabledScreensMap();

    m_availableScreens.append(screenId);

    // path is empty when a new screen appears that has no folderview base path associated with
    if (!screenUrl.isEmpty()) {
        auto it = m_screensPerPath.find(screenUrl);
        if (it == m_screensPerPath.end()) {
            m_screensPerPath[screenUrl] = {screenId};
        } else {
            it->append(screenId);
        }
    }

    Q_EMIT screensChanged();
}

void ScreenMapper::addMapping(const QUrl &url, int screen, MappingSignalBehavior behavior)
{
    m_screenItemMap[url] = screen;
    if (behavior == DelayedSignal) {
        m_screenMappingChangedTimer->start();
    } else {
        Q_EMIT screenMappingChanged();
    }
}

void ScreenMapper::removeFromMap(const QUrl &url)
{
    m_screenItemMap.remove(url);
    m_screenMappingChangedTimer->start();
}

int ScreenMapper::firstAvailableScreen(const QUrl &screenUrl) const
{
    auto screens = m_screensPerPath[screenUrl];
    const auto newFirstScreen = std::min_element(screens.constBegin(), screens.constEnd());
    return newFirstScreen == screens.constEnd() ? -1 : *newFirstScreen;
}

void ScreenMapper::removeItemFromDisabledScreen(const QUrl &url)
{
    for (auto it = m_itemsOnDisabledScreensMap.begin(); it != m_itemsOnDisabledScreensMap.end(); ++it) {
        auto urls = &(*it);
        urls->removeAll(url);
    }
}

void ScreenMapper::setSharedDesktop(bool sharedDesktops)
{
    if (m_sharedDesktops != sharedDesktops) {
        m_sharedDesktops = true;
        if (!m_corona)
            return;

        auto config = m_corona->config();
        KConfigGroup group(config, QLatin1String("ScreenMapping"));
        group.writeEntry(QLatin1String("sharedDesktops"), m_sharedDesktops);
    }
}

#ifdef BUILD_TESTING
void ScreenMapper::cleanup()
{
    m_screenItemMap.clear();
    m_itemsOnDisabledScreensMap.clear();
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
            connect(m_corona, &Plasma::Corona::screenRemoved, this, [this](int screenId) {
                removeScreen(screenId, {});
            });
            connect(m_corona, &Plasma::Corona::screenAdded, this, [this](int screenId) {
                addScreen(screenId, {});
            });

            auto config = m_corona->config();
            KConfigGroup group(config, QLatin1String("ScreenMapping"));
            const QStringList mapping = group.readEntry(QLatin1String("screenMapping"), QStringList{});
            setScreenMapping(mapping);
            m_sharedDesktops = group.readEntry(QLatin1String("sharedDesktops"), false);
            readDisabledScreensMap();
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
        Q_EMIT screenMappingChanged();
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

void ScreenMapper::readDisabledScreensMap()
{
    if (!m_corona)
        return;

    auto config = m_corona->config();
    KConfigGroup group(config, QLatin1String("ScreenMapping"));
    const QStringList serializedMap = group.readEntry(QLatin1String("itemsOnDisabledScreens"), QStringList{});
    m_itemsOnDisabledScreensMap.clear();
    bool readingScreenId = true;
    int vectorSize = -1;
    int screenId = -1;
    int vectorCounter = 0;
    for (const auto &entry : serializedMap) {
        if (readingScreenId) {
            screenId = entry.toInt();
            readingScreenId = false;
        } else if (vectorSize == -1) {
            vectorSize = entry.toInt();
        } else {
            const auto url = stringToUrl(entry);
            m_itemsOnDisabledScreensMap[screenId].append(url);
            vectorCounter++;
            if (vectorCounter == vectorSize) {
                readingScreenId = true;
                screenId = -1;
                vectorCounter = 0;
                vectorSize = -1;
            }
        }
    }
}

void ScreenMapper::saveDisabledScreensMap() const
{
    if (!m_corona)
        return;

    auto config = m_corona->config();
    KConfigGroup group(config, QLatin1String("ScreenMapping"));
    QStringList serializedMap;
    auto it = m_itemsOnDisabledScreensMap.constBegin();
    for (; it != m_itemsOnDisabledScreensMap.constEnd(); ++it) {
        serializedMap.append(QString::number(it.key()));
        const auto urls = it.value();
        serializedMap.append(QString::number(urls.size()));
        for (const auto &url : urls) {
            serializedMap.append(url.toString());
        }
    }

    group.writeEntry(QLatin1String("itemsOnDisabledScreens"), serializedMap);
}
