/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ActionEditor.h"

#include <KMessageBox>

#include <Solid/Predicate>

ActionEditor::ActionEditor(QWidget *parent)
    : QDialog(parent)
{
    topItem = new PredicateItem(Solid::Predicate(), nullptr);
    rootItem = nullptr;
    rootModel = new PredicateModel(topItem, this);
    // Prepare the dialog
    resize(QSize(600, 600)); // Set a decent initial size
    // setModal( true );
    // Set up the interface
    ui.setupUi(this);
    ui.TvPredicateTree->setHeaderHidden(true);
    ui.TvPredicateTree->setModel(rootModel);
    ui.IbActionIcon->setIconSize(KIconLoader::SizeLarge);

    ui.CbDeviceType->addItems(actionData()->interfaceList());

    // Connect up with everything needed -> slot names explain
    connect(ui.TvPredicateTree, &QTreeView::activated, this, &ActionEditor::updateParameter);
    connect(ui.PbParameterSave, &QPushButton::clicked, this, &ActionEditor::saveParameter);
    connect(ui.PbParameterReset, &QPushButton::clicked, this, &ActionEditor::updateParameter);
    connect(ui.CbDeviceType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ActionEditor::updatePropertyList);
    connect(ui.CbParameterType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ActionEditor::manageControlStatus);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &ActionEditor::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &ActionEditor::reject);

    if (ui.TvPredicateTree->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick)) {
        connect(ui.TvPredicateTree, &QTreeView::clicked, this, &ActionEditor::updateParameter);
    }
}

ActionEditor::~ActionEditor()
{
    delete topItem;
}

void ActionEditor::setActionToEdit(ActionItem *item)
{
    activeItem = item;

    // Set all the text appropriately
    ui.IbActionIcon->setIcon(item->icon());
    ui.LeActionFriendlyName->setText(item->name());
    ui.LeActionCommand->setText(item->exec());

    setPredicate(item->predicate());
    setWindowTitle(i18n("Editing Action '%1'", item->name())); // Set a friendly i18n caption
}

void ActionEditor::setPredicate(Solid::Predicate predicate)
{
    delete topItem;
    topItem = new PredicateItem(Solid::Predicate(), nullptr);
    rootItem = new PredicateItem(predicate, topItem);
    rootModel->setRootPredicate(rootItem->parent());

    // Select the top item, not the hidden root
    QModelIndex topItem = rootModel->index(0, 0, QModelIndex());
    ui.TvPredicateTree->setCurrentIndex(topItem);
    ui.TvPredicateTree->expandToDepth(2);
    updateParameter();
}

void ActionEditor::updateParameter()
{
    QModelIndex current = ui.TvPredicateTree->currentIndex();
    PredicateItem *currentItem = static_cast<PredicateItem *>(current.internalPointer());

    ui.CbParameterType->setCurrentIndex(currentItem->itemType);
    updatePropertyList();
    ui.CbDeviceType->setCurrentIndex(actionData()->interfacePosition(currentItem->ifaceType));
    int valuePos = actionData()->propertyPosition(currentItem->ifaceType, currentItem->property);
    ui.CbValueName->setCurrentIndex(valuePos);
    ui.LeValueMatch->setText(currentItem->value.toString());
    ui.CbValueMatch->setCurrentIndex(currentItem->compOperator);
}

void ActionEditor::saveParameter()
{
    QModelIndex current = ui.TvPredicateTree->currentIndex();
    PredicateItem *currentItem = static_cast<PredicateItem *>(current.internalPointer());

    // Hold onto this so we can determine if the number of children has changed...
    Solid::Predicate::Type oldType = currentItem->itemType;

    currentItem->setTypeByInt(ui.CbParameterType->currentIndex());
    currentItem->ifaceType = actionData()->interfaceFromName(ui.CbDeviceType->currentText());
    currentItem->property = actionData()->propertyInternal(currentItem->ifaceType, ui.CbValueName->currentText());
    currentItem->value = QVariant(ui.LeValueMatch->text());
    currentItem->setComparisonByInt(ui.CbValueMatch->currentIndex());

    rootModel->itemUpdated(current);
    rootModel->childrenChanging(current, oldType);
}

QString ActionEditor::predicateString()
{
    return rootItem->predicate().toString();
}

void ActionEditor::updatePropertyList()
{
    Solid::DeviceInterface::Type currentType;
    currentType = actionData()->interfaceFromName(ui.CbDeviceType->currentText());

    ui.CbValueName->clear();
    ui.CbValueName->addItems(actionData()->propertyList(currentType));
}

void ActionEditor::manageControlStatus()
{
    bool atomEnable = false;
    bool isEnable = false;

    switch (ui.CbParameterType->currentIndex()) {
    case Solid::Predicate::PropertyCheck:
        atomEnable = true;
    case Solid::Predicate::InterfaceCheck:
        isEnable = true;
        break;
    default:
        break;
    }
    ui.CbDeviceType->setEnabled(isEnable);
    ui.CbValueName->setEnabled(atomEnable);
    ui.CbValueMatch->setEnabled(atomEnable);
    ui.LeValueMatch->setEnabled(atomEnable);
}

SolidActionData *ActionEditor::actionData()
{
    return SolidActionData::instance();
}

void ActionEditor::accept()
{
    // Save any open parameter changes first...
    saveParameter();

    // Read the data and prepare to save
    QString iconName = ui.IbActionIcon->icon();
    QString actionName = ui.LeActionFriendlyName->text();
    QString command = ui.LeActionCommand->text();
    QString predicate = predicateString();

    // We need to ensure that they are all valid before applying
    if (iconName.isEmpty() || actionName.isEmpty() || command.isEmpty() || !Solid::Predicate::fromString(predicate).isValid()) {
        KMessageBox::error(this,
                           i18n("It appears that the action name, command, icon or condition are not valid.\nTherefore, changes will not be applied."),
                           i18n("Invalid action"));
        return;
    }
    // apply the changes
    if (iconName != activeItem->icon()) { // Has the icon changed?
        activeItem->setIcon(ui.IbActionIcon->icon()); // Write the change
    }
    if (actionName != activeItem->name()) { // Has the friendly name changed?
        activeItem->setName(ui.LeActionFriendlyName->text()); // Write the change
    }
    if (command != activeItem->exec()) { // Has the command changed?
        activeItem->setExec(ui.LeActionCommand->text()); // Write the change
    }
    if (predicate != activeItem->predicate().toString()) { // Has it changed?
        activeItem->setPredicate(predicate); // Write the change
    }

    QDialog::accept();
}
