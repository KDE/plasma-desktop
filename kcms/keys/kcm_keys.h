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

#ifndef KCM_KEYS_H
#define KCM_KEYS_H

#include <QObject>

#include <KQuickAddons/ConfigModule>

class QDBusError;

class FilteredShortcutsModel;
class KGlobalAccelInterface;
class ShortcutsModel;

class KCMKeys : public KQuickAddons::ConfigModule
{
    Q_OBJECT 

    Q_PROPERTY(ShortcutsModel *shortcutsModel READ shortcutsModel CONSTANT)
    Q_PROPERTY(FilteredShortcutsModel *filteredModel READ filteredModel CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccured)

public:
    KCMKeys(QObject *parent, const QVariantList &args);

    void defaults() override;
    void load() override;
    void save() override;

    Q_INVOKABLE void writeScheme(const QUrl &url);
    Q_INVOKABLE void loadScheme(const QUrl &url);
    Q_INVOKABLE QVariantList defaultSchemes() const;

    Q_INVOKABLE void addApplication(QQuickItem *ctx);

    Q_INVOKABLE QString keySequenceToString(const QKeySequence &keySequence) const;
    Q_INVOKABLE QString urlFilename(const QUrl &url);

    ShortcutsModel* shortcutsModel() const;
    FilteredShortcutsModel* filteredModel() const;
    QString lastError() const;

Q_SIGNALS:
    void errorOccured();

private:
    void setError(const QString &errorMessage);

    QString m_lastError;
    ShortcutsModel *m_shortcutsModel;
    FilteredShortcutsModel *m_fileredModel;
    KGlobalAccelInterface *m_globalAccelInterface;
};

#endif
