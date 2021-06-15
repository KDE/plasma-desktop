/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SCREENMAPPER_H
#define SCREENMAPPER_H

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
    void saveDisabledScreensMap() const;
    void readDisabledScreensMap();

    ScreenMapper(QObject *parent = nullptr);

    QHash<QUrl, int> m_screenItemMap;
    QHash<int, QVector<QUrl>> m_itemsOnDisabledScreensMap;
    QHash<QUrl, QVector<int>> m_screensPerPath; // screens per registered path
    QVector<int> m_availableScreens;
    Plasma::Corona *m_corona = nullptr;
    QTimer *m_screenMappingChangedTimer = nullptr;
    bool m_sharedDesktops = false; // all screens share the same desktops, disabling the screen mapping
};

#endif // SCREENMAPPER_H
