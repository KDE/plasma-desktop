/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 by Davide Bettio <davide.bettio@kdemail.net>
 * Copyright (C) 2019 Kai Uwe Broulik <kde@broulik.de>
 * Copyright (C) 2019 Cyril Rossi <cyril.rossi@enioka.com>
 *
 * Portions Copyright (C) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 * Portions Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 * Portions Copyright (C) 2008 by Petri Damsten <damu@iki.fi>
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcmstyle.h"

#include "styleconfdialog.h"

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

#include <QDBusReply>
#include <QLibrary>
#include <QMetaEnum>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QScopedPointer>
#include <QStyleFactory>
#include <QWidget>
#include <QWindow>

#include <KGlobal>
#include <KGlobalSettings>

#include "../krdb/krdb.h"

#include "stylesmodel.h"
#include "previewitem.h"
#include "stylesettings.h"
#include "styledata.h"
#include "gtkpage.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMStyleFactory, "kcm_style.json", registerPlugin<KCMStyle>();registerPlugin<StyleData>();)

extern "C"
{
    Q_DECL_EXPORT void kcminit_style()
    {
        uint flags = KRdbExportQtSettings | KRdbExportQtColors | KRdbExportXftSettings | KRdbExportGtkTheme;
        KConfig _config( QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals  );
        KConfigGroup config(&_config, "X11");

        // This key is written by the "colors" module.
        bool exportKDEColors = config.readEntry("exportKDEColors", true);
        if (exportKDEColors) {
            flags |= KRdbExportColors;
        }
        runRdb( flags );
    }
}

KCMStyle::KCMStyle(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new StyleData(this))
    , m_model(new StylesModel(this))
    , m_gtkPage()
{
    qmlRegisterUncreatableType<KCMStyle>("org.kde.private.kcms.style", 1, 0, "KCM", QStringLiteral("Cannot create instances of KCM"));
    qmlRegisterType<StyleSettings>();
    qmlRegisterType<StylesModel>();
    qmlRegisterType<PreviewItem>("org.kde.private.kcms.style", 1, 0, "PreviewItem");

    KAboutData *about =
        new KAboutData( QStringLiteral("kcm_style"), i18n("Application Style"), QStringLiteral("2.0"),
                        QString(), KAboutLicense::GPL,
                        i18n("(c) 2002 Karol Szwed, Daniel Molkentin, (c) 2019 Kai Uwe Broulik"));

    about->addAuthor(i18n("Karol Szwed"), QString(), QStringLiteral("gallium@kde.org"));
    about->addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@broulik.de"));
    setAboutData(about);

    if (gtkConfigKdedModuleLoaded()) {
        m_gtkPage = new GtkPage(this);
        connect(m_gtkPage, &GtkPage::gtkThemeSettingsChanged, this, [this](){
            setNeedsSave(true);
        });
    }
    connect(m_model, &StylesModel::selectedStyleChanged, this, [this](const QString &style) {
        styleSettings()->setWidgetStyle(style);
    });
    connect(styleSettings(), &StyleSettings::widgetStyleChanged, this, [this] {
        m_model->setSelectedStyle(styleSettings()->widgetStyle());
    });
    connect(styleSettings(), &StyleSettings::iconsOnButtonsChanged, this, [this] {
        m_effectsDirty = true;
    });
    connect(styleSettings(), &StyleSettings::iconsInMenusChanged, this, [this] {
        m_effectsDirty = true;
    });
}

KCMStyle::~KCMStyle() = default;

StylesModel *KCMStyle::model() const
{
    return m_model;
}

StyleSettings *KCMStyle::styleSettings() const
{
    return m_data->settings();
}

KCMStyle::ToolBarStyle KCMStyle::mainToolBarStyle() const
{
    return m_mainToolBarStyle;
}

void KCMStyle::setMainToolBarStyle(ToolBarStyle style)
{
    if (m_mainToolBarStyle != style) {
        m_mainToolBarStyle = style;
        emit mainToolBarStyleChanged();

        const QMetaEnum toolBarStyleEnum = QMetaEnum::fromType<ToolBarStyle>();
        styleSettings()->setToolButtonStyle(toolBarStyleEnum.valueToKey(m_mainToolBarStyle));
        m_effectsDirty = true;
    }
}

KCMStyle::ToolBarStyle KCMStyle::otherToolBarStyle() const
{
    return m_otherToolBarStyle;
}

void KCMStyle::setOtherToolBarStyle(ToolBarStyle style)
{
    if (m_otherToolBarStyle != style) {
        m_otherToolBarStyle = style;
        emit otherToolBarStyleChanged();

        const QMetaEnum toolBarStyleEnum = QMetaEnum::fromType<ToolBarStyle>();
        styleSettings()->setToolButtonStyleOtherToolbars(toolBarStyleEnum.valueToKey(m_otherToolBarStyle));
        m_effectsDirty = true;
    }
}

