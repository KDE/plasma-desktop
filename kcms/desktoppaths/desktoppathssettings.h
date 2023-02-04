/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCoreConfigSkeleton>

class XdgPathsSettingsStore;

class DesktopPathsSettings : public KCoreConfigSkeleton
{
    Q_OBJECT

    Q_PROPERTY(QUrl desktopLocation READ desktopLocation WRITE setDesktopLocation NOTIFY desktopLocationChanged)
    Q_PROPERTY(QUrl defaultDesktopLocation READ defaultDesktopLocation CONSTANT)

    Q_PROPERTY(QUrl documentsLocation READ documentsLocation WRITE setDocumentsLocation NOTIFY documentsLocationChanged)
    Q_PROPERTY(QUrl defaultDocumentsLocation READ defaultDocumentsLocation CONSTANT)

    Q_PROPERTY(QUrl downloadsLocation READ downloadsLocation WRITE setDownloadsLocation NOTIFY downloadsLocationChanged)
    Q_PROPERTY(QUrl defaultDownloadsLocation READ defaultDownloadsLocation CONSTANT)

    Q_PROPERTY(QUrl musicLocation READ musicLocation WRITE setMusicLocation NOTIFY musicLocationChanged)
    Q_PROPERTY(QUrl defaultMusicLocation READ defaultMusicLocation CONSTANT)

    Q_PROPERTY(QUrl picturesLocation READ picturesLocation WRITE setPicturesLocation NOTIFY picturesLocationChanged)
    Q_PROPERTY(QUrl defaultPicturesLocation READ defaultPicturesLocation CONSTANT)

    Q_PROPERTY(QUrl videosLocation READ videosLocation WRITE setVideosLocation NOTIFY videosLocationChanged)
    Q_PROPERTY(QUrl defaultVideosLocation READ defaultVideosLocation CONSTANT)

    Q_PROPERTY(QUrl publicLocation READ publicLocation WRITE setPublicLocation NOTIFY publicLocationChanged)
    Q_PROPERTY(QUrl defaultPublicLocation READ defaultPublicLocation CONSTANT)

    Q_PROPERTY(QUrl templatesLocation READ templatesLocation WRITE setTemplatesLocation NOTIFY templatesLocationChanged)
    Q_PROPERTY(QUrl defaultTemplatesLocation READ defaultTemplatesLocation CONSTANT)

public:
    DesktopPathsSettings(QObject *parent = nullptr);

    QUrl desktopLocation() const;
    void setDesktopLocation(const QUrl &url);
    QUrl defaultDesktopLocation() const;

    QUrl documentsLocation() const;
    void setDocumentsLocation(const QUrl &url);
    QUrl defaultDocumentsLocation() const;

    QUrl downloadsLocation() const;
    void setDownloadsLocation(const QUrl &url);
    QUrl defaultDownloadsLocation() const;

    QUrl musicLocation() const;
    void setMusicLocation(const QUrl &url);
    QUrl defaultMusicLocation() const;

    QUrl picturesLocation() const;
    void setPicturesLocation(const QUrl &url);
    QUrl defaultPicturesLocation() const;

    QUrl videosLocation() const;
    void setVideosLocation(const QUrl &url);
    QUrl defaultVideosLocation() const;

    QUrl publicLocation() const;
    void setPublicLocation(const QUrl &url);
    QUrl defaultPublicLocation() const;

    QUrl templatesLocation() const;
    void setTemplatesLocation(const QUrl &url);
    QUrl defaultTemplatesLocation() const;

Q_SIGNALS:
    void desktopLocationChanged();
    void documentsLocationChanged();
    void downloadsLocationChanged();
    void musicLocationChanged();
    void picturesLocationChanged();
    void videosLocationChanged();
    void publicLocationChanged();
    void templatesLocationChanged();

private:
    void addItemInternal(const QByteArray &propertyName, const QVariant &defaultValue, const std::function<void()> &signal);

private:
    XdgPathsSettingsStore *const m_xdgPathsStore;
};
