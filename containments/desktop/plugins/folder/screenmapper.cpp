/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <optional>

#include "screenmapper.h"

#include <QMap>
#include <QScreen>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <Plasma/Corona>
#include <PlasmaActivities/Consumer>
#include <chrono>

#include "debug.h"

using namespace std::chrono_literals;

namespace
{
// The maximum amount of mappings we allow. This prevents performance and memory exhaustion problems when too many
// items are on the desktop.
// https://bugs.kde.org/show_bug.cgi?id=469445
constexpr auto MAX_MAPPING_COUNT = 4096;
} // namespace

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
        KConfigGroup group(config, QStringLiteral("ScreenMapping"));
        group.writeEntry(QStringLiteral("screenMapping"), screenMapping());
        config->sync();
    });

    // used to compress screenMappingChanged signals when addMapping is called multiple times,
    // eg. from FolderModel::filterAcceptRows. The timer interval is an arbitrary number,
    // that doesn't delay too much the signal, but still compresses as much as possible
    m_screenMappingChangedTimer->setInterval(100ms);
    m_screenMappingChangedTimer->setSingleShot(true);
}

void ScreenMapper::removeScreen(int screenId, const QString &activity, const QUrl &screenUrl)
{
    const std::pair<int, QString> pair = std::make_pair(screenId, activity);

    if (screenId < 0 || !m_availableScreens.contains(pair))
        return;

    const auto screenPathWithScheme = screenUrl.url();
    // store the original location for the items
    QList<QUrl> urlsToRemoveFromMapping;
    for (auto it = m_screenItemMap.constBegin(); it != m_screenItemMap.constEnd(); ++it) {
        const auto &name = it.key();
        if (it.value() == screenId && name.first.url().startsWith(screenPathWithScheme) && name.second == activity) {
            bool found = false;
            for (const auto &disabledUrls : std::as_const(m_itemsOnDisabledScreensMap)) {
                found = disabledUrls.contains(name.first);
                if (found)
                    break;
            }
            if (!found) {
                auto urlVectorIt = m_itemsOnDisabledScreensMap.find(pair);
                if (urlVectorIt == m_itemsOnDisabledScreensMap.end()) {
                    m_itemsOnDisabledScreensMap[pair] = {name.first};
                } else {
                    urlVectorIt->insert(name.first);
                }
            }
            urlsToRemoveFromMapping.append(name.first);
        }
    }

    saveDisabledScreensMap();

    for (const auto &url : urlsToRemoveFromMapping)
        removeFromMap(url, activity);

    m_availableScreens.removeAll(pair);

    auto pathIt = m_screensPerPath.find(screenUrl);
    if (pathIt != m_screensPerPath.end() && pathIt.value().size() > 0) {
        // remove the screen for a certain url, used when switching the URL for a screen
        pathIt->removeAll(pair);
    } else if (screenUrl.isEmpty()) {
        // the screen was indeed removed so all references to it needs to be cleaned up
        for (auto &pathIt : m_screensPerPath) {
            pathIt.removeAll(pair);
        }
    }

    Q_EMIT screensChanged();
}

void ScreenMapper::addScreen(int screenId, const QString &activity, const QUrl &screenUrl)
{
    const std::pair<int, QString> pair = std::make_pair(screenId, activity);

    if (screenId < 0 || m_availableScreens.contains(pair))
        return;

    const auto screenPathWithScheme = screenUrl.url();
    // restore the stored locations
    auto it = m_itemsOnDisabledScreensMap.find(pair);
    if (it != m_itemsOnDisabledScreensMap.end()) {
        auto &items = it.value();
        auto itemIt = items.begin();
        while (itemIt != items.end()) {
            // add the items to the new screen, if they are on a disabled screen and their
            // location is below the new screen's path
            if (itemIt->url().startsWith(screenPathWithScheme)) {
                addMapping(*itemIt, screenId, activity, DelayedSignal);
                itemIt = items.erase(itemIt);
                continue;
            }
            itemIt = std::next(itemIt);
        }

        if (items.empty()) {
            m_itemsOnDisabledScreensMap.erase(it);
        }
    }
    saveDisabledScreensMap();

    m_availableScreens.append(pair);

    // path is empty when a new screen appears that has no folderview base path associated with
    if (!screenUrl.isEmpty()) {
        auto it = m_screensPerPath.find(screenUrl);
        if (it == m_screensPerPath.end()) {
            m_screensPerPath[screenUrl] = {pair};
        } else {
            it->append(pair);
        }
    }

    Q_EMIT screensChanged();
}

