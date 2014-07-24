/***************************************************************************
 *   Copyright (C) 2007 by Carlo Segato <brandon.ml@gmail.com>             *
 *   Copyright (C) 2008 Montel Laurent <montel@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "emoticonslist.h"
#include <QString>
#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QListWidgetItem>

#include <KGlobal>
#include <KUrl>
#include <KMessageBox>
#include <KDebug>
#include <KIcon>
#include <KAboutData>
#include <KStandardDirs>
#include <KFileDialog>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KInputDialog>
#include <KUrlRequesterDialog>
#include <kmessagebox_queued.h>
#include <kio/netaccess.h>
#include <KNS3/DownloadDialog>
#include <KLocalizedString>

EditDialog::EditDialog(QWidget *parent, const QString &name)
        : KDialog(parent)
{
    setCaption(name);
    setupDlg();
}

EditDialog::EditDialog(QWidget *parent, const QString &name, QListWidgetItem *itm, const QString &file)
        : KDialog(parent)
{
    setCaption(name);
    emoticon = file;
    setupDlg();
    leText->setText(itm->text());
    btnIcon->setIcon(itm->icon());
}

void EditDialog::setupDlg()
{
    wdg = new QWidget(this);
    QVBoxLayout *vl = new QVBoxLayout;
    QHBoxLayout *hb = new QHBoxLayout;
    leText = new KLineEdit(this);
    btnIcon = new QPushButton(this);
    btnIcon->setFixedSize(QSize(64, 64));
    btnIcon->setIconSize(QSize(64, 64));

    QLabel *lab = new QLabel(i18n("Insert the string for the emoticon.  If you want multiple strings, separate them by spaces."), wdg);
    lab->setWordWrap(true);
    vl->addWidget(lab);
    hb->addWidget(btnIcon);
    hb->addWidget(leText);
    vl->addLayout(hb);
    wdg->setLayout(vl);
    setMainWidget(wdg);
    connect(btnIcon, SIGNAL(clicked()), this, SLOT(btnIconClicked()));
    connect(leText, SIGNAL(textChanged(const QString &)), this, SLOT(updateOkButton()));
    updateOkButton();
    leText->setFocus();
}

void EditDialog::btnIconClicked()
{
    KUrl url =  KFileDialog::getImageOpenUrl();

    if (!url.isLocalFile())
        return;

    emoticon = url.toLocalFile();

    if (emoticon.isEmpty())
        return;

    btnIcon->setIcon(QPixmap(emoticon));
    updateOkButton();
}

void EditDialog::updateOkButton()
{
    enableButtonOk(!leText->text().isEmpty() && !emoticon.isEmpty());
}

K_PLUGIN_FACTORY(EmoticonsFactory, registerPlugin<EmoticonList>();)

EmoticonList::EmoticonList(QWidget *parent, const QVariantList &args)
        : KCModule(parent, args)
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_emoticons"), i18n("Emoticons"), QStringLiteral("1.0"), QString(), KAboutLicense::GPL);
    setAboutData(about);
//     setButtons(Apply | Help);
    setupUi(this);
    btAdd->setIcon(KIcon("list-add"));
    btEdit->setIcon(KIcon("edit-rename"));
    btRemoveEmoticon->setIcon(KIcon("edit-delete"));
    btNew->setIcon(KIcon("document-new"));
    btGetNew->setIcon(KIcon("get-hot-new-stuff"));
    btInstall->setIcon(KIcon("document-import"));
    btRemoveTheme->setIcon(KIcon("edit-delete"));

    connect(themeList, SIGNAL(itemSelectionChanged()), this, SLOT(selectTheme()));
    connect(themeList, SIGNAL(itemSelectionChanged()), this, SLOT(updateButton()));
    connect(btRemoveTheme, SIGNAL(clicked()), this, SLOT(btRemoveThemeClicked()));
    connect(btInstall, SIGNAL(clicked()), this, SLOT(installEmoticonTheme()));
    connect(btNew, SIGNAL(clicked()), this, SLOT(newTheme()));
    connect(btGetNew, SIGNAL(clicked()), this, SLOT(getNewStuff()));
    connect(cbStrict, SIGNAL(clicked()), this, SLOT(somethingChanged()));

    connect(btAdd, SIGNAL(clicked()), this, SLOT(addEmoticon()));
    connect(btEdit, SIGNAL(clicked()), this, SLOT(editEmoticon()));
    connect(btRemoveEmoticon, SIGNAL(clicked()), this, SLOT(btRemoveEmoticonClicked()));
    connect(emoList, SIGNAL(itemSelectionChanged()), this, SLOT(updateButton()));
    connect(emoList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(editEmoticon()));
}

EmoticonList::~EmoticonList()
{
}

bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
    return (QString::localeAwareCompare(s1, s2) < 0);
}

void EmoticonList::load()
{
    KStandardDirs dir;

    delFiles.clear();
    themeList->clear();
    emoMap.clear();
    emoList->clear();

    QStringList themeList = kEmoticons.themeList();
    qSort(themeList.begin(), themeList.end(), caseInsensitiveLessThan);

    for (int i = 0; i < themeList.count(); i++) {
        loadTheme(themeList.at(i));
    }

    if (kEmoticons.parseMode() & KEmoticonsTheme::StrictParse) {
        cbStrict->setChecked(true);
    } else {
        cbStrict->setChecked(false);
    }

    updateButton();
    emit changed(false);
}

void EmoticonList::save()
{
    for (int i = 0; i < delFiles.size(); i++) {
        KIO::NetAccess::del(QUrl(delFiles.at(i)), this);
    }

    foreach (KEmoticonsTheme t, emoMap) {
        t.save();
    }

    if (themeList->currentItem()) {
        kEmoticons.setTheme(themeList->currentItem()->text());
    }

    if (cbStrict->isChecked()) {
        kEmoticons.setParseMode((kEmoticons.parseMode() |= KEmoticonsTheme::StrictParse) &= ~KEmoticonsTheme::RelaxedParse);
    } else {
        kEmoticons.setParseMode((kEmoticons.parseMode() |= KEmoticonsTheme::RelaxedParse) &= ~KEmoticonsTheme::StrictParse);
    }
}

void EmoticonList::somethingChanged()
{
    emit changed(true);
}

void EmoticonList::updateButton()
{
    bool can = canEditTheme();
    btRemoveEmoticon->setEnabled(themeList->currentItem() && emoList->selectedItems().size() && can);
    btRemoveTheme->setEnabled(themeList->currentItem() && themeList->currentItem()->text() != "kde4" && themeList->count() > 1 && can);
    btEdit->setEnabled(themeList->currentItem() && emoList->selectedItems().size() && can);
    btAdd->setEnabled(themeList->currentItem() && can);
}

void EmoticonList::selectTheme()
{

    kDebug() << "current_item: " << themeList->currentItem();

    updateButton();
    if (!themeList->currentItem()) {
        emoList->clear();
        return;
    }

    if (!themeList->currentItem()) {
        themeList->currentItem()->setSelected(true);
        return;
    }
    emoList->clear();

    KEmoticonsTheme em = emoMap.value(themeList->currentItem()->text());
    QHash<QString, QStringList>::const_iterator it = em.emoticonsMap().constBegin();

    for (; it != em.emoticonsMap().constEnd(); ++it) {
        QString text;
        if (it.value().size()) {
            text = it.value().at(0);
            for (int i = 1; i < it.value().size(); i++) {
                text += ' ' + it.value().at(i);
            }
        }

        new QListWidgetItem(QIcon(it.key()), text, emoList);
    }
    emit changed();
}

void EmoticonList::btRemoveThemeClicked()
{
    if (!themeList->currentItem()) {
        return;
    }

    QString name = themeList->currentItem()->text();

    delFiles.append(KStandardDirs::locate("emoticons", name + QDir::separator()));
    delete themeList->currentItem();
    emoMap.remove(name);
    emit changed();
}

void EmoticonList::installEmoticonTheme()
{
    QUrl themeURL = KUrlRequesterDialog::getUrl(QUrl(), this, i18n("Drag or Type Emoticon Theme URL"));
    if (themeURL.isEmpty())
        return;

    if (!themeURL.isLocalFile()) {
        KMessageBox::queuedMessageBox(this, KMessageBox::Error, i18n("Emoticon themes must be installed from local files."),
                                      i18n("Could Not Install Emoticon Theme"));
        return;
    }
    QStringList installed = kEmoticons.installTheme(themeURL.toLocalFile());
    for (int i = 0; i < installed.size(); i++)
        loadTheme(installed.at(i));
}

void EmoticonList::btRemoveEmoticonClicked()
{
    if (!emoList->currentItem()) {
        return;
    }

    QListWidgetItem *itm = emoList->currentItem();
    KEmoticonsTheme theme = emoMap.value(themeList->currentItem()->text());
    QString fPath = theme.emoticonsMap().key(itm->text().split(' '));
    if (theme.removeEmoticon(itm->text())) {
        int ret = KMessageBox::questionYesNo(this, i18n("Do you want to remove %1 too?", fPath), i18n("Delete emoticon"));
        if (ret == KMessageBox::Yes) {
            delFiles.append(fPath);
        }

        delete itm;
        themeList->currentItem()->setIcon(QIcon(previewEmoticon(theme)));
        emit changed();
    }
}

void EmoticonList::addEmoticon()
{
    if (!themeList->currentItem())
        return;

    EditDialog *dlg = new EditDialog(this, i18n("Add Emoticon"));

    if (dlg->exec() == QDialog::Rejected) {
        delete dlg;
        return;
    }

    KEmoticonsTheme theme = emoMap.value(themeList->currentItem()->text());
    if (theme.addEmoticon(dlg->getEmoticon(), dlg->getText(), KEmoticonsProvider::Copy)) {
        new QListWidgetItem(QPixmap(dlg->getEmoticon()), dlg->getText(), emoList);
        themeList->currentItem()->setIcon(QIcon(previewEmoticon(theme)));
        emit changed();
    }
    delete dlg;
}

void EmoticonList::editEmoticon()
{
    if (!themeList->currentItem() || !emoList->currentItem())
        return;

    KEmoticonsTheme theme = emoMap.value(themeList->currentItem()->text());
    QString path = theme.emoticonsMap().key(emoList->currentItem()->text().split(' '));
    QString f = QFileInfo(path).fileName();
    EditDialog *dlg = new EditDialog(this, i18n("Edit Emoticon"), emoList->currentItem(), path);

    if (dlg->exec() == QDialog::Rejected) {
        delete dlg;
        return;
    }
    bool copy;
    QString emo = dlg->getEmoticon();
    if (path != dlg->getEmoticon()) {
        copy = true;
    } else {
        copy = false;

        KStandardDirs *dir = KGlobal::dirs();
        emo = dir->findResource("emoticons", themeList->currentItem()->text() + QDir::separator() + f);

        if (emo.isNull())
            emo = dir->findResource("emoticons", themeList->currentItem()->text() + QDir::separator()  + f + ".mng");
        if (emo.isNull())
            emo = dir->findResource("emoticons", themeList->currentItem()->text() + QDir::separator()  + f + ".png");
        if (emo.isNull())
            emo = dir->findResource("emoticons", themeList->currentItem()->text() + QDir::separator()  + f + ".gif");
        if (emo.isNull())
            emo = dir->findResource("emoticons", themeList->currentItem()->text() + QDir::separator()  + f + ".jpg");
        if (emo.isNull())
            emo = dir->findResource("emoticons", themeList->currentItem()->text() + QDir::separator()  + f + ".jpeg");
        if (emo.isNull()) {
            delete dlg;
            return;
        }
    }

    if (theme.removeEmoticon(emoList->currentItem()->text())) {
        delete emoList->currentItem();
    }

    if (theme.addEmoticon(emo, dlg->getText(), copy ? KEmoticonsProvider::Copy : KEmoticonsProvider::DoNotCopy)) {
        new QListWidgetItem(QPixmap(emo), dlg->getText(), emoList);
    }

    emit changed();
    delete dlg;
}

void EmoticonList::newTheme()
{
    QString name = KInputDialog::getText(i18n("New Emoticon Theme"), i18n("Enter the name of the new emoticon theme:"));
    if (name.isEmpty())
        return;
    QString path = KGlobal::dirs()->saveLocation("emoticons", name, false);

    if (KIO::NetAccess::exists(QUrl(path), KIO::NetAccess::SourceSide, this)) {
        KMessageBox::error(this, i18n("%1 theme already exists", name));
    } else {
        QString constraint("(exist Library)");
        KService::List srv = KServiceTypeTrader::self()->query("KEmoticons", constraint);

        QStringList ls;
        int current = 0;

        for (int i = 0; i < srv.size(); ++i) {
            ls << srv.at(i)->name();

            if (srv.at(i)->property("X-KDE-Priority").toInt() > srv.at(current)->property("X-KDE-Priority").toInt()) {
                current = i;
            }
        }

        bool ok;
        QString type = KInputDialog::getItem(i18n("New Emoticon Theme"), i18n("Choose the type of emoticon theme to create"), ls, current, false, &ok, this);

        if (ok && !type.isEmpty()) {
            int index = ls.indexOf(type);
            kEmoticons.newTheme(name, srv.at(index));

            loadTheme(name);
        }
    }
}

void EmoticonList::loadTheme(const QString &name)
{
    if (name.isEmpty())
        return;

    if (emoMap.contains(name)) {
        emoMap.remove(name);
        QList<QListWidgetItem *>ls = themeList->findItems(name, Qt::MatchExactly);
        if (ls.size()) {
            delete ls.at(0);
        }
    }

    KEmoticonsTheme emo = kEmoticons.theme(name);
    if (!emo.isNull()) {
        emoMap[name] = emo;
        QIcon previewIcon = QIcon(previewEmoticon(emo));
        QListWidgetItem *itm = new QListWidgetItem(previewIcon, name, themeList);

        if (name == kEmoticons.currentThemeName()) {
            themeList->setCurrentItem(itm);
        }
    }
}

void EmoticonList::getNewStuff()
{
    KNS3::DownloadDialog dialog("emoticons.knsrc", this);
    dialog.exec();
    if (!dialog.changedEntries().isEmpty()) {
        KNS3::Entry::List entries = dialog.changedEntries();

        for (int i = 0; i < entries.size(); i ++) {
            if (entries.at(i).status() == KNS3::Entry::Installed
                && !entries.at(i).installedFiles().isEmpty()) {
                QString name = entries.at(i).installedFiles().at(0).section('/', -2, -2);
                loadTheme(name);
            } else if (entries.at(i).status() == KNS3::Entry::Deleted) {
                QString name = entries.at(i).uninstalledFiles().at(0).section('/', -2, -2);
                QList<QListWidgetItem*> ls = themeList->findItems(name, Qt::MatchExactly);
                if (ls.size()) {
                    delete ls.at(0);
                    emoMap.remove(name);
                }
            }
        }
    }
}

QString EmoticonList::previewEmoticon(const KEmoticonsTheme &theme)
{
        QString path = theme.tokenize(":)")[0].picPath;
        if (path.isEmpty()) {
            path = theme.emoticonsMap().keys().value(0);
        }
        return path;
}




void EmoticonList::initDefaults()
{
    QList<QListWidgetItem *>ls = themeList->findItems("kde4", Qt::MatchExactly);
    themeList->setCurrentItem( ls.first() );

    cbStrict->setChecked(false);
}


void EmoticonList::defaults()
{
    initDefaults();
    selectTheme();
    emit changed(true);
}

bool EmoticonList::canEditTheme()
{
    if (!themeList->currentItem()) {
        return false;
    }

    KEmoticonsTheme theme = emoMap.value(themeList->currentItem()->text());
    QFileInfo inf(theme.themePath() + '/' + theme.fileName());

    return inf.isWritable();
}

#include "emoticonslist.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
