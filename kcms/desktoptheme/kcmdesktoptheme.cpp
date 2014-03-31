/*
 * KCMDesktopTheme
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

#include "kcmdesktoptheme.h"

#include "thememodel.h"

#include <QFileDialog>

#include <kaboutdata.h>
#include <kautostart.h>
#include <KGlobal>
#include <KStandardDirs>
#include <KNewStuff3/KNS3/DownloadDialog>

#include <Plasma/Theme>

/**** DLL Interface for kcontrol ****/

#include <kpluginfactory.h>
#include <kpluginloader.h>

K_PLUGIN_FACTORY_WITH_JSON(KCMDesktopThemeFactory, "desktoptheme.json", registerPlugin<KCMDesktopTheme>();)


KCMDesktopTheme::KCMDesktopTheme( QWidget* parent, const QVariantList& )
    : KCModule( parent )
    , m_dialog(0)
    , m_installProcess(0)
    , m_defaultTheme(new Plasma::Theme(this))
{
    setQuickHelp( i18n("<h1>Desktop Theme</h1>"
            "This module allows you to modify the visual appearance "
            "of the desktop."));

    setupUi(this);

    m_bDesktopThemeDirty = false;
    m_bDetailsDirty = false;

    KAutostart plasmaNetbookAutoStart("plasma-netbook");
    m_isNetbook = plasmaNetbookAutoStart.autostarts();

    KGlobal::dirs()->addResourceType("themes", "data", "kstyle/themes");

    KAboutData *about =
        new KAboutData( I18N_NOOP("KCMDesktopTheme"), 0,
                        i18n("KDE Desktop Theme Module"),
                        0, QString(), KAboutData::License_GPL,
                        i18n("(c) 2002 Karol Szwed, Daniel Molkentin"));

    about->addAuthor(i18n("Karol Szwed"), QString(), QStringLiteral("gallium@kde.org"));
    about->addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    about->addAuthor(i18n("Ralf Nolden"), QString(), QStringLiteral("nolden@kde.org"));
    setAboutData( about );

    m_newThemeButton->setIcon(QIcon::fromTheme("get-hot-new-stuff"));

    m_themeModel = new ThemeModel(this);
    m_theme->setModel(m_themeModel);
    m_theme->setItemDelegate(new ThemeDelegate(m_theme));
    m_theme->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

    connect(m_detailsWidget, SIGNAL(changed()), this, SLOT(detailChanged()));

    connect(m_theme->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setDesktopThemeDirty()));
    connect(m_newThemeButton, SIGNAL(clicked()), this, SLOT(getNewThemes()));
    connect(m_fileInstallButton, &QPushButton::clicked, this, &KCMDesktopTheme::showFileDialog);
}


KCMDesktopTheme::~KCMDesktopTheme()
{
}

void KCMDesktopTheme::load()
{
    KConfig config( "kdeglobals", KConfig::FullConfig );

    loadDesktopTheme();

    m_bDesktopThemeDirty = false;
    m_bDetailsDirty = false;

    emit changed( false );
}


void KCMDesktopTheme::save()
{
    qDebug() << "Save!";
    // Don't do anything if we don't need to.
    if ( !( m_bDesktopThemeDirty) && !(m_bDetailsDirty) )
        return;

    //Desktop theme
    if ( m_bDesktopThemeDirty )
    {
        QString theme = m_themeModel->data(m_theme->currentIndex(), ThemeModel::PackageNameRole).toString();
        qDebug() << "theme changed to " << theme;
        if (m_isNetbook) {
            KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-netbook");
            cg.writeEntry("name", theme);
        } else {
            qDebug() << "m-defaultTheme" << theme;
            m_defaultTheme->setThemeName(theme);
        }
    }

    if (m_bDetailsDirty)
    {
        m_detailsWidget->save();
    }

    // Clean up
    m_bDesktopThemeDirty = false;
    m_bDetailsDirty = false;
    emit changed( false );
    qDebug() << "saved.";
}

void KCMDesktopTheme::defaults()
{
    // TODO: reset back to default theme?
}

void KCMDesktopTheme::setDesktopThemeDirty()
{
    m_bDesktopThemeDirty = true;
    emit changed(true);
}

void KCMDesktopTheme::getNewThemes()
{
    KNS3::DownloadDialog dialog("plasma-themes.knsrc", this);
    dialog.exec();
    KNS3::Entry::List entries = dialog.changedEntries();

    if (entries.size() > 0) {
        loadDesktopTheme();
    }
}

void KCMDesktopTheme::loadDesktopTheme()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_themeModel->reload();
    QString themeName;
    KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme");
    themeName = cg.readEntry("name", m_defaultTheme->themeName());

    m_theme->setCurrentIndex(m_themeModel->indexOf(themeName));
    QApplication::restoreOverrideCursor();
}

void KCMDesktopTheme::detailChanged()
{
    m_bDetailsDirty = true;
    emit changed(true);
}

void KCMDesktopTheme::showFileDialog()
{
    if (!m_dialog) {
        QUrl baseUrl;
        m_dialog = new QFileDialog(m_fileInstallButton, i18n("Open Theme"),
                                      QDir::homePath(),
                                      i18n("Theme Files (*.zip *.tar.gz *.tar.bz2)"));
        m_dialog->setFileMode(QFileDialog::ExistingFile);
        connect(m_dialog, &QDialog::accepted, this, &KCMDesktopTheme::fileBrowserCompleted);
    }

    m_dialog->exec();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void KCMDesktopTheme::fileBrowserCompleted()
{
    if (m_dialog && m_dialog->selectedFiles().count() > 0) {
        foreach (const QString &file, m_dialog->selectedFiles()) {
            qDebug() << "INSTALL Theme file: " << file;
            m_newThemeButton->setEnabled(false);
            installTheme(file);
        }
    }
}

void KCMDesktopTheme::installTheme(const QString &file)
{
    qDebug() << "Installing ... " << file;

    QString program = "plasmapkg2";
    QStringList arguments;
    arguments << "-t" << "theme" << "-i" << file;

    if (!m_installProcess) {
        qDebug() << program << arguments.join(" ");
        QProcess *myProcess = new QProcess(this);
        //connect(myProcess, &QProcess::finished, this, &KCMDesktopTheme::installFinished);
        connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(installFinished(int, QProcess::ExitStatus)));
        connect(myProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(installError(QProcess::ProcessError)));

        myProcess->start(program, arguments);
    } else {
        qWarning() << "theme install process already running, refusing to install simultaneously";
    }
}

void KCMDesktopTheme::installFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    //int exitCode = m_installProcess->exitCode();
    if (exitCode == 0) {
        qDebug() << "Theme installed successfully :)";
        m_themeModel->reload();
        m_statusLabel->setText(i18n("Theme installed successfully."));
    } else {
        qDebug() << "Theme installation failed.";
        m_statusLabel->setText(i18n("Theme installation failed. (%1)", exitCode));
    }
    m_newThemeButton->setEnabled(true);
}

void KCMDesktopTheme::installError(QProcess::ProcessError e)
{
    qDebug() << "Theme installation failed. :(";
    m_statusLabel->setText(i18n("Theme installation failed."));
    m_newThemeButton->setEnabled(true);

}


#include "kcmdesktoptheme.moc"

// vim: set noet ts=4:
