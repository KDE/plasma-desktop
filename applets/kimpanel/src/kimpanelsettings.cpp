#include "kimpanelsettings.h"
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kdebug.h>

namespace KIM
{
    class SettingsHelper
    {
      public:
        SettingsHelper() : q(0) {}
        // q is also static, no need to delete it here
        ~SettingsHelper() {}
        Settings *q;
    };
    K_GLOBAL_STATIC(SettingsHelper, s_globalSettings)
    Settings *Settings::self()
    {
      if (!s_globalSettings->q) {
        s_globalSettings->q = new Settings;
        s_globalSettings->q->readConfig();
      }

      return s_globalSettings->q;
    }

    Settings::Settings()
    {
        //FIXME: if/when kconfig gets change notification, this will be unecessary
        KDirWatch::self()->addFile(KStandardDirs::locateLocal("config", "kimpanelrc"));
        connect(KDirWatch::self(), SIGNAL(created(const QString &)), this, SLOT(settingsFileChanged()));
        connect(KDirWatch::self(), SIGNAL(dirty(const QString &)), this, SLOT(settingsFileChanged()));
    }

    Settings::~Settings()
    {
    }

    void Settings::settingsFileChanged()
    {
        readConfig();
        emit configChanged();
    }
} // namespace KIM

// vim: sw=4 sts=4 et tw=100
