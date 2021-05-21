/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DESKTOPPATHSSETTINGS_H
#define DESKTOPPATHSSETTINGS_H

#include <KCoreConfigSkeleton>

class XdgPathsSettingsStore;

class DesktopPathsSettings : public KCoreConfigSkeleton
{
    Q_OBJECT
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

Q_SIGNALS:
    void widgetChanged();

private:
    void addItemInternal(const QByteArray &propertyName, const QVariant &defaultValue);

private:
    XdgPathsSettingsStore *m_xdgPathsStore;
};

#endif
