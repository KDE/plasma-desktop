/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 by Davide Bettio <davide.bettio@kdemail.net>

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

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include <config-X11.h>
#endif

#include "styleconfdialog.h"
#include "ui_stylepreview.h"
#include "ui_styleconfig.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kautostart.h>
#include <KDebug>
#include <KLibrary>
#include <KColorScheme>
#include <KStandardDirs>
#include <KNewStuff3/KNS3/DownloadDialog>
#include <KConfigGroup>

#include <QFile>
#include <QSettings>
#include <QAbstractItemView>
#include <QLabel>
#include <QGroupBox>
#include <QPainter>
#include <QPixmapCache>
#include <QStyleFactory>
#include <QFormLayout>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QStyle>

#include <KGlobal>
#include <KGlobalSettings>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include <QX11Info>
#endif

#include "../krdb/krdb.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include <X11/Xlib.h>
#endif

// X11 namespace cleanup
#undef Bool
#undef Below
#undef KeyPress
#undef KeyRelease


/**** DLL Interface for kcontrol ****/

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdemacros.h>

K_PLUGIN_FACTORY(KCMStyleFactory, registerPlugin<KCMStyle>();)
K_EXPORT_PLUGIN(KCMStyleFactory("kcmstyle"))


extern "C"
{
    Q_DECL_EXPORT void kcminit_style()
    {
        uint flags = KRdbExportQtSettings | KRdbExportQtColors | KRdbExportXftSettings | KRdbExportGtkTheme;
        KConfig _config( QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals  );
        KConfigGroup config(&_config, "X11");

        // This key is written by the "colors" module.
        bool exportKDEColors = config.readEntry("exportKDEColors", true);
        if (exportKDEColors)
            flags |= KRdbExportColors;
        runRdb( flags );
    }
}

class StylePreview : public QWidget, public Ui::StylePreview
{
public:
    StylePreview(QWidget *parent = nullptr)
    : QWidget(parent)
    {
        setupUi(this);

        // Ensure that the user can't toy with the child widgets.
        // Method borrowed from Qt's qtconfig.
        QList<QWidget*> widgets = findChildren<QWidget*>();
        foreach (QWidget* widget, widgets)
        {
            widget->installEventFilter(this);
            widget->setFocusPolicy(Qt::NoFocus);
        }
    }

    bool eventFilter( QObject* /* obj */, QEvent* ev ) override
    {
        switch( ev->type() )
        {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseMove:
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
            case QEvent::Enter:
            case QEvent::Leave:
            case QEvent::Wheel:
            case QEvent::ContextMenu:
                  return true; // ignore
            default:
                break;
        }
        return false;
    }
};

class StyleConfig : public QWidget, public Ui::StyleConfig
{
public:
    StyleConfig(QWidget *parent = nullptr)
    : QWidget(parent)
    {
        setupUi(this);
    }
};

QString KCMStyle::defaultStyle()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    return QStringLiteral("breeze");
#else
    return QString(); // native style
#endif
}

