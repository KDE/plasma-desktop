#include "kimthemepackage.h"

#include <plasma/packagestructure.h>
#include <plasma/packagemetadata.h>
#include <knewstuff2/engine.h>
#include <KStandardDirs>
#include <KMessageBox>
#include <karchive.h>
#include <kcomponentdata.h>
#include <kdesktopfile.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kzip.h>

#include <QWidget>
#include <QString>
#include <QDir>
#include <QFile>
#include <QRegExp>

namespace KIM
{
    ThemePackage::ThemePackage(QObject *parent)
        : Plasma::PackageStructure(parent, QString("KIM Panel Theme"))
    {
        QStringList mimetypes;
        mimetypes << "image/svg+xml" << "text/*";
        setDefaultMimetypes(mimetypes);
    }

#if 0
    void ThemePackage::createNewWidgetBrowser(QWidget *parent)
    {
        KNS::Engine engine(parent);
        if (engine.init("kimthemes.knsrc")) {
            KNS::Entry::List entries = engine.downloadDialogModal(parent);

            foreach (KNS::Entry *entry, entries) {
                if (entry->status() != KNS::Entry::Installed) {
                    continue;
                }

                // install the packges!
                foreach (const QString &package, entry->installedFiles()) {
                    if (!installTheme(package)) {
                        kDebug() << "FAIL! on install of" << package;
                        KMessageBox::error(0, i18n("Installation of <b>%1</b> failed.", package),
                                i18n("Installation Failed"));
                    }
                    kDebug() << package;
                }
            }
        }
    }
#endif

    bool ThemePackage::installTheme(const QString &package)
    {
        QString packageRoot = KStandardDirs::locateLocal("data","kimpanel/themes/");
        //TODO: report *what* failed if something does fail
        QDir root(packageRoot);

        if (!root.exists()) {
            KStandardDirs::makeDir(packageRoot);
            if (!root.exists()) {
                kWarning() << "Could not create package root directory:" << packageRoot;
                return false;
            }
        }

        QFileInfo fileInfo(package);
        if (!fileInfo.exists()) {
            kWarning() << "No such file:" << package;
            return false;
        }

        QString path;
        KTempDir tempdir;
        bool archivedPackage = false;

        if (fileInfo.isDir()) {
            // we have a directory, so let's just install what is in there
            path = package;

            // make sure we end in a slash!
            if (path[path.size() - 1] != '/') {
                path.append('/');
            }
        } else {
            KZip archive(package);
            if (!archive.open(QIODevice::ReadOnly)) {
                kWarning() << "Could not open package file:" << package;
                return false;
            }

            archivedPackage = true;
            const KArchiveDirectory *source = archive.directory();
            const KArchiveEntry *metadata = source->entry("metadata.desktop");
            if (!metadata) {
                kWarning() << "No metadata file in package" << package;
                return false;
            }

            path = tempdir.name();
            source->copyTo(path);
        }

        QString metadataPath = path + "metadata.desktop";
        if (!QFile::exists(metadataPath)) {
            kWarning() << "No metadata file in package" << package;
            return false;
        }

        Plasma::PackageMetadata meta(metadataPath);
        QString targetName = meta.pluginName();

        if (targetName.isEmpty()) {
            kWarning() << "Package plugin name not specified";
            return false;
        }

        // Ensure that package names are safe so package uninstall can't inject
        // bad characters into the paths used for removal.
        QRegExp validatePluginName("^[\\w-\\.]+$"); // Only allow letters, numbers, underscore and period.
        if (!validatePluginName.exactMatch(targetName)) {
            kWarning() << "Package plugin name " << targetName << "contains invalid characters";
            return false;
        }

        targetName = packageRoot + '/' + targetName;
        if (QFile::exists(targetName)) {
            kWarning() << targetName << "already exists";
            return false;
        }

        if (archivedPackage) {
            // it's in a temp dir, so just move it over.
            KIO::CopyJob *job = KIO::move(KUrl(path), KUrl(targetName), KIO::HideProgressInfo);
            if (!job->exec()) {
                kWarning() << "Could not move package to destination:" << targetName << " : " << job->errorString();
                return false;
            }
        } else {
            // it's a directory containing the stuff, so copy the contents rather
            // than move them
            KIO::CopyJob *job = KIO::copy(KUrl(path), KUrl(targetName), KIO::HideProgressInfo);
            if (!job->exec()) {
                kWarning() << "Could not copy package to destination:" << targetName << " : " << job->errorString();
                return false;
            }
        }

        if (archivedPackage) {
            // no need to remove the temp dir (which has been successfully moved if it's an archive)
            tempdir.setAutoRemove(false);
        }

        return true;
    }
} // namespace KIM
// vim: sw=4 sts=4 et tw=100
