/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SolidActions.h"
#include "ActionItem.h"

#include <KAboutData>
#include <KBuildSycocaProgressDialog>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KMessageBox>
#include <KPluginFactory>
#include <KStandardGuiItem>

#include <KIO/Global>

#include <QComboBox>
#include <QDebug>
#include <QPushButton>

#include <Solid/DeviceInterface>
#include <Solid/Predicate>

K_PLUGIN_FACTORY(SolidActionsFactory, registerPlugin<SolidActions>();)

SolidActions::SolidActions(QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
    KAboutData *about = new KAboutData(QStringLiteral("Device Actions"),
                                       i18n("Solid Device Actions Editor"),
                                       QStringLiteral("1.2"),
                                       i18n("Solid Device Actions Control Panel Module"),
                                       KAboutLicense::GPL,
                                       i18n("(c) 2009, 2014 Solid Device Actions team"));
    about->addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("ben@eclipse.endoftheinternet.org"));
    about->addCredit(QStringLiteral("Lukáš Tinkl"), i18n("Port to Plasma 5"), QStringLiteral("ltinkl@redhat.com"));
    setAboutData(about);
    setButtons(KCModule::Help);

    // Prepare main display dialog
    actionModel = new ActionModel(this);
    mainUi.setupUi(this);
    mainUi.TvActions->setModel(actionModel);
    mainUi.TvActions->setHeaderHidden(true);
    mainUi.TvActions->setRootIsDecorated(false);
    mainUi.TvActions->setSelectionMode(QAbstractItemView::SingleSelection);
    KStandardGuiItem::assign(mainUi.PbAddAction, KStandardGuiItem::Add);
    mainUi.PbEditAction->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));

    connect(mainUi.PbAddAction, &QPushButton::clicked, this, &SolidActions::slotShowAddDialog);
    connect(mainUi.PbEditAction, &QPushButton::clicked, this, &SolidActions::editAction);
    connect(mainUi.PbDeleteAction, &QPushButton::clicked, this, &SolidActions::deleteAction);
    connect(mainUi.TvActions->selectionModel(), &QItemSelectionModel::currentChanged, this, &SolidActions::toggleEditDelete);
    connect(mainUi.TvActions, &QTreeView::doubleClicked, this, &SolidActions::editAction);

    // Prepare + connect up with Edit dialog
    editUi = new ActionEditor(this);
    connect(editUi, &ActionEditor::accepted, this, &SolidActions::acceptActionChanges);

    // Prepare + connect up add action dialog
    addDialog = new QDialog(this);
    addUi.setupUi(addDialog);
    addDialog->resize(QSize(300, 100)); // Set a sensible default size

    slotTextChanged(addUi.LeActionName->text());
    connect(addUi.LeActionName, &QLineEdit::textChanged, this, &SolidActions::slotTextChanged);
    connect(addUi.buttonBox, &QDialogButtonBox::accepted, this, &SolidActions::addAction);
    connect(addUi.buttonBox, &QDialogButtonBox::rejected, addDialog, &QDialog::reject);
}

SolidActions::~SolidActions()
{
    delete editUi;
    delete actionModel;
}

void SolidActions::slotShowAddDialog()
{
    addDialog->show();
    addUi.LeActionName->setFocus();
    addUi.LeActionName->clear();
}