void ScreenMapper::addMapping(const QUrl &url, int screen, const QString &activity, MappingSignalBehavior behavior)
{
    if (m_screenItemMap.count() > MAX_MAPPING_COUNT) {
        // Don't spam this
        static auto reported = false;
        if (!reported) {
            qCCritical(FOLDER)
                << "Greater than" << MAX_MAPPING_COUNT
                << "files and folders on the desktop; this is too many to map their positions in a performant way! Not adding any more position mappings.";
            reported = true;
        }
        return;
    }

    m_screenItemMap[std::make_pair(url, activity)] = screen;

    if (behavior == DelayedSignal) {
        m_screenMappingChangedTimer->start();
    } else {
        Q_EMIT screenMappingChanged();
    }
}

void ScreenMapper::removeFromMap(const QUrl &url, const QString &activity)
{
    m_screenItemMap.remove(std::make_pair(url, activity));

    m_screenMappingChangedTimer->start();
}

int ScreenMapper::firstAvailableScreen(const QUrl &screenUrl, const QString &activity) const
{
    auto screens = m_screensPerPath[screenUrl];
    std::optional<int> newFirstScreen = std::nullopt;

    for (const std::pair<int, QString> &screen : std::as_const(screens)) {
        if (screen.second != activity) {
            continue;
        }

        if (newFirstScreen.has_value()) {
            if (newFirstScreen > screen.first) {
                newFirstScreen = screen.first;
            }
        } else {
            newFirstScreen = screen.first;
        }
    }

    return newFirstScreen.value_or(-1);
}

void ScreenMapper::removeItemFromDisabledScreen(const QUrl &url)
{
    for (auto it = m_itemsOnDisabledScreensMap.begin(); it != m_itemsOnDisabledScreensMap.end(); ++it) {
        auto urls = &(*it);
        urls->remove(url);
    }
}

void ScreenMapper::setSharedDesktop(bool sharedDesktops)
{
    if (m_sharedDesktops != sharedDesktops) {
        m_sharedDesktops = true;
        if (!m_corona)
            return;

        auto config = m_corona->config();
        KConfigGroup group(config, QStringLiteral("ScreenMapping"));
        group.writeEntry(QStringLiteral("sharedDesktops"), m_sharedDesktops);
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
            auto config = m_corona->config();
            KConfigGroup group(config, QStringLiteral("ScreenMapping"));
            const QStringList mapping = group.readEntry(QStringLiteral("screenMapping"), QStringList{});
            setScreenMapping(mapping);
            m_sharedDesktops = group.readEntry(QStringLiteral("sharedDesktops"), false);

            const QStringList serializedMap = group.readEntry(QStringLiteral("itemsOnDisabledScreens"), QStringList{});
            readDisabledScreensMap(serializedMap);
        }
    }
}

QStringList ScreenMapper::screenMapping() const
{
    QStringList result;
    result.reserve(m_screenItemMap.count() * 3); // Match setScreenMapping()
    auto it = m_screenItemMap.constBegin();
    int i = 0;
    while (it != m_screenItemMap.constEnd()) {
        if (i >= MAX_MAPPING_COUNT) {
            qCCritical(FOLDER)
                << "Greater than" << MAX_MAPPING_COUNT
                << "disabled files and folders; this is too many to remember their position in a performant way! Not adding any more position mappings.";
            break;
        }
        result.append(it.key().first.toString());
        result.append(QString::number(it.value())); // Screen ID
        result.append(it.key().second); // Activity ID
        ++it;
        ++i;
    }

    return result;
}

