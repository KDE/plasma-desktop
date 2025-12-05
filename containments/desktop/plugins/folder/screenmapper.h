/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QVariantHash>

#include <Plasma/Corona>

class QTimer;

class ScreenMapper : public QObject
{
    Q_OBJECT

public:
    enum MappingSignalBehavior {
        DelayedSignal = 0,
        ImmediateSignal,
    };

    static ScreenMapper *instance();
    ~ScreenMapper() override = default;

    /**
     * Convert m_screenItemMap to QStringList
     *
     * The format of screenMapping is
     * - URL of the item
     * - Screen ID
     * - Activity ID
     *
     * @return QStringList the formatted config strings in a list
     */
    QStringList screenMapping() const;
    void setScreenMapping(const QStringList &mapping);

    int screenForItem(const QUrl &url, const QString &activity) const;
    void maybeMoveToDisabledScreens(const QUrl &url, const QString &activity);
    void addMapping(const QUrl &url, int screen, const QString &activity, MappingSignalBehavior behavior = ImmediateSignal);
    void removeFromMap(const QUrl &url, const QString &activity);
    void setCorona(Plasma::Corona *corona);

    void addScreen(int screenId, const QString &activity, const QUrl &screenUrl);
    void removeScreen(int screenId, const QString &activity, const QUrl &screenUrl);
    int firstAvailableScreen(const QUrl &screenUrl, const QString &activity) const;
    void removeItemFromDisabledScreen(const QUrl &url);

    void addScreenTransition(int screenSrc, int screenDst, const QString &activity);

    bool sharedDesktops() const
    {
        return m_sharedDesktops;
    }
    void setSharedDesktop(bool sharedDesktops);

#ifdef BUILD_TESTING
    void cleanup();
#endif

    static QUrl stringToUrl(const QString &path);

Q_SIGNALS:
    void screenMappingChanged() const;
    void screensChanged() const;

private:
    void processScreenTransition();

    /**
     * The format of DisabledScreensMap is:
     * - Screen ID (controlled by readingScreenId)
     * - Activity ID (controlled by readingActivityId)
     * - The number of items
     * - List of items
     *
     * @return QStringList the formatted config strings in a list
     */
    QStringList disabledScreensMap() const;
    void saveDisabledScreensMap() const;
    void readDisabledScreensMap(const QStringList &serializedMap);

    ScreenMapper(QObject *parent = nullptr);

    QHash<std::pair<QUrl, QString /* activity ID */>, int> m_screenItemMap;
    // Use QSet when appropriate to improve lookup times substantially.
    QHash<std::pair<int /* screen */, QString /* activity ID */>, QSet<QUrl>> m_itemsOnDisabledScreensMap;
    QHash<QUrl, QList<std::pair<int /* screen */, QString /* activity ID */>>> m_screensPerPath; // screens per registered path
    QList<std::pair<int /* screen */, QString /* activity ID */>> m_availableScreens;
    QPointer<Plasma::Corona> m_corona;
    QTimer *const m_screenMappingChangedTimer;
    bool m_sharedDesktops = false; // all screens share the same desktops, disabling the screen mapping
    bool m_disabledScreensMapDirty = false;
    QList<std::tuple<int /* screen src */, int /* screen dst */, QString /* activity ID */>> m_screenTransitions;
    QTimer *const m_screenTransitionTimer;

    friend class ScreenMapperTest;
};
