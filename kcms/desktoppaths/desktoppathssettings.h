/**
 *  Copyright 2020 Kevin Ottens <kevin.ottens@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DESKTOPPATHSSETTINGS_H
#define DESKTOPPATHSSETTINGS_H

#include <KCoreConfigSkeleton>

class PathsSettingsStore;
class XdgPathsSettingsStore;

class DesktopPathsSettings : public KCoreConfigSkeleton
{
    Q_OBJECT
public:
    DesktopPathsSettings(QObject *parent = nullptr);

    QUrl autostartLocation() const;
    void setAutostartLocation(const QUrl &url);

    QUrl desktopLocation() const;
    void setDesktopLocation(const QUrl &url);

    QUrl documentsLocation() const;
    void setDocumentsLocation(const QUrl &url);

    QUrl downloadsLocation() const;
    void setDownloadsLocation(const QUrl &url);

    QUrl musicLocation() const;
    void setMusicLocation(const QUrl &url);

    QUrl picturesLocation() const;
    void setPicturesLocation(const QUrl &url);

    QUrl videosLocation() const;
    void setVideosLocation(const QUrl &url);

private:
    bool usrSave() override;

private:
    PathsSettingsStore *m_pathsStore;
    XdgPathsSettingsStore *m_xdgPathsStore;
};

#endif

