/*
    SPDX-FileCopyrightText: 2026 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_shortcuts.h"
#include "kcm_shortcuts_debug.h"

#include <KLocalizedString>

#include "globalaccelmodel.h"
#include "shortcutslistmodel.h"
#include "shortcutsmodel.h"

Q_DECLARE_LOGGING_CATEGORY(KCMSHORTCUTS)
K_PLUGIN_FACTORY_WITH_JSON(KCMShortcutsFactory, "kcm_shortcuts.json", registerPlugin<KCMShortcuts>();)

KCMShortcuts::KCMShortcuts(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickConfigModule(parent, metaData)
{
    constexpr char uri[] = "org.kde.private.kcms.shortcuts";
    qmlRegisterUncreatableType<ShortcutsModel>(uri, 2, 0, "ShortcutsModel", "Can't create ShortcutsModel");
    qmlRegisterUncreatableType<ShortcutsListModel>(uri, 2, 0, "ShortcutsListModel", "Can't create ShortcutsListModel");

    m_globalAccelModel = new GlobalAccelModel(this);
    m_shortcutsModel = new ShortcutsModel(this);
    m_shortcutsModel->addSourceModel(m_globalAccelModel);
    m_shortcutsListModel = new ShortcutsListModel(this);
    m_shortcutsListModel->setSourceModel(m_shortcutsModel);
}

void KCMShortcuts::load()
{
    m_globalAccelModel->load();
}

void KCMShortcuts::save()
{
    m_globalAccelModel->save();
}

void KCMShortcuts::defaults()
{
    m_globalAccelModel->defaults();
}

ShortcutsModel *KCMShortcuts::shortcutsModel() const
{
    return m_shortcutsModel;
}

ShortcutsListModel *KCMShortcuts::shortcutsListModel() const
{
    return m_shortcutsListModel;
}

#include "kcm_shortcuts.moc"
#include "moc_kcm_shortcuts.cpp"