KCMStyle::KCMStyle( QWidget* parent, const QVariantList& )
    : KCModule( parent ), appliedStyle(nullptr)
{
    setQuickHelp( i18n("This module allows you to modify the visual appearance "
                       "of applications' user interface elements."));

    m_bStyleDirty= false;
    m_bEffectsDirty = false;


    KGlobal::dirs()->addResourceType("themes", "data", "kstyle/themes");

    KAboutData *about =
        new KAboutData( QStringLiteral("kcmstyle"), i18n("Application Style"), QStringLiteral("1.0"),
                        QString(), KAboutLicense::GPL,
                        i18n("(c) 2002 Karol Szwed, Daniel Molkentin"));

    about->addAuthor(i18n("Karol Szwed"), QString(), QStringLiteral("gallium@kde.org"));
    about->addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    setAboutData( about );

    // Setup mainLayout
    mainLayout = new QVBoxLayout( this );
    mainLayout->setContentsMargins(0, 0, 0, 0);

    styleConfig = new StyleConfig();

    QHBoxLayout *previewLayout = new QHBoxLayout();
    QGroupBox *gbPreview = new QGroupBox( i18n( "Preview" ) );
    QHBoxLayout *previewLayoutInner = new QHBoxLayout(gbPreview);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayoutInner->setContentsMargins(0, 0, 0, 0);
    previewLayout->addStretch();
    previewLayout->addWidget( gbPreview );
    previewLayout->addStretch();

    stylePreview = new StylePreview( gbPreview );
    previewLayoutInner->addWidget( stylePreview );

    mainLayout->addWidget( styleConfig );
    mainLayout->addLayout( previewLayout );
    mainLayout->addStretch();

    connect( styleConfig->comboStyle, SIGNAL(activated(int)), this, SLOT(styleChanged()) );
    connect( styleConfig->comboStyle, SIGNAL(activated(int)), this, SLOT(updateConfigButton()) );
    connect( styleConfig->pbConfigStyle, &QAbstractButton::clicked, this, &KCMStyle::styleSpecificConfig );
    connect( styleConfig->comboStyle, SIGNAL(activated(int)), this, SLOT(setStyleDirty()) );
    connect( styleConfig->cbIconsOnButtons, &QAbstractButton::toggled, this, &KCMStyle::setEffectsDirty );
    connect( styleConfig->cbIconsInMenus, &QAbstractButton::toggled, this, &KCMStyle::setEffectsDirty );
    connect( styleConfig->comboToolbarIcons, SIGNAL(activated(int)), this, SLOT(setEffectsDirty()) );
    connect( styleConfig->comboSecondaryToolbarIcons, SIGNAL(activated(int)), this, SLOT(setEffectsDirty()) );

    addWhatsThis();
}


KCMStyle::~KCMStyle()
{
    qDeleteAll(styleEntries);
    delete appliedStyle;
}

void KCMStyle::updateConfigButton()
{
    if (!styleEntries[currentStyle()] || styleEntries[currentStyle()]->configPage.isEmpty()) {
        styleConfig->pbConfigStyle->setEnabled(false);
        return;
    }

    // We don't check whether it's loadable here -
    // lets us report an error and not waste time
    // loading things if the user doesn't click the button
    styleConfig->pbConfigStyle->setEnabled( true );
}

void KCMStyle::styleSpecificConfig()
{
    QString libname = styleEntries[currentStyle()]->configPage;

    KLibrary library(libname);
    if (!library.load()) {
        KMessageBox::detailedError(this,
            i18n("There was an error loading the configuration dialog for this style."),
            library.errorString(),
            i18n("Unable to Load Dialog"));
        return;
    }

    KLibrary::void_function_ptr allocPtr = library.resolveFunction("allocate_kstyle_config");

    if (!allocPtr)
    {
        KMessageBox::detailedError(this,
            i18n("There was an error loading the configuration dialog for this style."),
            library.errorString(),
            i18n("Unable to Load Dialog"));
        return;
    }

    //Create the container dialog
    StyleConfigDialog* dial = new StyleConfigDialog(this, styleEntries[currentStyle()]->name);

    typedef QWidget*(* factoryRoutine)( QWidget* parent );

    //Get the factory, and make the widget.
    factoryRoutine factory      = (factoryRoutine)(allocPtr); //Grmbl. So here I am on my
    //"never use C casts" moralizing streak, and I find that one can't go void* -> function ptr
    //even with a reinterpret_cast.

    QWidget*       pluginConfig = factory( dial );

    //Insert it in...
    dial->setMainWidget( pluginConfig );

    //..and connect it to the wrapper
    connect(pluginConfig, SIGNAL(changed(bool)), dial, SLOT(setDirty(bool)));
    connect(dial, SIGNAL(defaults()), pluginConfig, SLOT(defaults()));
    connect(dial, SIGNAL(save()), pluginConfig, SLOT(save()));

    if (dial->exec() == QDialog::Accepted  && dial->isDirty() ) {
        // Force re-rendering of the preview, to apply settings
        switchStyle(currentStyle(), true);

        //For now, ask all KDE apps to recreate their styles to apply the setitngs
        KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);

        // We call setStyleDirty here to make sure we force style re-creation
        setStyleDirty();
    }

    delete dial;
}

void KCMStyle::changeEvent( QEvent *event )
{
    KCModule::changeEvent( event );
    if ( event->type() == QEvent::PaletteChange ) {
        // Force re-rendering of the preview, to apply new palette
        switchStyle(currentStyle(), true);
    }
}

