/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDialog>
#include <QListWidgetItem>

#include "keyboard_config.h"

#include <config-keyboard.h>

struct Rules;
class Flags;
class Ui_AddLayoutDialog;

enum LayoutDataRoles { LayoutNameRole = Qt::UserRole, VariantNameRole };

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
    void layoutChanged(QListWidgetItem *, QListWidgetItem *);
    void layoutSearched(const QString &);
    void preview();

private:
    const Rules *rules;
    Flags *flags;
    const QString &model;
    const QStringList &options;
    Ui_AddLayoutDialog *layoutDialogUi;
    QString selectedLayout;
    LayoutUnit selectedLayoutUnit;
};
