#include "kcm_kimpanel.h"
#include "kimpanelconfig.h"

#include <KAboutData>
#include <KColorButton>
#include <KColorDialog>
#include <KFileDialog>
#include <KGenericFactory>
#include <KGlobal>
#include <KGlobalSettings>
#include <KInputDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <kio/netaccess.h>
#include <knewstuff2/engine.h>

K_PLUGIN_FACTORY( KIMPanelFactory, registerPlugin<KIM::PanelCm>(); )
K_EXPORT_PLUGIN( KIMPanelFactory("kcm_kimpanel") )

namespace KIM
{
    PanelCm::PanelCm(QWidget *parent, const QVariantList &)
        : KCModule( KIMPanelFactory::componentData(), parent)
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
    }

    PanelCm::~PanelCm()
    {
    }

    void PanelCm::load()
    {

    }

    void PanelCm::save()
    {

    }

    void PanelCm::defaults()
    {

    }
} // namespace KIM
// vim: sw=4 sts=4 et tw=100
