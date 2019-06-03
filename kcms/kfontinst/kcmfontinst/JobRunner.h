/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef __JOB_RUNNER_H__
#define __JOB_RUNNER_H__

#include <QUrl>
#include <QDialog>
#include "FontInstInterface.h"

class QLabel;
class QProgressBar;
class QStackedWidget;
class QCloseEvent;
class QCheckBox;
class QDialogButtonBox;
class QAbstractButton;
class QTemporaryDir;

namespace KFI
{

class CActionLabel;

class CJobRunner : public QDialog
{
    Q_OBJECT

    public:

    struct Item : public QUrl
    {
        enum EType
        {
            TYPE1_FONT,
            TYPE1_AFM,
            TYPE1_PFM,
            OTHER_FONT
        };

        Item(const QUrl &u=QUrl(), const QString &n=QString(), bool dis=false);
        Item(const QString &file, const QString &family, quint32 style, bool system);
        QString displayName() const { return name.isEmpty() ? url() : name; }
        QString name,
                fileName;  // Only required so that we can sort an ItemList so that afm/pfms follow after pfa/pfbs
        EType   type;
        bool    isDisabled;

        bool operator<(const Item &o) const;
    };

    typedef QList<Item> ItemList;

    enum ECommand
    {
        CMD_INSTALL,
        CMD_DELETE,
        CMD_ENABLE,
        CMD_DISABLE,
        CMD_UPDATE,
        CMD_MOVE,
        CMD_REMOVE_FILE
    };

    explicit CJobRunner(QWidget *parent, int xid=0);
    ~CJobRunner() override;

    static FontInstInterface * dbus();
    static QString             folderName(bool sys);
    static void startDbusService();

    static QUrl encode(const QString &family, quint32 style, bool system);

    static void     getAssociatedUrls(const QUrl &url, QList<QUrl> &list, bool afmAndPfm, QWidget *widget);
    int             exec(ECommand cmd, const ItemList &urls, bool destIsSystem);

    Q_SIGNALS:

    void configuring();

    private Q_SLOTS:

    void doNext();
    void checkInterface();
    void dbusServiceOwnerChanged(const QString &name, const QString &from, const QString &to);
    void dbusStatus(int pid, int status);
    void slotButtonClicked(QAbstractButton *button);

    private:

    void    contineuToNext(bool cont);
    void    closeEvent(QCloseEvent *e) override;
    void    setPage(int page, const QString &msg=QString());
    QString fileName(const QUrl &url);
    QString errorString(int value) const;

    private:

    ECommand                itsCmd;
    ItemList                itsUrls;
    ItemList::ConstIterator itsIt,
                            itsEnd,
                            itsPrev;
    bool                    itsDestIsSystem;
    QLabel                  *itsStatusLabel,
                            *itsSkipLabel,
                            *itsErrorLabel;
    QProgressBar            *itsProgress;
    bool                    itsAutoSkip,
                            itsCancelClicked,
                            itsModified;
    QTemporaryDir           *itsTempDir;
    QString                 itsCurrentFile;
    CActionLabel            *itsActionLabel;
    QStackedWidget          *itsStack;
    int                     itsLastDBusStatus;
    QCheckBox               *itsDontShowFinishedMsg;
    QDialogButtonBox        *itsButtonBox;
    QPushButton             *itsSkipButton;
    QPushButton             *itsAutoSkipButton;
};

}

#endif