void ScreenMapper::setScreenMapping(const QStringList &mapping)
{
    decltype(m_screenItemMap) newMap;
    // <url>,<screen id>,<activity id>
    constexpr int sizeOfParamGroup = 3; // Match screenMapping()
    const int count = std::min<int>(mapping.size(), MAX_MAPPING_COUNT * sizeOfParamGroup);
    Q_ASSERT(count % sizeOfParamGroup == 0);

    newMap.reserve(count / sizeOfParamGroup);
    QMap<int, int> screenConsistencyMap;
    for (int i = 0; i < count - (sizeOfParamGroup - 1); i += sizeOfParamGroup) {
        if (i + (sizeOfParamGroup - 1) < count) {
            const QUrl url = QUrl::fromUserInput(mapping[i], {}, QUrl::AssumeLocalFile);
            const QString activity = mapping[i + 2];
            newMap[std::make_pair(url, activity)] = mapping[i + 1].toInt();
            screenConsistencyMap[mapping[i + 1].toInt()] = -1;
        }
    }

    int lastMappedScreen = 0;
    for (int key : screenConsistencyMap.keys()) {
        screenConsistencyMap[key] = lastMappedScreen++;
    }

    for (auto it = newMap.begin(); it != newMap.end(); it++) {
        newMap[it.key()] = screenConsistencyMap.value(it.value());
    }

    if (m_screenItemMap != newMap) {
        m_screenItemMap = newMap;
        Q_EMIT screenMappingChanged();
    }
}

int ScreenMapper::screenForItem(const QUrl &url, const QString &activity) const
{
    const int screen = m_screenItemMap.value(std::make_pair(url, activity), -1);

    if (!m_availableScreens.contains(std::make_pair(screen, activity)))
        return -1;

    return screen;
}

QUrl ScreenMapper::stringToUrl(const QString &path)
{
    return QUrl::fromUserInput(path, {}, QUrl::AssumeLocalFile);
}

QStringList ScreenMapper::disabledScreensMap() const
{
    QStringList serializedMap;
    auto it = m_itemsOnDisabledScreensMap.constBegin();
    for (int i = 0; it != m_itemsOnDisabledScreensMap.constEnd(); it = std::next(it), ++i) {
        if (i >= MAX_MAPPING_COUNT) {
            qCCritical(FOLDER)
                << "Greater than" << MAX_MAPPING_COUNT
                << "files and folders on the desktop; this is too many to map their positions in a performant way! Not adding any more position mappings.";
            break;
        }
        serializedMap.append(QString::number(it.key().first)); // Screen ID
        serializedMap.append(it.key().second); // Activity ID
        const auto urls = it.value();
        serializedMap.append(QString::number(urls.size())); // Number of urls
        for (const auto &url : urls) {
            serializedMap.append(url.toString());
        }
    }

    return serializedMap;
}

void ScreenMapper::readDisabledScreensMap(const QStringList &serializedMap)
{
    m_itemsOnDisabledScreensMap.clear();
    bool readingScreenId = true;
    bool readingActivityId = true;
    int vectorSize = -1;
    int screenId = -1;
    QString activityId;
    int vectorCounter = 0;

    // <screen id>,<activity id>,<number of urls>,<url 1>, ...
    for (const auto &entry : serializedMap) {
        if (readingScreenId) {
            screenId = entry.toInt();
            readingScreenId = false;
        } else if (readingActivityId) {
            activityId = entry;
            readingActivityId = false;
        } else if (vectorSize == -1 /*number of urls is not read*/) {
            vectorSize = entry.toInt();
        } else {
            const auto url = stringToUrl(entry);
            const auto pair = std::make_pair(screenId, activityId);
            auto urlVectorIt = m_itemsOnDisabledScreensMap.find(pair);
            if (urlVectorIt == m_itemsOnDisabledScreensMap.end()) {
                m_itemsOnDisabledScreensMap[pair] = {url};
            } else {
                urlVectorIt->insert(url);
            }
            vectorCounter++;
            if (vectorCounter == vectorSize) {
                readingScreenId = true;
                readingActivityId = true;
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
    KConfigGroup group(config, QStringLiteral("ScreenMapping"));
    const auto serializedMap = disabledScreensMap();

    group.writeEntry(QStringLiteral("itemsOnDisabledScreens"), serializedMap);
}

#include "moc_screenmapper.cpp"
