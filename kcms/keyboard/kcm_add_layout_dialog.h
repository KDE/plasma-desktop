/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
