/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_keyboard.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include <KGlobalAccel>

#include "bindings.h"
#include "keyboard_config.h"
#include "keyboard_dbus.h"
#include "keyboardmiscsettings.h"
#include "keyboardmodel.h"
#include "keyboardsettings.h"
#include "keyboardsettingsdata.h"
#include "layoutmodel.h"
#include "shortcuthelper.h"
#include "tastenbrett.h"
#include "userlayoutmodel.h"
#include "workspace_options.h"
#include "x11_helper.h"
#include "xkboptionsmodel.h"

#include "debug.h"

KCMKeyboard::KCMKeyboard(QObject *parent, const KPluginMetaData &data)
    : KQuickManagedConfigModule(parent, data)
    , m_data(new KeyboardSettingsData(this))
    , m_config(new KeyboardConfig(m_data->keyboardSettings(), this))
    , m_layoutModel(new LayoutModel(this))
    , m_userLayoutModel(new UserLayoutModel(m_config, this))
    , m_keyboardModel(new KeyboardModel(this))
    , m_shortcutHelper(new ShortcutHelper(this))
    , m_xkbOptionsModel(new XkbOptionsModel(this))
{
    qmlRegisterAnonymousType<WorkspaceOptions>("org.kde.plasma.keyboard.kcm", 0);
    qmlRegisterAnonymousType<KeyboardMiscSettings>("org.kde.plasma.keyboard.kcm", 0);
    qmlRegisterAnonymousType<KeyboardSettings>("org.kde.plasma.keyboard.kcm", 0);

    connect(m_data->keyboardSettings(), &KeyboardSettings::configureLayoutsChanged, this, [this]() -> void {
        if (m_data->keyboardSettings()->configureLayouts()) {
            const QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
            for (const auto &layoutUnit : layouts) {
                m_config->layouts().append(layoutUnit);
            }

            m_userLayoutModel->reset();
        } else {
            m_userLayoutModel->clear();
        }
    });

    connect(m_data->keyboardSettings(), &KeyboardSettings::resetOldXkbOptionsChanged, this, [this]() -> void {
        if (m_data->keyboardSettings()->resetOldXkbOptions()) {
            m_xkbOptionsModel->populateWithCurrentXkbOptions();
            m_data->keyboardSettings()->setXkbOptions(m_xkbOptionsModel->xkbOptions());
        }
    });

    connect(m_userLayoutModel, &UserLayoutModel::modelReset, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::rowsInserted, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::rowsRemoved, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::dataChanged, this, &KCMKeyboard::resetShortcuts);
    connect(m_userLayoutModel, &UserLayoutModel::rowsMoved, this, &KCMKeyboard::settingsChanged);
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

LayoutModel *KCMKeyboard::layouts() const
{
    return m_layoutModel;
}

UserLayoutModel *KCMKeyboard::userLayoutModel() const
{
    return m_userLayoutModel;
}

KeyboardModel *KCMKeyboard::keyboards() const
{
    return m_keyboardModel;
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
    return X11Helper::MAX_GROUP_COUNT;
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

    m_shortcutHelper->defaults();
    m_xkbOptionsModel->setXkbOptions(m_data->keyboardSettings()->defaultXkbOptionsValue());
    m_config->defaults();
    m_userLayoutModel->reset();
}

void KCMKeyboard::load()
{
    KQuickManagedConfigModule::load();

    m_shortcutHelper->load();
    m_xkbOptionsModel->setXkbOptions(m_data->keyboardSettings()->xkbOptions());
    m_config->load();
    m_shortcutHelper->actionColletion()->loadLayoutShortcuts(m_config->layouts());
    m_userLayoutModel->reset();
}

void KCMKeyboard::save()
{
    KQuickManagedConfigModule::save();

    m_shortcutHelper->save();

    QStringList options;
    if (m_data->keyboardSettings()->resetOldXkbOptions()) {
        options = m_xkbOptionsModel->xkbOptions();

        // QStringLists with a single empty string are serialized as "\\0", avoid that
        // by saving them as an empty list instead. This way it can be passed as-is to
        // libxkbcommon/setxkbmap. Before KConfigXT it used QStringList::join(",").
        if (options.size() == 1 && options.constFirst().isEmpty()) {
            options.clear();
        }
    }
    m_data->keyboardSettings()->setXkbOptions(options);
    m_data->keyboardSettings()->save();

    m_shortcutHelper->actionColletion()->setLayoutShortcuts(m_config->layouts());
    m_config->save();
    m_userLayoutModel->reset();

    QDBusMessage message = QDBusMessage::createSignal(KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE);
    QDBusConnection::sessionBus().send(message);
}

bool KCMKeyboard::isSaveNeeded() const
{
    return m_data->workspaceOptions()->isSaveNeeded() || m_data->keyboardMiscSettings()->isSaveNeeded() || m_config->isSaveNeeded()
        || m_shortcutHelper->isSaveNeeded() || m_xkbOptionsModel->xkbOptions() != m_data->keyboardSettings()->xkbOptions();
}

bool KCMKeyboard::isDefaults() const
{
    return m_data->workspaceOptions()->isDefaults() || m_data->keyboardMiscSettings()->isDefaults() || m_config->isDefaults()
        || m_shortcutHelper->isSaveNeeded() || m_xkbOptionsModel->xkbOptions() == m_data->keyboardSettings()->xkbOptions();
}

void KCMKeyboard::resetShortcuts()
{
    settingsChanged();

    m_shortcutHelper->actionColletion()->resetLayoutShortcuts();
    m_shortcutHelper->actionColletion()->setLayoutShortcuts(m_config->layouts());
}

#include "kcm_keyboard.moc"
#include "moc_kcm_keyboard.cpp"
