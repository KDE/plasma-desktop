/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KCM_KEYS_H
#define KCM_KEYS_H

#include <QKeySequence>
#include <QObject>

#include <KQuickAddons/ConfigModule>

class FilteredShortcutsModel;
class KGlobalAccelInterface;
class GlobalAccelModel;
class ShortcutsModel;
class StandardShortcutsModel;

class KCMKeys : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(FilteredShortcutsModel *filteredModel READ filteredModel CONSTANT)
    Q_PROPERTY(ShortcutsModel *shortcutsModel READ shortcutsModel CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccured)

public:
    KCMKeys(QObject *parent, const QVariantList &args);

    void defaults() override;
    void load() override;
    void save() override;

    Q_INVOKABLE void
    requestKeySequence(QQuickItem *context, const QModelIndex &index, const QKeySequence &newSequence, const QKeySequence &oldSequence = QKeySequence());

    Q_INVOKABLE void writeScheme(const QUrl &url);
    Q_INVOKABLE void loadScheme(const QUrl &url);
    Q_INVOKABLE QVariantList defaultSchemes() const;

    Q_INVOKABLE void addApplication(QQuickItem *ctx);

    Q_INVOKABLE QString keySequenceToString(const QKeySequence &keySequence) const;
    Q_INVOKABLE QString urlFilename(const QUrl &url);

    FilteredShortcutsModel *filteredModel() const;
    ShortcutsModel *shortcutsModel() const;
    QString lastError() const;

Q_SIGNALS:
    void errorOccured();

private:
    void setError(const QString &errorMessage);
    QModelIndex conflictingIndex(const QKeySequence &keySequence);

    QString m_lastError;
    FilteredShortcutsModel *m_filteredModel;
    GlobalAccelModel *m_globalAccelModel;
    KGlobalAccelInterface *m_globalAccelInterface;
    ShortcutsModel *m_shortcutsModel;
    StandardShortcutsModel *m_standardShortcutsModel;
};

#endif
