#include "kcm_kimpanel.h"
#include "kimpanelsettings.h"

#include <kio/netaccess.h>
#include <knewstuff2/engine.h>
#include <KAboutData>
#include <KAction>
#include <KColorButton>
#include <KColorDialog>
#include <KComboBox>
#include <KDesktopFile>
#include <KFileDialog>
#include <KGenericFactory>
#include <KGlobal>
#include <KGlobalSettings>
#include <KIcon>
#include <KMenu>
#include <KMessageBox>
#include <KStandardDirs>
#include <KUrl>

#include <QAction>
#include <QSignalMapper>

K_PLUGIN_FACTORY( KIMPanelFactory, registerPlugin<KIM::PanelCm>(); )
K_EXPORT_PLUGIN( KIMPanelFactory("kcm_kimpanel") )

namespace KIM
{
    PanelCm::PanelCm(QWidget *parent, const QVariantList &)
        : KCModule( KIMPanelFactory::componentData(), parent),
          m_installer(new KIM::ThemePackage())
    {
        KAboutData* about = new KAboutData(
            "kcm_kimpanel", 0, ki18n("Input method panel"), 0, KLocalizedString(),
            KAboutData::License_GPL,
            ki18n("(c) 2009 Wang Hoi")
        );
        about->addAuthor( ki18n("Wang Hoi"), KLocalizedString(),
                         "zealot.hoi@gmail.com" );
        setAboutData( about );

        setupUi(this);
        m_widgetsMenu = new KMenu(i18n("Get New Widgets"), this);
        connect(m_widgetsMenu, SIGNAL(aboutToShow()), this, SLOT(populateWidgetsMenu()));
        newThemeButton->setMenu(m_widgetsMenu);

        //connect(themeCombox,SIGNAL(curr(int)),this,SIGNAL(changed()));
        connect(themeCombox,SIGNAL(activated(int)),this,SLOT(themeComboxIndexChanged(int)));

        connect(themeCombox,SIGNAL(activated(int)),this,SLOT(checkChanged()));
        connect(preferIconSizeSpin,SIGNAL(valueChanged(int)),this,SLOT(checkChanged()));
        connect(statusbarLayoutCombox,SIGNAL(activated(int)),this,SLOT(checkChanged()));
        connect(lookuptableLayoutCombox,SIGNAL(activated(int)),this,SLOT(checkChanged()));
        connect(lookuptableLayoutConstraint,SIGNAL(valueChanged(int)),this,SLOT(checkChanged()));

        connect(KIM::Settings::self(),SIGNAL(configChanged()),this,SLOT(load()));
    }

    PanelCm::~PanelCm()
    {
    }

    void PanelCm::load()
    {
        kDebug();

        KIM::Settings::self()->readConfig();

        reloadThemes();

        preferIconSizeSpin->setValue(Settings::self()->preferIconSize());

        statusbarLayoutCombox->clear();
        statusbarLayoutCombox->addItem(i18n("Horizontal"),(int)Settings::StatusbarHorizontal);
        statusbarLayoutCombox->addItem(i18n("Vertical"),(int)Settings::StatusbarVertical);
        statusbarLayoutCombox->addItem(i18nc("Arrange icons in multiple rows/cols way","Matrix"),(int)Settings::StatusbarMatrix);

        int idx = statusbarLayoutCombox->findData((int)Settings::self()->floatingStatusbarLayout());
        if (idx == -1) {
            idx = 0;
        }
        statusbarLayoutCombox->setCurrentIndex(idx);

        lookuptableLayoutCombox->clear();
        lookuptableLayoutCombox->addItem(i18n("Horizontal"),(int)Settings::LookupTableHorizontal);
        lookuptableLayoutCombox->addItem(i18n("Vertical"),(int)Settings::LookupTableVertical);
        lookuptableLayoutCombox->addItem(i18nc("Place candidate words in multiple rows and cols, but row count is fixed",
                    "FixedRows"),(int)Settings::LookupTableFixedRows);
        lookuptableLayoutCombox->addItem(i18nc("Place candidate words in multiple rows and cols, but column count is fixed",
                    "FixedColumns"),(int)Settings::LookupTableFixedColumns);

        idx = lookuptableLayoutCombox->findData((int)Settings::self()->lookupTableLayout());
        if (idx == -1) {
            idx = 0;
        }
        lookuptableLayoutCombox->setCurrentIndex(idx);

        bool to_enable = ((lookuptableLayoutCombox->itemData(idx) == (int)Settings::LookupTableFixedRows) || (lookuptableLayoutCombox->itemData(idx) == (int)Settings::LookupTableFixedColumns));
        lookuptableLayoutConstraintLabel->setEnabled(to_enable);
        lookuptableLayoutConstraint->setEnabled(to_enable);

        lookuptableLayoutConstraint->setValue(Settings::self()->lookupTableConstraint());
    }

