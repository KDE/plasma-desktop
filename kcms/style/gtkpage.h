/*
 * Copyright 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QDBusInterface>
#include <QQmlListReference>

#include "gtkthemesmodel.h"

class GtkPage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(GtkThemesModel *gtkThemesModel MEMBER m_gtkThemesModel NOTIFY gtkThemesModelChanged)

public:
    GtkPage(QObject *parent = nullptr);
    ~GtkPage() override;

    void load();
    void save();
    void defaults();

public Q_SLOTS:
    QString gtkThemeFromConfig();

    bool gtkPreviewAvailable();

    void showGtkPreview();

    void installGtkThemeFromFile(const QUrl &fileUrl);

    void onThemeRemoved();
    void onGhnsEntriesChanged(const QQmlListReference &changedEnties);

Q_SIGNALS:
    void gtkThemesModelChanged(GtkThemesModel *model);

    void showErrorMessage(const QString &message);
    void selectGtkThemeInCombobox(const QString &themeName);

    void gtkThemeSettingsChanged();

private:
    GtkThemesModel *m_gtkThemesModel;

    QDBusInterface gtkConfigInterface;
};
