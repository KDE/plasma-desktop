/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_keyboard.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include <qqml.h>

#include <KConfigGroup>
#include <KGlobalAccel>
#include <KSharedConfig>

#include "bindings.h"
#include "keyboard_config.h"
#include "keyboardmiscsettings.h"
#include "keyboardsettings.h"
#include "keyboardsettingsdata.h"
#include "layoutunit.h"
#include "shortcuthelper.h"
#include "tastenbrett.h"
#include "userlayoutmodel.h"
#include "workspace_options.h"
#include "xkb_rules.h"
#include "xkboptionsmodel.h"

#include "debug.h"

using namespace Qt::StringLiterals;

KCMKeyboard::KCMKeyboard(QObject *parent, const KPluginMetaData &data)
    : KQuickManagedConfigModule(parent, data)
    , m_data(new KeyboardSettingsData(this))
    , m_config(new KeyboardConfig(m_data->keyboardSettings(), this))
    , m_userLayoutModel(new UserLayoutModel(m_config, this))
    , m_shortcutHelper(new ShortcutHelper(this))
    , m_xkbOptionsModel(new XkbOptionsModel(this))
{
    const auto uri = "org.kde.plasma.private.kcm_keyboard";
    qmlRegisterAnonymousType<WorkspaceOptions>(uri, 1);
    qmlRegisterAnonymousType<KeyboardMiscSettings>(uri, 1);
    qmlRegisterAnonymousType<KeyboardSettings>(uri, 1);
    qmlRegisterUncreatableMetaObject(NumLockState::staticMetaObject, uri, 1, 0, "NumLockState", QString());

    connect(m_data->keyboardSettings(), &KeyboardSettings::resetOldXkbOptionsChanged, this, [this]() -> void {
        if (m_data->keyboardSettings()->resetOldXkbOptions()) {
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

QString KCMKeyboard::defaultLayout() const
{
    // Mirrors how KWin picks the layout when none is configured in kxkbrc (see Xkb::reconfigure in kwin/src/xkb.cpp).
    QString layouts;
    QString variants;

    // With FollowLocale1, KWin takes the layout from systemd-localed
    if (KSharedConfig::openConfig(u"kwinrc"_s)->group(u"Wayland"_s).readEntry("FollowLocale1", false)) {
        auto message =
            QDBusMessage::createMethodCall(u"org.freedesktop.locale1"_s, u"/org/freedesktop/locale1"_s, u"org.freedesktop.DBus.Properties"_s, u"GetAll"_s);
        message.setArguments({u"org.freedesktop.locale1"_s});
        const QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid()) {
            layouts = reply.value().value(u"X11Layout"_s).toString();
            variants = reply.value().value(u"X11Variant"_s).toString();
        } else {
            qCWarning(KCM_KEYBOARD) << "Failed to query org.freedesktop.locale1:" << reply.error().message();
        }
    }

    // Layout and variant from the environment are only ever used together
    if (layouts.isEmpty()) {
        layouts = qEnvironmentVariable("XKB_DEFAULT_LAYOUT");
        variants = qEnvironmentVariable("XKB_DEFAULT_VARIANT");
    }

    // libxkbcommon's built-in default
    if (layouts.isEmpty()) {
        layouts = u"us"_s;
        variants.clear();
    }

    const QStringList layoutList = layouts.split(u',');
    const QStringList variantList = variants.split(u',');

    QStringList names;
    for (qsizetype i = 0; i < layoutList.size(); ++i) {
        const QString layout = layoutList.at(i).trimmed();
        const QString variant = variantList.value(i).trimmed();
        if (layout.isEmpty()) {
            continue;
        }

        const std::optional<LayoutInfo> layoutInfo = Rules::self().getLayoutInfo(layout);
        if (!layoutInfo) {
            names.append(variant.isEmpty() ? layout : u"%1(%2)"_s.arg(layout, variant));
            continue;
        }
        if (!variant.isEmpty()) {
            if (const std::optional<VariantInfo> variantInfo = layoutInfo->getVariantInfo(variant)) {
                names.append(variantInfo->description);
                continue;
            }
        }
        names.append(layoutInfo->description);
    }

    return QLocale().createSeparatedList(names);
}

#include "moc_kcm_keyboard.cpp"