void SolidActions::slotTextChanged(const QString &text)
{
    addUi.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void SolidActions::load()
{
    fillActionsList();
}

void SolidActions::defaults()
{
}

void SolidActions::save()
{
}

void SolidActions::addAction()
{
    const QString enteredName = addUi.LeActionName->text();
    KDesktopFile templateDesktop(QStandardPaths::GenericDataLocation, QStringLiteral("kcmsolidactions/solid-action-template.desktop")); // Lets get the template

    // Lets get a desktop file
    QString internalName = enteredName; // copy the name the user entered -> we will be making mods
    internalName.replace(QChar(' '), QChar('-'), Qt::CaseSensitive); // replace spaces with dashes
    internalName = KIO::encodeFileName(internalName);

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/solid/actions/"; // Get the location on disk for "data"
    if (!QDir().exists(filePath)) {
        QDir().mkpath(filePath);
    }
    filePath += internalName + ".desktop";

    // Fill in an initial template
    KDesktopFile *newDesktop = templateDesktop.copyTo(filePath);
    newDesktop->actionGroup(QStringLiteral("open")).writeEntry("Name", enteredName); // ditto
    delete newDesktop; // Force file to be written

    // Prepare to open the editDialog
    fillActionsList();
    const QList<ActionItem *> actionList = actionModel->actionList();
    QModelIndex newAction;
    for (ActionItem *newItem : actionList) { // Lets find our new action
        if (newItem->desktopMasterPath == filePath) {
            const int position = actionList.indexOf(newItem);
            newAction = actionModel->index(position, 0); // Grab it
            break;
        }
    }

    mainUi.TvActions->setCurrentIndex(newAction); // Set it as currently active
    addDialog->hide();
    editAction(); // Open the edit dialog
}

void SolidActions::editAction()
{
    ActionItem *selectedItem = selectedAction();
    if (!selectedItem) {
        return;
    }

    // We should error out here if we have to
    if (!selectedItem->predicate().isValid()) {
        KMessageBox::error(this, i18n("It appears that the predicate for this action is not valid."), i18n("Error Parsing Device Conditions"));
        return;
    }

    // Display us!
    editUi->setActionToEdit(selectedItem);
    editUi->setWindowIcon(windowIcon());
    editUi->show();
}

void SolidActions::deleteAction()
{
    ActionItem *action = selectedAction();
    if (action->isUserSupplied()) { // Is the action user supplied?
        QFile::remove(action->desktopMasterPath); // Remove the main desktop file then
    }
    QFile::remove(action->desktopWritePath); // Remove the modified desktop file now
    fillActionsList(); // Update the list of actions
}

ActionItem *SolidActions::selectedAction() const
{
    QModelIndex action = mainUi.TvActions->currentIndex();
    ActionItem *actionItem = actionModel->data(action, Qt::UserRole).value<ActionItem *>();
    return actionItem;
}

void SolidActions::fillActionsList()
{
    mainUi.TvActions->clearSelection();
    actionModel->buildActionList();
    mainUi.TvActions->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mainUi.TvActions->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    toggleEditDelete();
}

void SolidActions::acceptActionChanges()
{
    // Re-read the actions list to ensure changes are reflected
    KBuildSycocaProgressDialog::rebuildKSycoca(this);
    fillActionsList();
}

void SolidActions::toggleEditDelete()
{
    bool toggle = true;

    if (!mainUi.TvActions->currentIndex().isValid()) { // Is an action selected?
        mainUi.PbDeleteAction->setText(i18n("No Action Selected")); // Set a friendly disabled text
        mainUi.PbDeleteAction->setIcon(QIcon());
        toggle = false;
    }

    mainUi.PbEditAction->setEnabled(toggle); // Change them to the new state
    mainUi.PbDeleteAction->setEnabled(toggle); // Ditto

    if (!toggle) {
        return;
    }

    // What functionality do we need to change?
    if (selectedAction()->isUserSupplied()) {
        // We are able to directly delete it, enable full delete functionality
        KStandardGuiItem::assign(mainUi.PbDeleteAction, KStandardGuiItem::Remove);
    } else if (QFile::exists(selectedAction()->desktopWritePath)) { // Does the write file exist?
        // We are able to revert, lets show it
        KStandardGuiItem::assign(mainUi.PbDeleteAction, KStandardGuiItem::Discard);
    } else {
        // We cannot do anything then, disable delete functionality
        mainUi.PbDeleteAction->setText(i18n("Cannot be deleted"));
        mainUi.PbDeleteAction->setIcon(QIcon());
        mainUi.PbDeleteAction->setEnabled(false);
    }
}

#include "SolidActions.moc"
