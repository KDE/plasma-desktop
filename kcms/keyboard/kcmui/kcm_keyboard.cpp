/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcm_keyboard.h"

#include <KAboutData>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QQuickItem>
#include <QString>

#include "hardware_config_model.h"
#include "layout_config_model.h"
#include "layout_list_xkb_expand_proxy_model.h"
#include "advanced_model.h"
#include "advanced_filter_proxy_model.h"

#include "../keyboard_dbus.h"

K_PLUGIN_FACTORY_WITH_JSON(KeyboardModuleFactory,
                           "kcm_keyboard.json",
                           registerPlugin<KcmKeyboard>();)

KcmKeyboard::KcmKeyboard(QObject* parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_hardwareConfigModel(new HardwareConfigModel(this))
    , m_layoutConfigModel(new LayoutConfigModel(this))
    , m_underlyingAdvancedModel(new AdvancedModel(this))
{
    m_advancedModel = new AdvancedFilterProxyModel(this);
    m_advancedModel->setSourceModel(m_underlyingAdvancedModel);
    
    KAboutData* about = new KAboutData(
        "kcm_keyboard", i18n("..."),
        QStringLiteral("1.0"), QString(), KAboutLicense::GPL);
    
    about->addAuthor(i18n("Andriy Rysin"));
    about->addAuthor(i18n("Park Gun"));
    setAboutData(about);
    setButtons(Help | Apply | Default);

    m_actionCollection = new KActionCollection(this, QStringLiteral("KDE Keyboard Layout Switcher"));
    m_nextLayoutAction = m_actionCollection->addAction(QStringLiteral("Switch to Next Keyboard Layout"));
    m_nextLayoutAction->setText(i18n("Switch to Next Keyboard Layout"));
    m_nextLayoutAction->setProperty("isConfigurationAction", true);

    QObject::connect(mainUi(), SIGNAL(changed()), this, SLOT(changed()));
}

KcmKeyboard::~KcmKeyboard()
{
}

void KcmKeyboard::load()
{
    {
        KConfigGroup kxkbrc(
            KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
            QStringLiteral("Layout"));

        m_hardwareConfigModel->setKeyboardModel(kxkbrc.readEntry<QString>("Model", "pc104"));

        m_layoutConfigModel->loadEnabledLayouts();

        m_layoutConfigModel->setShowLayoutIndicator(kxkbrc.readEntry<bool>("ShowLayoutIndicator", true));
        m_layoutConfigModel->setShowForSingleLayout(kxkbrc.readEntry<bool>("ShowSingle", false));

        m_layoutConfigModel->setLayoutIndicatorShowFlag(kxkbrc.readEntry<bool>("ShowFlag", false));
        m_layoutConfigModel->setLayoutIndicatorShowLabel(kxkbrc.readEntry<bool>("ShowLabel", true));

        QString switching_policy = kxkbrc.readEntry<QString>("SwitchMode", "Global");

        auto it = std::find(
            std::begin(LayoutConfigModel::switch_modes),
            std::end(LayoutConfigModel::switch_modes),
            switching_policy);
        if (it != std::end(LayoutConfigModel::switch_modes)) {
            m_layoutConfigModel->setSwitchingPolicyIndex(
                static_cast<int>(it - std::begin(LayoutConfigModel::switch_modes)));
        } else {
            qWarning() << "invalid switching policy " << switching_policy;
        }

        QStringList options = kxkbrc.readEntry<QString>("Options", "").split(",");
        QStringList miscOptions;
        for (QString const& option : options) {
            QString groupName = option.split(":")[0];

            if (groupName == "grp") {
                m_layoutConfigModel->setMainShiftKey(option);
            }
            else if (groupName == "lv3") {
                m_layoutConfigModel->setThirdLevelShortcut(option);
            }
            else {
                miscOptions << option;
            }
        }

        m_underlyingAdvancedModel->setEnabledOptions(miscOptions);
    }

    // Alternative Shortcut
    KGlobalAccel::self()->setShortcut(
        m_nextLayoutAction,
        QList<QKeySequence>() << QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_K),
        KGlobalAccel::Autoloading);
    QList<QKeySequence> shortcuts = KGlobalAccel::self()->shortcut(m_nextLayoutAction);
    QKeySequence shortcut = shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();
    m_layoutConfigModel->setAlternativeShortcut(shortcut.toString());

    {
        KConfigGroup kcminputrc(
            KSharedConfig::openConfig(QStringLiteral("kcminputrc")),
            QStringLiteral("Keyboard"));

        m_hardwareConfigModel->setNumlockOnStartup(
            kcminputrc.readEntry<int>("NumLock", static_cast<int>(TriState::UNCHANGED)));
        m_hardwareConfigModel->setKeyboardRepeat(
            kcminputrc.readEntry<int>("KeyboardRepeating", static_cast<int>(TriState::UNCHANGED)));
        m_hardwareConfigModel->setRepeatDelay(
            kcminputrc.readEntry<int>("RepeatDelay", 600));
        m_hardwareConfigModel->setRepeatRate(
            kcminputrc.readEntry<double>("RepeatRate", 25.0));
    }
}