void KCMStyle::load()
{
    KConfig config( QStringLiteral("kdeglobals"), KConfig::FullConfig );

    loadStyle( config );
    loadEffects( config );

    m_bStyleDirty= false;
    m_bEffectsDirty = false;
    //Enable/disable the button for the initial style
    updateConfigButton();

    emit changed( false );
}


void KCMStyle::save()
{
    // Don't do anything if we don't need to.
    if ( !(m_bStyleDirty | m_bEffectsDirty ) )
        return;

    // Save effects.
        KConfig      _config(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
        KConfigGroup config(&_config, "KDE");
    // Effects page
    config.writeEntry( "ShowIconsOnPushButtons", styleConfig->cbIconsOnButtons->isChecked());
    config.writeEntry( "ShowIconsInMenuItems", styleConfig->cbIconsInMenus->isChecked());

    config.writeEntry("widgetStyle", currentStyle());

    KConfigGroup toolbarStyleGroup(&_config, "Toolbar style");
    toolbarStyleGroup.writeEntry("ToolButtonStyle",
                            toolbarButtonText(styleConfig->comboToolbarIcons->currentIndex()));
    toolbarStyleGroup.writeEntry("ToolButtonStyleOtherToolbars",
                            toolbarButtonText(styleConfig->comboSecondaryToolbarIcons->currentIndex()));

    _config.sync();

    // Export the changes we made to qtrc, and update all qt-only
    // applications on the fly, ensuring that we still follow the user's
    // export fonts/colors settings.
    if (m_bStyleDirty || m_bEffectsDirty)    // Export only if necessary
    {
        uint flags = KRdbExportQtSettings | KRdbExportGtkTheme;
        KConfig _kconfig( QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals  );
        KConfigGroup kconfig(&_kconfig, "X11");
        bool exportKDEColors = kconfig.readEntry("exportKDEColors", true);
        if (exportKDEColors)
            flags |= KRdbExportColors;
        runRdb( flags );
    }

    // Now allow KDE apps to reconfigure themselves.
    if ( m_bStyleDirty )
        KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);

    if ( m_bEffectsDirty ) {
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_STYLE);
        // ##### FIXME - Doesn't apply all settings correctly due to bugs in
        // KApplication/KToolbar
        KGlobalSettings::self()->emitChange(KGlobalSettings::ToolbarStyleChanged);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        // Send signal to all kwin instances
        QDBusMessage message =
        QDBusMessage::createSignal(QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"));
        QDBusConnection::sessionBus().send(message);
#endif
    }

    // Clean up
    m_bStyleDirty    = false;
    m_bEffectsDirty  = false;
    emit changed( false );
}


bool KCMStyle::findStyle( const QString& str, int& combobox_item )
{
    StyleEntry* se   = styleEntries[str.toLower()];

    QString     name = se ? se->name : str;

    combobox_item = 0;

    //look up name
    for( int i = 0; i < styleConfig->comboStyle->count(); i++ )
    {
        if ( styleConfig->comboStyle->itemText(i) == name )
        {
            combobox_item = i;
            return true;
        }
    }

    return false;
}


void KCMStyle::defaults()
{
    // Select default style
    int item = 0;
    bool found;

    found = findStyle( defaultStyle(), item );
    if (!found)
        found = findStyle( QStringLiteral("oxygen"), item );
    if (!found)
        found = findStyle( QStringLiteral("plastique"), item );
    if (!found)
        found = findStyle( QStringLiteral("windows"), item );
    if (!found)
        found = findStyle( QStringLiteral("platinum"), item );
    if (!found)
        found = findStyle( QStringLiteral("motif"), item );

    styleConfig->comboStyle->setCurrentIndex( item );

    m_bStyleDirty = true;
    switchStyle( currentStyle() );  // make resets visible

    // Effects
    styleConfig->comboToolbarIcons->setCurrentIndex(toolbarButtonIndex(QStringLiteral("TextBesideIcon")));
    styleConfig->comboSecondaryToolbarIcons->setCurrentIndex(toolbarButtonIndex(QStringLiteral("TextBesideIcon")));
    styleConfig->cbIconsOnButtons->setChecked(true);
    styleConfig->cbIconsInMenus->setChecked(true);
    emit changed(true);
    emit updateConfigButton();
}

void KCMStyle::setEffectsDirty()
{
    m_bEffectsDirty = true;
    emit changed(true);
}