    void PanelCm::save()
    {
        kDebug();

        Settings::self()->setTheme(themeCombox->itemData(themeCombox->currentIndex()).toString());
        Settings::self()->setPreferIconSize(preferIconSizeSpin->value());
        Settings::self()->setFloatingStatusbarLayout(statusbarLayoutCombox->itemData(statusbarLayoutCombox->currentIndex()).toInt());
        Settings::self()->setLookupTableLayout(lookuptableLayoutCombox->itemData(lookuptableLayoutCombox->currentIndex()).toInt());
        if (lookuptableLayoutConstraint->isEnabled()) {
            Settings::self()->setLookupTableConstraint(lookuptableLayoutConstraint->value());
        }
        Settings::self()->writeConfig();
    }

    void PanelCm::defaults()
    {
        kDebug();
    }

    void PanelCm::populateWidgetsMenu()
    {
        if (!m_widgetsMenu->actions().isEmpty()) {
            // already populated.
            return;
        }

        QAction *action = new QAction(KIcon("applications-internet"),
                                      i18n("Download New KIM Theme"), this);
        connect(action, SIGNAL(triggered()), this, SLOT(downloadThemes()));
        m_widgetsMenu->addAction(action);
        m_widgetsMenu->addSeparator();
        action = new QAction(KIcon("package-x-generic"),
                             i18n("Install Theme From Local File..."), this);
        connect(action, SIGNAL(triggered()), this, SLOT(openThemeFile()));
        m_widgetsMenu->addAction(action);
    }

    void PanelCm::downloadThemes()
    {
        KNS::Engine engine(this);
        if (engine.init("kimpanel-themes.knsrc")) {
            KNS::Entry::List entries = engine.downloadDialogModal(this);

            if (entries.size() > 0) {
                reloadThemes();
            }
            qDeleteAll(entries);
        }
    }

    void PanelCm::openThemeFile()
    {
        QString package = KFileDialog::getOpenFileName(KUrl(),"application/zip",this);

        if (package.isEmpty()) {
            return;
        }

        if (!m_installer->installTheme(package)) {
            kWarning() << "FAIL! on install of" << package;
            KMessageBox::error(0, i18n("Installation of <b>%1</b> failed.", package),
                    i18n("Installation Failed"));
        } else {
            KMessageBox::information(0, i18n("Installation of <b>%1</b> succeds.", package),
                    i18n("Installation Succeds"));
        }
        reloadThemes();
    }

    void PanelCm::reloadThemes()
    {
        QString oldName = themeCombox->itemData(themeCombox->currentIndex()).toString();
        if (oldName.isEmpty()) {
            oldName = Settings::self()->theme();
        }

        KStandardDirs dirs;
        QStringList themePaths = dirs.findAllResources("data", 
               "kimpanel/themes/*/metadata.desktop",
               KStandardDirs::NoDuplicates);
        QStringList themeNames;
        QStringList themeDisplayNames;
        foreach (const QString &path,themePaths) {
            int sepEnd = path.lastIndexOf('/');
            int sepStart = path.lastIndexOf('/',sepEnd-1)+1;
            QString name = path.mid(sepStart,sepEnd-sepStart);
            themeNames << name;
            KDesktopFile df(path);
            themeDisplayNames << df.readName();
        }

        themeCombox->clear();
        themeCombox->addItems(themeDisplayNames);
        for (int i = 0; i<themeNames.size(); ++i) {
            themeCombox->setItemData(i, themeNames.at(i));
        }

        int idx = themeNames.indexOf(oldName);
        // current theme get removed
        if (idx == -1) {
            int default_idx = themeNames.indexOf("default");
            idx = (default_idx==-1)? 0 : default_idx;
            changed();
        }
        themeCombox->setCurrentIndex(idx);
    }

    void PanelCm::themeComboxIndexChanged(int index)
    {
        kDebug() << index;
        checkChanged();
    }

    void PanelCm::checkChanged()
    {
        bool has_changed = false;
        has_changed |= (themeCombox->itemData(themeCombox->currentIndex()).toString() != Settings::self()->theme());
        has_changed |= (preferIconSizeSpin->value() != Settings::self()->preferIconSize());
        has_changed |= (statusbarLayoutCombox->itemData(statusbarLayoutCombox->currentIndex()) != 
                Settings::self()->floatingStatusbarLayout());
        has_changed |= (lookuptableLayoutCombox->itemData(lookuptableLayoutCombox->currentIndex()) != 
                Settings::self()->lookupTableLayout());
        has_changed |= (lookuptableLayoutConstraint->value() != 
                Settings::self()->lookupTableConstraint());

        bool to_enable = ((lookuptableLayoutCombox->itemData(lookuptableLayoutCombox->currentIndex()).toInt() == (int)Settings::LookupTableFixedRows) ||
                (lookuptableLayoutCombox->itemData(lookuptableLayoutCombox->currentIndex()) == (int)Settings::LookupTableFixedColumns));
        lookuptableLayoutConstraintLabel->setEnabled(to_enable);
        lookuptableLayoutConstraint->setEnabled(to_enable);

#if 0
        if (lookuptableLayoutConstraint->enabled()) {
            lookuptableLayoutConstraint->setValue(Settings::self()->lookupTableConstraint());
        }
#endif


        emit changed(has_changed);
    }
} // namespace KIM
// vim: sw=4 sts=4 et tw=100
