/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "SolidActions.h"
#include "ActionItem.h"

#include <KUrl>
#include <KDialog>
#include <KAboutData>
#include <KMessageBox>
#include <KDesktopFile>
#include <KIO/NetAccess>
#include <KStandardDirs>
#include <KPluginFactory>
#include <KBuildSycocaProgressDialog>

#include <QComboBox>
#include <QPushButton>

#include <Solid/DeviceInterface>
#include <Solid/Predicate>

K_PLUGIN_FACTORY( SolidActionsFactory, registerPlugin<SolidActions>(); )
K_EXPORT_PLUGIN( SolidActionsFactory("kcmsolidactions", "kcm_solid_actions") )

SolidActions::SolidActions(QWidget* parent, const QVariantList&)
        : KCModule(SolidActionsFactory::componentData(), parent)
{
    KAboutData * about = new KAboutData("Device Actions", 0, ki18n("Solid Device Actions Editor"), "1.1",
                                       ki18n("Solid Device Actions Control Panel Module"),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2009 Solid Device Actions team"));
    about->addAuthor(ki18n("Ben Cooksley"), ki18n("Maintainer"), "ben@eclipse.endoftheinternet.org");
    setAboutData(about);
    setButtons(KCModule::Help);

    // Prepare main display dialog
    actionModel = new ActionModel( this );
    mainUi.setupUi( this );
    mainUi.TvActions->setModel( actionModel );
    mainUi.TvActions->setHeaderHidden( true );
    mainUi.TvActions->setRootIsDecorated( false );
    mainUi.TvActions->setSelectionMode( QAbstractItemView::SingleSelection );
    mainUi.PbAddAction->setGuiItem( KStandardGuiItem::add() );
    mainUi.PbEditAction->setIcon( KIcon("document-edit") );

    connect( mainUi.PbAddAction, SIGNAL(clicked()), this, SLOT(slotShowAddDialog()) );
    connect( mainUi.PbEditAction, SIGNAL(clicked()), this, SLOT(editAction()) );
    connect( mainUi.PbDeleteAction, SIGNAL(clicked()), this, SLOT(deleteAction()) );
    connect( mainUi.TvActions->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(toggleEditDelete()) );
    connect( mainUi.TvActions, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editAction()) );

    // Prepare + connect up with Edit dialog
    editUi = new ActionEditor(this);
    connect( editUi, SIGNAL(accepted()), this, SLOT(acceptActionChanges()) );

    // Prepare + connect up add action dialog
    addDialog = new KDialog(this);
    addUi.setupUi( addDialog->mainWidget() );
    addDialog->setInitialSize( QSize(300, 100) ); // Set a sensible default size

    slotTextChanged( addUi.LeActionName->text() );
    connect( addUi.LeActionName, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)) );
    connect( addDialog, SIGNAL(okClicked()), this, SLOT(addAction()) );
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

void SolidActions::slotTextChanged( const QString & text )
{
    addDialog->enableButtonOk( !text.isEmpty() );
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
    QString enteredName = addUi.LeActionName->text();
    KDesktopFile templateDesktop(KStandardDirs::locate("data", "kcmsolidactions/solid-action-template.desktop")); // Lets get the template

    // Lets get a desktop file
    QString internalName = enteredName; // copy the name the user entered -> we will be making mods
    internalName.replace(QChar(' '), QChar('-'), Qt::CaseSensitive); // replace spaces with dashes
    QString filePath = KStandardDirs::locateLocal("data", 0); // Get the location on disk for "data"
    filePath = filePath + "solid/actions/" + internalName + ".desktop"; // Create a file path for new action

    // Fill in an initial template
    KDesktopFile * newDesktop = templateDesktop.copyTo(filePath);
    newDesktop->actionGroup("open").writeEntry("Name", enteredName); // ditto
    delete newDesktop; // Force file to be written

    // Prepare to open the editDialog
    fillActionsList();
    QList<ActionItem*> actionList = actionModel->actionList();
    QModelIndex newAction;
    foreach( ActionItem * newItem, actionList ) { // Lets find our new action
        if( newItem->desktopMasterPath == filePath ) {
            int position = actionList.indexOf( newItem );
            newAction = actionModel->index( position, 0, QModelIndex() ); // Grab it
            break;
        }
    }

    mainUi.TvActions->setCurrentIndex( newAction ); // Set it as currently active
    editAction(); // Open the edit dialog
}

void SolidActions::editAction()
{
    ActionItem * selectedItem = selectedAction();
    if( !selectedItem ) {
        return;
    }

    // We should error out here if we have to
    if( !selectedItem->predicate().isValid() ) {
        KMessageBox::error(this, i18n("It appears that the predicate for this action is not valid."), i18n("Error Parsing Device Conditions"));
        return;
    }

    // Display us!
    editUi->setActionToEdit( selectedItem );
    editUi->setWindowIcon( windowIcon() );
    editUi->show();
}

void SolidActions::deleteAction()
{
    ActionItem * action = selectedAction();
    if( action->isUserSupplied() ) { // Is the action user supplied?
        KIO::NetAccess::del( KUrl(action->desktopMasterPath), this); // Remove the main desktop file then
    }
    KIO::NetAccess::del( KUrl(action->desktopWritePath), this); // Remove the modified desktop file now
    fillActionsList(); // Update the list of actions
}

ActionItem * SolidActions::selectedAction()
{
    QModelIndex action = mainUi.TvActions->currentIndex();
    ActionItem * actionItem = actionModel->data( action, Qt::UserRole ).value<ActionItem*>();
    return actionItem;
}

void SolidActions::fillActionsList()
{
    mainUi.TvActions->selectionModel()->clearSelection();
    actionModel->buildActionList();
    mainUi.TvActions->header()->setResizeMode( 0, QHeaderView::Stretch );
    mainUi.TvActions->header()->setResizeMode( 1, QHeaderView::ResizeToContents );
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

    if( !mainUi.TvActions->currentIndex().isValid() ) { // Is an action selected?
        mainUi.PbDeleteAction->setText( i18n("No Action Selected") ); // Set a friendly disabled text
        mainUi.PbDeleteAction->setIcon( KIcon() );
        toggle = false;
    }

    mainUi.PbEditAction->setEnabled(toggle); // Change them to the new state
    mainUi.PbDeleteAction->setEnabled(toggle); // Ditto

    if( !toggle ) {
        return;
    }

    KUrl writeDesktopFile( selectedAction()->desktopWritePath ); // Get the write desktop file
    // What functionality do we need to change?
    if( selectedAction()->isUserSupplied() ) {
        // We are able to directly delete it, enable full delete functionality
        mainUi.PbDeleteAction->setGuiItem( KStandardGuiItem::remove() );
    } else if( KIO::NetAccess::exists(writeDesktopFile, KIO::NetAccess::SourceSide, this) ) { // Does the write file exist?
        // We are able to revert, lets show it
        mainUi.PbDeleteAction->setGuiItem( KStandardGuiItem::discard() );
    } else {
        // We cannot do anything then, disable delete functionality
        mainUi.PbDeleteAction->setText( i18n("Cannot be deleted") );
        mainUi.PbDeleteAction->setIcon( KIcon() );
        mainUi.PbDeleteAction->setEnabled( false );
    }
}

#include "SolidActions.moc"