void KCMStyle::configure(const QString &title, const QString &styleName, QQuickItem *ctx)
{
    if (m_styleConfigDialog) {
        return;
    }

    const QString configPage = m_model->styleConfigPage(styleName);
    if (configPage.isEmpty()) {
        return;
    }

    QLibrary library(KPluginLoader::findPlugin(configPage));
    if (!library.load()) {
        qWarning() << "Failed to load style config page" << configPage << library.errorString();
        emit showErrorMessage(i18n("There was an error loading the configuration dialog for this style."));
        return;
    }

    auto allocPtr = library.resolve("allocate_kstyle_config");
    if (!allocPtr) {
        qWarning() << "Failed to resolve allocate_kstyle_config in" << configPage;
        emit showErrorMessage(i18n("There was an error loading the configuration dialog for this style."));
        return;
    }

    m_styleConfigDialog = new StyleConfigDialog(nullptr/*this*/, title);
    m_styleConfigDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_styleConfigDialog->setWindowModality(Qt::WindowModal);
    m_styleConfigDialog->winId(); // so it creates windowHandle

    if (ctx && ctx->window()) {
        if (QWindow *actualWindow = QQuickRenderControl::renderWindowFor(ctx->window())) {
            m_styleConfigDialog->windowHandle()->setTransientParent(actualWindow);
        }
    }

    typedef QWidget*(* factoryRoutine)( QWidget* parent );

    //Get the factory, and make the widget.
    factoryRoutine factory      = (factoryRoutine)(allocPtr); //Grmbl. So here I am on my
    //"never use C casts" moralizing streak, and I find that one can't go void* -> function ptr
    //even with a reinterpret_cast.

    QWidget*       pluginConfig = factory( m_styleConfigDialog.data() );

    //Insert it in...
    m_styleConfigDialog->setMainWidget( pluginConfig );

    //..and connect it to the wrapper
    connect(pluginConfig, SIGNAL(changed(bool)), m_styleConfigDialog.data(), SLOT(setDirty(bool)));
    connect(m_styleConfigDialog.data(), SIGNAL(defaults()), pluginConfig, SLOT(defaults()));
    connect(m_styleConfigDialog.data(), SIGNAL(save()), pluginConfig, SLOT(save()));

    connect(m_styleConfigDialog.data(), &QDialog::accepted, this, [this, styleName] {
        if (!m_styleConfigDialog->isDirty()) {
            return;
        }

        // Force re-rendering of the preview, to apply settings
        emit styleReconfigured(styleName);

        //For now, ask all KDE apps to recreate their styles to apply the setitngs
        KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);

        // When user edited a style, assume they want to use it, too
        styleSettings()->setWidgetStyle(styleName);

        // We call setNeedsSave(true) here to make sure we force style re-creation
        setNeedsSave(true);
    });

    m_styleConfigDialog->show();
}

bool KCMStyle::gtkConfigKdedModuleLoaded()
{
    QDBusInterface kdedInterface(
        QStringLiteral("org.kde.kded5"),
        QStringLiteral("/kded"),
        QStringLiteral("org.kde.kded5")
    );
    QDBusReply<QStringList> loadedKdedModules = kdedInterface.call(QStringLiteral("loadedModules"));
    return loadedKdedModules.value().contains(QStringLiteral("gtkconfig"));
}

void KCMStyle::load()
{
    if (m_gtkPage) {
        m_gtkPage->load();
    }

    ManagedConfigModule::load();
    m_model->load();
    m_previousStyle = styleSettings()->widgetStyle();

    loadSettingsToModel();

    m_effectsDirty = false;
}

void KCMStyle::save()
{
    if (m_gtkPage) {
        m_gtkPage->save();
    }

    // Check whether the new style can actually be loaded before saving it.
    // Otherwise apps will use the default style despite something else having been written to the config
    bool newStyleLoaded = false;
    if (styleSettings()->widgetStyle() != m_previousStyle) {
        QScopedPointer<QStyle> newStyle(QStyleFactory::create(styleSettings()->widgetStyle()));
        if (newStyle) {
            newStyleLoaded = true;
            m_previousStyle = styleSettings()->widgetStyle();
        } else {
            const QString styleDisplay = m_model->data(m_model->index(m_model->indexOfStyle(styleSettings()->widgetStyle()), 0), Qt::DisplayRole).toString();
            emit showErrorMessage(i18n("Failed to apply selected style '%1'.", styleDisplay));

            // Reset selected style back to current in case of failure
            styleSettings()->setWidgetStyle(m_previousStyle);
        }
    }

    ManagedConfigModule::save();

    // Export the changes we made to qtrc, and update all qt-only
    // applications on the fly, ensuring that we still follow the user's
    // export fonts/colors settings.
    uint flags = KRdbExportQtSettings | KRdbExportGtkTheme;
    KConfig _kconfig( QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals  );
    KConfigGroup kconfig(&_kconfig, "X11");
    bool exportKDEColors = kconfig.readEntry("exportKDEColors", true);
    if (exportKDEColors) {
        flags |= KRdbExportColors;
    }
    runRdb( flags );

    // Now allow KDE apps to reconfigure themselves.
    if (newStyleLoaded) {
        KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);
    }

    if (m_effectsDirty) {
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_STYLE);
        // ##### FIXME - Doesn't apply all settings correctly due to bugs in
        // KApplication/KToolbar
        KGlobalSettings::self()->emitChange(KGlobalSettings::ToolbarStyleChanged);
    }

    m_effectsDirty = false;
}

void KCMStyle::defaults()
{
    if (m_gtkPage) {
        m_gtkPage->defaults();
    }

    // TODO the old code had a fallback chain but do we actually support not having Breeze for Plasma?
    // defaultStyle() -> oxygen -> plastique -> windows -> platinum -> motif

    ManagedConfigModule::defaults();

    loadSettingsToModel();
}

void KCMStyle::loadSettingsToModel()
{
    emit styleSettings()->widgetStyleChanged();

    const QMetaEnum toolBarStyleEnum = QMetaEnum::fromType<ToolBarStyle>();
    setMainToolBarStyle(static_cast<ToolBarStyle>(toolBarStyleEnum.keyToValue(qUtf8Printable(styleSettings()->toolButtonStyle()))));
    setOtherToolBarStyle(static_cast<ToolBarStyle>(toolBarStyleEnum.keyToValue(qUtf8Printable(styleSettings()->toolButtonStyleOtherToolbars()))));
}

#include "kcmstyle.moc"
