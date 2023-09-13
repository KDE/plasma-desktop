/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <algorithm>

#include "kcm_add_layout_dialog.h"
#include <KLocalizedString>
#include <QDebug>

#include "flags.h"
#include "iso_codes.h"
#include "tastenbrett.h"
#include "xkb_rules.h"

#include "ui_kcm_add_layout_dialog.h"

AddLayoutDialog::AddLayoutDialog(const Rules *rules_, Flags *flags_, const QString &model_, const QStringList &options_, bool showLabel, QWidget *parent)
    : QDialog(parent)
    , rules(rules_)
    , flags(flags_)
    , model(model_)
    , options(options_)
{
    layoutDialogUi = new Ui_AddLayoutDialog();
    layoutDialogUi->setupUi(this);

    for (const LayoutInfo *layoutInfo : std::as_const(rules->layoutInfos)) {
        QIcon icon;
        if (flags) {
            icon = flags->getIcon(layoutInfo->name);
            if (icon.isNull()) {
                // HACK: QListWidget->iconSize() returns an invalid size, so we can't use that to construct an empty icon
                // instead we pick a large size and QListWidget will scale it down automatically
                QPixmap emptyPixmap(QSize(48, 48));
                emptyPixmap.fill(Qt::transparent);
                icon = QIcon(emptyPixmap); // align text with no icons
            }
        }
        QListWidgetItem *defaultVariantItem = new QListWidgetItem(layoutInfo->description);
        if (flags) {
            defaultVariantItem->setIcon(icon);
        }
        defaultVariantItem->setData(LayoutNameRole, layoutInfo->name);
        defaultVariantItem->setData(VariantNameRole, QStringLiteral(""));
        layoutDialogUi->layoutListWidget->addItem(defaultVariantItem);
        for (const VariantInfo *variantInfo : layoutInfo->variantInfos) {
            QListWidgetItem *newItem = new QListWidgetItem(variantInfo->description);
            if (flags) {
                newItem->setIcon(icon);
            }
            newItem->setData(LayoutNameRole, layoutInfo->name);
            newItem->setData(VariantNameRole, variantInfo->name);
            layoutDialogUi->layoutListWidget->addItem(newItem);
        }
    }

    if (showLabel) {
        layoutDialogUi->labelEdit->setMaxLength(LayoutUnit::MAX_LABEL_LENGTH);
    } else {
        layoutDialogUi->labelLabel->setVisible(false);
        layoutDialogUi->labelEdit->setVisible(false);
    }

    connect(layoutDialogUi->layoutSearchField,
            static_cast<void (QLineEdit::*)(const QString &)>(&QLineEdit::textChanged),
            this,
            &AddLayoutDialog::layoutSearched);
    connect(layoutDialogUi->layoutListWidget,
            static_cast<void (QListWidget::*)(QListWidgetItem *, QListWidgetItem *)>(&QListWidget::currentItemChanged),
            this,
            &AddLayoutDialog::layoutChanged);

    connect(layoutDialogUi->prevbutton, &QPushButton::clicked, this, &AddLayoutDialog::preview);
    layoutDialogUi->prevbutton->setVisible(Tastenbrett::exists());
    layoutDialogUi->prevbutton->setEnabled(false);
    layoutDialogUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void AddLayoutDialog::layoutSearched(const QString &searchText)
{
    QListWidget *list = layoutDialogUi->layoutListWidget;
    for (int row = 0; row < list->count(); ++row) {
        QString itemText = list->item(row)->text();
        bool match = (itemText.contains(searchText, Qt::CaseInsensitive));
        const QStringList words = searchText.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        match = match || std::all_of(words.cbegin(), words.cend(), [&itemText](const QString &word) {
                    return itemText.contains(word, Qt::CaseInsensitive);
                });
        list->item(row)->setHidden(!match);
    }
}

void AddLayoutDialog::layoutChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (current) {
        selectedLayout = current->data(LayoutNameRole).toString();
        selectedLayoutUnit.setLayout(selectedLayout);
        selectedLayoutUnit.setVariant(current->data(VariantNameRole).toString());
    }
    layoutDialogUi->prevbutton->setEnabled((bool)(current));
    layoutDialogUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled((bool)(current));
}

void AddLayoutDialog::accept()
{
    QString label = layoutDialogUi->labelEdit->text();
    if (label == selectedLayout) {
        label = QString();
    }
    selectedLayoutUnit.setDisplayName(label);
    selectedLayoutUnit.setShortcut(layoutDialogUi->kkeysequencewidget->keySequence());
    QDialog::accept();
}

void AddLayoutDialog::preview()
{
    QString variant = layoutDialogUi->layoutListWidget->currentItem()->data(VariantNameRole).toString();
    Tastenbrett::launch(model, selectedLayout, variant, options.join(','));
}