void KCMStyle::setStyleDirty()
{
    m_bStyleDirty = true;
    emit changed(true);
}

// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

void KCMStyle::loadStyle( KConfig& config )
{
    styleConfig->comboStyle->clear();
    // Create a dictionary of WidgetStyle to Name and Desc. mappings,
    // as well as the config page info
    qDeleteAll(styleEntries);
    styleEntries.clear();

    QString strWidgetStyle;
    QStringList list = KGlobal::dirs()->findAllResources("themes", QStringLiteral("*.themerc"),
                                                         KStandardDirs::Recursive |
                                                         KStandardDirs::NoDuplicates);
    for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
    {
        KConfig config(  *it, KConfig::SimpleConfig);
        if ( !(config.hasGroup("KDE") && config.hasGroup("Misc")) )
            continue;

        KConfigGroup configGroup = config.group("KDE");

        strWidgetStyle = configGroup.readEntry("WidgetStyle");
        if (strWidgetStyle.isNull())
            continue;

        // We have a widgetstyle, so lets read the i18n entries for it...
        StyleEntry* entry = new StyleEntry;
        configGroup = config.group("Misc");
        entry->name = configGroup.readEntry("Name");
        entry->desc = configGroup.readEntry("Comment", i18n("No description available."));
        entry->configPage = configGroup.readEntry("ConfigPage", QString());

        // Check if this style should be shown
        configGroup = config.group("Desktop Entry");
        entry->hidden = configGroup.readEntry("Hidden", false);

        // Insert the entry into our dictionary.
        styleEntries.insert(strWidgetStyle.toLower(), entry);
    }

    // Obtain all style names
    QStringList allStyles = QStyleFactory::keys();

    // Get translated names, remove all hidden style entries.
    QStringList styles;
    StyleEntry* entry;
    for (QStringList::iterator it = allStyles.begin(); it != allStyles.end(); ++it)
    {
        QString id = (*it).toLower();
        // Find the entry.
        if ( (entry = styleEntries[id]) != nullptr )
        {
            // Do not add hidden entries
            if (entry->hidden)
                continue;

            styles += entry->name;

            nameToStyleKey[entry->name] = id;
        }
        else
        {
            styles += (*it); //Fall back to the key (but in original case)
            nameToStyleKey[*it] = id;
        }
    }

    // Sort the style list, and add it to the combobox
    styles.sort();
    styleConfig->comboStyle->addItems( styles );

    // Find out which style is currently being used
    KConfigGroup configGroup = config.group( "KDE" );
    QString defaultStyle = KCMStyle::defaultStyle();
    QString cfgStyle = configGroup.readEntry( "widgetStyle", defaultStyle );

    // Select the current style
    // Do not use comboStyle->listBox() as this may be NULL for some styles when
    // they use QPopupMenus for the drop-down list!

    // ##### Since Trolltech likes to seemingly copy & paste code,
    // QStringList::findItem() doesn't have a Qt::StringComparisonMode field.
    // We roll our own (yuck)
    cfgStyle = cfgStyle.toLower();
    int item = 0;
    for( int i = 0; i < styleConfig->comboStyle->count(); i++ )
    {
        QString id = nameToStyleKey[styleConfig->comboStyle->itemText(i)];
        item = i;
        if ( id == cfgStyle )   // ExactMatch
            break;
        else if ( id.contains( cfgStyle ) )
            break;
        else if ( id.contains( QApplication::style()->metaObject()->className() ) )
            break;
        item = 0;
    }
    styleConfig->comboStyle->setCurrentIndex( item );
    m_bStyleDirty = false;

    switchStyle( currentStyle() );  // make resets visible
}

QString KCMStyle::currentStyle()
{
    return nameToStyleKey[styleConfig->comboStyle->currentText()];
}


void KCMStyle::styleChanged()
{
    switchStyle( currentStyle() );
}


void KCMStyle::switchStyle(const QString& styleName, bool force)
{
    // Don't flicker the preview if the same style is chosen in the cb
    if (!force && appliedStyle && appliedStyle->objectName() == styleName)
        return;

    // Create an instance of the new style...
    QStyle* style = QStyleFactory::create(styleName);
    if (!style)
        return;

    // Prevent Qt from wrongly caching radio button images
    QPixmapCache::clear();

    setStyleRecursive( stylePreview, style );

    // this flickers, but reliably draws the widgets correctly.
    stylePreview->resize( stylePreview->sizeHint() );

    delete appliedStyle;
    appliedStyle = style;
}

