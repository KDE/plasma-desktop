/*
    SPDX-FileCopyrightText: 2016, 2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMARTLAUNCHER_ITEM_H
#define SMARTLAUNCHER_ITEM_H

#include <QObject>
#include <QSharedPointer>
#include <QUrl>
#include <QWeakPointer>

#include "smartlauncherbackend.h"

namespace SmartLauncher
{
class Item : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl launcherUrl READ launcherUrl WRITE setLauncherUrl NOTIFY launcherUrlChanged)

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool countVisible READ countVisible NOTIFY countVisibleChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool progressVisible READ progressVisible NOTIFY progressVisibleChanged)
    Q_PROPERTY(bool urgent READ urgent NOTIFY urgentChanged)

public:
    explicit Item(QObject *parent = nullptr);
    ~Item() override = default;

    QUrl launcherUrl() const;
    void setLauncherUrl(const QUrl &launcherUrl);

    int count() const;
    bool countVisible() const;
    int progress() const;
    bool progressVisible() const;
    bool urgent() const;

Q_SIGNALS:
    void launcherUrlChanged(const QUrl &launcherUrl);

    void countChanged(int count);
    void countVisibleChanged(bool countVisible);
    void progressChanged(int progress);
    void progressVisibleChanged(bool progressVisible);
    void urgentChanged(bool urgent);

private:
    void init();

    void populate();
    void clear();

    void setCount(int count);
    void setCountVisible(bool countVisible);
    void setProgress(int progress);
    void setProgressVisible(bool progressVisible);
    void setUrgent(bool urgent);

    static QWeakPointer<Backend> s_backend;

    QSharedPointer<Backend> m_backendPtr;

    QUrl m_launcherUrl;
    QString m_storageId;

    bool m_inited = false;

    int m_count = 0;
    bool m_countVisible = false;
    int m_progress = 0;
    bool m_progressVisible = false;
    bool m_urgent = false;
};

} // namespace SmartLauncher

#endif // SMARTLAUNCHER_ITEM_H
