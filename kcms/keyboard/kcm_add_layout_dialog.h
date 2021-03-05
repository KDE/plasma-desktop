/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KCM_ADD_LAYOUT_DIALOG_H_
#define KCM_ADD_LAYOUT_DIALOG_H_

#include <QDialog>

#include "keyboard_config.h"

#include <config-keyboard.h>

struct Rules;
class Flags;
class Ui_AddLayoutDialog;

class AddLayoutDialog : public QDialog
{
    Q_OBJECT

public:
    AddLayoutDialog(const Rules *rules, Flags *flags, const QString &model, const QStringList &options, bool showLabel, QWidget *parent = nullptr);

    LayoutUnit getSelectedLayoutUnit()
    {
        return selectedLayoutUnit;
    }
    void accept() override;

public Q_SLOTS:
    void languageChanged(int langIdx);
    void layoutChanged(int layoutIdx);
    void preview();

private:
    const Rules *rules;
    Flags *flags;
    const QString &model;
    const QStringList &options;
    Ui_AddLayoutDialog *layoutDialogUi;
    QString selectedLanguage;
    QString selectedLayout;
    LayoutUnit selectedLayoutUnit;
};

#endif /* KCM_ADD_LAYOUT_DIALOG_H_ */
