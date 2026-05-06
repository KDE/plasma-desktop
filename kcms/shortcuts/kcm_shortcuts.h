/*
    SPDX-FileCopyrightText: 2026 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

#include <KQuickConfigModule>

class GlobalAccelModel;
class ShortcutsModel;
class ShortcutsListModel;

class KCMShortcuts : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(ShortcutsModel *shortcutsModel READ shortcutsModel CONSTANT)

public:
    KCMShortcuts(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);

    void defaults() override;
    void load() override;
    void save() override;

    ShortcutsModel *shortcutsModel() const;
    ShortcutsListModel *shortcutsListModel() const;

private:
    GlobalAccelModel *m_globalAccelModel;
    ShortcutsModel *m_shortcutsModel;
    ShortcutsListModel *m_shortcutsListModel;
};
