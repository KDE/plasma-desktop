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

#ifndef EMOTICONSLIST_H
#define EMOTICONSLIST_H

#include <QStringList>
#include <KLineEdit>
#include <KDialog>

#include <kcmodule.h>
#include <kemoticons.h>
#include "ui_emoticonslist.h"

class EditDialog : public KDialog
{
    Q_OBJECT

public:
    EditDialog(QWidget *parent, const QString &name);
    EditDialog(QWidget *parent, const QString &name, QListWidgetItem *itm, const QString &file);
    QString getText() const { return leText->text(); }
    QString getEmoticon() const { return emoticon; }
private slots:
    void btnIconClicked();
    void updateOkButton();
private:
    void setupDlg();
    QWidget *wdg;
    KLineEdit *leText;
    QPushButton *btnIcon;
    QString emoticon;
};

class EmoticonList : public KCModule, Ui::EmoticonsManager
{
    Q_OBJECT

public:
    EmoticonList(QWidget *parent, const QVariantList &args);
    ~EmoticonList();
    void load();
    void save();
    virtual void defaults();
private slots:
    void installEmoticonTheme();
    void btRemoveThemeClicked();
    void btRemoveEmoticonClicked();
    void selectTheme();
    void addEmoticon();
    void editEmoticon();
    void newTheme();
    void getNewStuff();
    void updateButton();
    void somethingChanged();
private:
    void loadTheme(const QString &name);
    void initDefaults();
    bool canEditTheme();
    QString previewEmoticon(const KEmoticonsTheme &theme);
    QHash<QString, KEmoticonsTheme> emoMap;
    QStringList delFiles;
    KEmoticons kEmoticons;
};

#endif /* EMOTICONSLIST_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
