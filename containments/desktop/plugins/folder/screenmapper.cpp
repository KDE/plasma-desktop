/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <optional>

#include "screenmapper.h"

#include <QMap>
#include <QScreen>
#include <QTimer>

#include <KActivities/Consumer>
#include <KConfig>
#include <KConfigGroup>
#include <Plasma/Corona>
#include <chrono>

using namespace std::chrono_literals;

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
        qWarning() << "DDDDDSAVINGMAPPING";
        auto config = m_corona->config();
        KConfigGroup group(config, QStringLiteral("ScreenMapping"));
        group.writeEntry(QStringLiteral("screenMapping-%1").arg(m_corona->numScreens()), screenMapping());
        config->sync();
    });

    // used to compress screenMappingChanged signals when addMapping is called multiple times,
    // eg. from FolderModel::filterAcceptRows. The timer interval is an arbitrary number,
    // that doesn't delay too much the signal, but still compresses as much as possible
    m_screenMappingChangedTimer->setInterval(100ms);
    m_screenMappingChangedTimer->setSingleShot(true);
}

void ScreenMapper::loadMapping(int numScreens)
{
    auto config = m_corona->config();
    KConfigGroup group(config, QStringLiteral("ScreenMapping"));
    QStringList mapping = group.readEntry(QStringLiteral("screenMapping-%1").arg(numScreens), QStringList{});
    if (mapping.isEmpty() && numScreens > 1) {
        QStringList mapping = group.readEntry(QStringLiteral("screenMapping-%1").arg(numScreens - 1), QStringList{});
    }
    if (mapping.isEmpty()) {
        mapping = group.readEntry(QStringLiteral("screenMapping"), QStringList{});
    }
    setScreenMapping(mapping);
}

void ScreenMapper::removeScreen(int screenId, const QString &activity, const QUrl &screenUrl)
{
    if (screenId < 0) {
        return;
    }

    // Quirk of ShellCorona: when screenRemoved is emitted, m_corona->numScreens() is still the old count, while when screenAdded is emitted is already the new
    // count. should this be fixed?
    loadMapping(m_corona->numScreens() - 1);
    qWarning() << "ScreenMapper::removeScreen" << screenId << m_corona->numScreens();
    const std::pair<int, QString> pair = std::make_pair(screenId, activity);

    if (!m_availableScreens.contains(pair)) {
        return;
    }

    const auto screenPathWithScheme = screenUrl.url();

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
    m_screenMappingChangedTimer->start();
}

void ScreenMapper::addScreen(int screenId, const QString &activity, const QUrl &screenUrl)
{
    if (screenId < 0) {
        return;
    }

    qWarning() << "ScreenMapper::addScreen" << screenId << m_corona->numScreens();
    loadMapping(m_corona->numScreens());

    const std::pair<int, QString> pair = std::make_pair(screenId, activity);

    const auto screenPathWithScheme = screenUrl.url();
    // restore the stored locations
    // TODO
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
    m_screenMappingChangedTimer->start();
}

void ScreenMapper::addMapping(const QUrl &url, int screen, const QString &activity, MappingSignalBehavior behavior)
{
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

void ScreenMapper::setSharedDesktop(bool sharedDesktops)
{
    if (m_sharedDesktops != sharedDesktops) {
        m_sharedDesktops = true;
        if (!m_corona)
            return;

        auto config = m_corona->config();
        KConfigGroup group(config, QStringLiteral("ScreenMapping"));
        group.writeEntry(QStringLiteral("screenMapping-%1").arg(m_corona->numScreens()), m_sharedDesktops);
    }
}

#ifdef BUILD_TESTING
void ScreenMapper::cleanup()
{
    m_screenItemMap.clear();
    m_screensPerPath.clear();
    m_availableScreens.clear();
}
#endif

void ScreenMapper::setCorona(Plasma::Corona *corona, const QString &activity)
{
    if (m_corona != corona) {
        Q_ASSERT(!m_corona);

        m_corona = corona;
        if (m_corona) {
            connect(m_corona, &Plasma::Corona::screenRemoved, this, [this, activity](int screenId) {
                removeScreen(screenId, activity, {});
            });
            connect(m_corona, &Plasma::Corona::screenAdded, this, [this, activity](int screenId) {
                addScreen(screenId, activity, {});
            });

            loadMapping(m_corona->numScreens());
            KConfigGroup group(m_corona->config(), QStringLiteral("ScreenMapping"));
            m_sharedDesktops = group.readEntry(QStringLiteral("sharedDesktops"), false);
        }
    }
}

QStringList ScreenMapper::screenMapping() const
{
    QStringList result;
    result.reserve(m_screenItemMap.count() * 3); // Match setScreenMapping()
    auto it = m_screenItemMap.constBegin();
    while (it != m_screenItemMap.constEnd()) {
        result.append(it.key().first.toString());
        result.append(QString::number(it.value())); // Screen ID
        result.append(it.key().second); // Activity ID
        ++it;
    }

    return result;
}

void ScreenMapper::setScreenMapping(const QStringList &mapping)
{
    decltype(m_screenItemMap) newMap;
    const int count = mapping.count();
    const bool useDefaultActivity = count % 3 != 0; // Missing activity ID from the old config before 5.25
    const int sizeOfParamGroup = useDefaultActivity ? 2 : 3; // Match screenMapping()

    newMap.reserve(count / sizeOfParamGroup);
    QMap<int, int> screenConsistencyMap;
    for (int i = 0; i < count - (sizeOfParamGroup - 1); i += sizeOfParamGroup) {
        if (i + (sizeOfParamGroup - 1) < count) {
            const QUrl url = QUrl::fromUserInput(mapping[i], {}, QUrl::AssumeLocalFile);
            const QString activity = useDefaultActivity ? KActivities::Consumer().currentActivity() : mapping[i + 2];
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
        // Q_EMIT screenMappingChanged();
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