void KCMStyle::setStyleRecursive(QWidget* w, QStyle* s)
{
    // Don't let broken styles kill the palette
    // for other styles being previewed. (e.g SGI style)
    w->setPalette(QPalette());

    QPalette newPalette(KGlobalSettings::createApplicationPalette());
    s->polish( newPalette );
    w->setPalette(newPalette);

    // Apply the new style.
    w->setStyle(s);

    // Recursively update all children.
    const QObjectList children = w->children();

    // Apply the style to each child widget.
    foreach (QObject* child, children)
    {
        if (child->isWidgetType())
            setStyleRecursive((QWidget *) child, s);
    }
}

// ----------------------------------------------------------------
// All the Effects stuff
// ----------------------------------------------------------------
QString KCMStyle::toolbarButtonText(int index)
{
    switch (index) {
        case 1:
            return QStringLiteral("TextOnly");
        case 2:
            return QStringLiteral("TextBesideIcon");
        case 3:
            return QStringLiteral("TextUnderIcon");
        default:
            break;
    }

    return QStringLiteral("NoText");
}

int KCMStyle::toolbarButtonIndex(const QString &text)
{
    if (text == QLatin1String("TextOnly")) {
        return 1;
    } else if (text == QLatin1String("TextBesideIcon")) {
        return 2;
    } else if (text == QLatin1String("TextUnderIcon")) {
        return 3;
    }

    return 0;
}

QString KCMStyle::menuBarStyleText(int index)
{
    switch (index) {
        case 1:
            return QStringLiteral("Decoration");
        case 2:
            return QStringLiteral("Widget");
    }

    return QStringLiteral("InApplication");
}

int KCMStyle::menuBarStyleIndex(const QString &text)
{
    if (text == QLatin1String("Decoration")) {
        return 1;
    } else if (text == QLatin1String("Widget")) {
        return 2;
    }

    return 0;
}

void KCMStyle::loadEffects( KConfig& config )
{
    // KDE's Part via KConfig
    KConfigGroup configGroup = config.group("Toolbar style");

    QString tbIcon = configGroup.readEntry("ToolButtonStyle", "TextBesideIcon");
    styleConfig->comboToolbarIcons->setCurrentIndex(toolbarButtonIndex(tbIcon));
    tbIcon = configGroup.readEntry("ToolButtonStyleOtherToolbars", "TextBesideIcon");
    styleConfig->comboSecondaryToolbarIcons->setCurrentIndex(toolbarButtonIndex(tbIcon));

    configGroup = config.group("KDE");
    styleConfig->cbIconsOnButtons->setChecked(configGroup.readEntry("ShowIconsOnPushButtons", true));
    styleConfig->cbIconsInMenus->setChecked(configGroup.readEntry("ShowIconsInMenuItems", true));

    m_bEffectsDirty = false;
}

void KCMStyle::addWhatsThis()
{
    stylePreview->setWhatsThis( i18n("This area shows a preview of the currently selected style "
                            "without having to apply it to the whole desktop.") );
    styleConfig->comboStyle->setWhatsThis( i18n("Here you can choose from a list of"
                            " predefined widget styles (e.g. the way buttons are drawn) which"
                            " may or may not be combined with a theme (additional information"
                            " like a marble texture or a gradient).") );
    styleConfig->cbIconsOnButtons->setWhatsThis( i18n( "If you enable this option, applications will "
                            "show small icons alongside some important buttons.") );
    styleConfig->cbIconsInMenus->setWhatsThis( i18n( "If you enable this option, applications will "
                            "show small icons alongside most menu items.") );
    styleConfig->comboToolbarIcons->setWhatsThis( i18n( "<p><b>No text:</b> Shows only icons on toolbar buttons. "
                            "Best option for low resolutions.</p>"
                            "<p><b>Text only: </b>Shows only text on toolbar buttons.</p>"
                            "<p><b>Text beside icons: </b> Shows icons and text on toolbar buttons. "
                            "Text is aligned beside the icon.</p>"
                            "<b>Text below icons: </b> Shows icons and text on toolbar buttons. "
                            "Text is aligned below the icon.") );
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