void KcmKeyboard::save()
{
    {
        KConfigGroup kxkbrc(
            KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
            QStringLiteral("Layout"));

        kxkbrc.writeEntry<QString>("Model", m_hardwareConfigModel->keyboardModel());

        kxkbrc.writeEntry<bool>("ShowLayoutIndicator", m_layoutConfigModel->showLayoutIndicator());

        kxkbrc.writeEntry<bool>("ShowSingle", m_layoutConfigModel->showForSingleLayout());

        kxkbrc.writeEntry<bool>("ShowFlag", m_layoutConfigModel->layoutIndicatorShowFlag());
        kxkbrc.writeEntry<bool>("ShowLabel", m_layoutConfigModel->layoutIndicatorShowLabel());

        if (m_layoutConfigModel->switchingPolicyIndex() >= 0 && m_layoutConfigModel->switchingPolicyIndex() < std::end(LayoutConfigModel::switch_modes) - std::begin(LayoutConfigModel::switch_modes)) {
            kxkbrc.writeEntry<QString>("SwitchMode", LayoutConfigModel::switch_modes[m_layoutConfigModel->switchingPolicyIndex()]);
        } else {
            qWarning() << "invalid switching policy index" << m_layoutConfigModel->switchingPolicyIndex();
        }

        QStringList options = m_underlyingAdvancedModel->enabledOptions();
        if (!m_layoutConfigModel->mainShiftKey().isEmpty()) {
            options << m_layoutConfigModel->mainShiftKey();
        }
        if (!m_layoutConfigModel->thirdLevelShortcut().isEmpty()) {
            options << m_layoutConfigModel->thirdLevelShortcut();
        }
        kxkbrc.writeEntry<QString>("Options", options.join(","));

        m_layoutConfigModel->saveEnabledLayouts();

        kxkbrc.sync();
    }

    KGlobalAccel::self()->setShortcut(
        m_nextLayoutAction,
        QList<QKeySequence>() << QKeySequence(m_layoutConfigModel->alternativeShortcut()),
        KGlobalAccel::NoAutoloading);

    {
        KConfigGroup kcminputrc(
            KSharedConfig::openConfig(QStringLiteral("kcminputrc")),
            QStringLiteral("Keyboard"));

        kcminputrc.writeEntry<int>("NumLock", static_cast<int>(m_hardwareConfigModel->numlockOnStartup()));
        kcminputrc.writeEntry<int>("KeyboardRepeating", static_cast<int>(m_hardwareConfigModel->keyboardRepeat()));
        kcminputrc.writeEntry<int>("RepeatDelay", m_hardwareConfigModel->repeatDelay());
        kcminputrc.writeEntry<double>("RepeatRate", m_hardwareConfigModel->repeatRate());

        kcminputrc.sync();
    }

    // dbus call to the kded (in X11) / kwin (in Wayland) to apply the config changes
    QDBusMessage message = QDBusMessage::createSignal(
        KEYBOARD_DBUS_OBJECT_PATH,
        KEYBOARD_DBUS_SERVICE_NAME,
        KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE);
    QDBusConnection::sessionBus().send(message);
}

void KcmKeyboard::defaults()
{
    m_hardwareConfigModel->setKeyboardModel(QStringLiteral("pc104"));

    m_hardwareConfigModel->setNumlockOnStartup(static_cast<int>(TriState::UNCHANGED));
    m_hardwareConfigModel->setKeyboardRepeat(static_cast<int>(TriState::UNCHANGED));
    m_hardwareConfigModel->setRepeatDelay(600);
    m_hardwareConfigModel->setRepeatRate(25.0);

    m_layoutConfigModel->setShowLayoutIndicator(true);
    m_layoutConfigModel->setShowForSingleLayout(false);
    m_layoutConfigModel->setLayoutIndicatorShowFlag(false);
    m_layoutConfigModel->setLayoutIndicatorShowLabel(true);
    m_layoutConfigModel->setSwitchingPolicyIndex(0);
    m_layoutConfigModel->setAlternativeShortcut(
        QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_K).toString());

    changed();
}

void KcmKeyboard::changed()
{
    setNeedsSave(true);
    emit needsSaveChanged();
}

HardwareConfigModel* KcmKeyboard::hardwareModel()
{
    return m_hardwareConfigModel;
}

LayoutConfigModel* KcmKeyboard::layoutModel()
{
    return m_layoutConfigModel;
}

AdvancedFilterProxyModel *KcmKeyboard::advancedModel()
{
    return m_advancedModel;
}

#include <kcm_keyboard.moc>
