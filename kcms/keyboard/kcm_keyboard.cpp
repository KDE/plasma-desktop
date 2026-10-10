/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_keyboard.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include <qqml.h>

#include <KGlobalAccel>

#include "bindings.h"
#include "keyboardmiscsettings.h"
#include "keyboardsettings.h"
#include "keyboardsettingsdata.h"
#include "layoutunit.h"
#include "shortcuthelper.h"
#include "tastenbrett.h"
#include "userlayoutmodel.h"
#include "workspace_options.h"
#include "xkboptionsmodel.h"

#include "debug.h"

KCMKeyboard::KCMKeyboard(QObject *parent, const KPluginMetaData &data)
    : KQuickManagedConfigModule(parent, data)
    , m_data(new KeyboardSettingsData(this))
    , m_userLayoutModel(new UserLayoutModel(m_data->keyboardSettings(), this))
    , m_shortcutHelper(new ShortcutHelper(this))
    , m_xkbOptionsModel(new XkbOptionsModel(this))
{
    const auto uri = "org.kde.plasma.private.kcm_keyboard";
    qmlRegisterAnonymousType<WorkspaceOptions>(uri, 1);
    qmlRegisterAnonymousType<KeyboardMiscSettings>(uri, 1);
    qmlRegisterAnonymousType<KeyboardSettings>(uri, 1);
    qmlRegisterUncreatableMetaObject(NumLockState::staticMetaObject, uri, 1, 0, "NumLockState", QString());

    connect(m_userLayoutModel, &UserLayoutModel::modelReset, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::rowsInserted, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::rowsRemoved, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::dataChanged, this, &KCMKeyboard::resetShortcuts);
    connect(m_shortcutHelper, &ShortcutHelper::alternativeShortcutChanged, this, &KCMKeyboard::settingsChanged);
    connect(m_shortcutHelper, &ShortcutHelper::lastUsedShortcutChanged, this, &KCMKeyboard::settingsChanged);
    connect(m_xkbOptionsModel, &XkbOptionsModel::dataChanged, this, &KCMKeyboard::settingsChanged);
    connect(m_xkbOptionsModel, &XkbOptionsModel::modelReset, this, &KCMKeyboard::settingsChanged);

    setButtons(Help | Default | Apply);
}

KCMKeyboard::~KCMKeyboard()
{
}

WorkspaceOptions *KCMKeyboard::workspaceOptions() const
{
    return m_data->workspaceOptions();
}

KeyboardMiscSettings *KCMKeyboard::miscSettings() const
{
    return m_data->keyboardMiscSettings();
}

KeyboardSettings *KCMKeyboard::keyboardSettings() const
{
    return m_data->keyboardSettings();
}

UserLayoutModel *KCMKeyboard::userLayoutModel() const
{
    return m_userLayoutModel;
}

ShortcutHelper *KCMKeyboard::shortcutHelper() const
{
    return m_shortcutHelper;
}

XkbOptionsModel *KCMKeyboard::xkbOptionsModel() const
{
    return m_xkbOptionsModel;
}

int KCMKeyboard::maxGroupCount() const
{
    // more information about the limit https://bugs.freedesktop.org/show_bug.cgi?id=19501
    return 4;
}

bool KCMKeyboard::hasAccentSupport()
{
    static bool isPlasmaIM = (qgetenv("QT_IM_MODULE") == "plasmaim");
    return isPlasmaIM;
}

void KCMKeyboard::requestPreview(const QString &model, const QString &layout, const QString &variant, const QString &title)
{
    Tastenbrett::launch(model, layout, variant, m_xkbOptionsModel->xkbOptions().join(QLatin1Char(',')), title);
}

void KCMKeyboard::defaults()
{
    KQuickManagedConfigModule::defaults();

    m_userLayoutModel->resetToSettings();
    m_shortcutHelper->defaults();
    m_xkbOptionsModel->setXkbOptions(m_data->keyboardSettings()->defaultXkbOptionsValue());
}

void KCMKeyboard::load()
{
    KQuickManagedConfigModule::load();

    m_userLayoutModel->resetToSettings();
    m_shortcutHelper->load();
    m_shortcutHelper->actionCollection()->loadLayoutShortcuts(m_userLayoutModel);
    m_xkbOptionsModel->setXkbOptions(m_data->keyboardSettings()->xkbOptions());
}

void KCMKeyboard::save()
{
    QStringList options = m_xkbOptionsModel->xkbOptions();

    // QStringLists with a single empty string are serialized as "\\0", avoid that
    // by saving them as an empty list instead. This way it can be passed as-is to
    // libxkbcommon/setxkbmap.
    if (options.size() == 1 && options.constFirst().isEmpty()) {
        options.clear();
    }
    m_data->keyboardSettings()->setXkbOptions(options);

    KQuickManagedConfigModule::save();

    m_shortcutHelper->save();
    m_shortcutHelper->actionCollection()->setLayoutShortcuts(m_userLayoutModel);
}

bool KCMKeyboard::isSaveNeeded() const
{
    return m_shortcutHelper->isSaveNeeded() || m_xkbOptionsModel->xkbOptions() != m_data->keyboardSettings()->xkbOptions();
}

bool KCMKeyboard::isDefaults() const
{
    return m_shortcutHelper->isDefaults() && m_xkbOptionsModel->xkbOptions() == m_data->keyboardSettings()->defaultXkbOptionsValue();
}

void KCMKeyboard::resetShortcuts()
{
    settingsChanged();

    m_shortcutHelper->actionCollection()->resetLayoutShortcuts();
    m_shortcutHelper->actionCollection()->setLayoutShortcuts(m_userLayoutModel);
}

#include "moc_kcm_keyboard.cpp"
