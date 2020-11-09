/*
 * Copyright 2020 David Redondo <kde@david-redondo.de>
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

#ifndef GLOBALACCELMODEL_H
#define GLOBALACCELMODEL_H

#include <QList>

#include "basemodel.h"


class QDBusError;

class KConfigBase;
class KGlobalAccelInterface;
class KGlobalShortcutInfo;

class FilteredShortcutsModel;

class GlobalAccelModel : public BaseModel
{
    Q_OBJECT

public:

    GlobalAccelModel(KGlobalAccelInterface *interface, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;

    void addApplication(const QString &desktopFileName, const QString &displayName);

    void exportToConfig(const KConfigBase &config) override;
    void importConfig(const KConfigBase &config) override;

    void load() override;
    void save() override;
 

Q_SIGNALS:
    void errorOccured(const QString&);

private:
    Component loadComponent(const QList<KGlobalShortcutInfo> &info);
    void removeComponent(const Component &component);
    void genericErrorOccured(const QString &description, const QDBusError &error);

    KGlobalAccelInterface *m_globalAccelInterface;
};

#endif // SHORTCUTSMODEL_H
