/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QObject>
#include <QVariantHash>
#include <QVector>

#include "folderplugin_private_export.h"

class QTimer;

namespace Plasma
{
class Corona;
}

class FOLDERPLUGIN_TESTS_EXPORT ScreenMapper : public QObject
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
    void addMapping(const QUrl &url, int screen, const QString &activity, MappingSignalBehavior behavior = ImmediateSignal);
    void removeFromMap(const QUrl &url, const QString &activity);
    void setCorona(Plasma::Corona *corona, const QString &activity);

    void addScreen(int screenId, const QString &activity, const QUrl &screenUrl);
    void removeScreen(int screenId, const QString &activity, const QUrl &screenUrl);
    int firstAvailableScreen(const QUrl &screenUrl, const QString &activity) const;

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
    ScreenMapper(QObject *parent = nullptr);
    void loadMapping(int numScreens);

    QHash<std::pair<QUrl, QString /* activity ID */>, int> m_screenItemMap;
    QHash<QUrl, QVector<std::pair<int /* screen */, QString /* activity ID */>>> m_screensPerPath; // screens per registered path
    QVector<std::pair<int /* screen */, QString /* activity ID */>> m_availableScreens;
    Plasma::Corona *m_corona = nullptr;
    QTimer *const m_screenMappingChangedTimer;
    bool m_sharedDesktops = false; // all screens share the same desktops, disabling the screen mapping

    friend class ScreenMapperTest;
};
