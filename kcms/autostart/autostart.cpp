/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@gmail.com                                                     *
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
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

#include "autostart.h"
#include "autostartitem.h"
#include "addscriptdialog.h"
#include "advanceddialog.h"
#include "autostartmodel.h"

#include <QDir>
#include <QTreeWidget>
#include <QStringList>

#include <KConfig>
#include <KConfigGroup>
#include <KShell>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KDesktopFile>
#include <KMessageBox>
#include <KAboutData>
#include <KLocalizedString>
#include <KIO/DeleteJob>
#include <KIO/CopyJob>
#include <KShell>
#include <QDebug>

K_PLUGIN_FACTORY(AutostartFactory, registerPlugin<Autostart>();)

Autostart::Autostart( QWidget* parent, const QVariantList& )
    : KCModule(parent )
    , m_model(new AutostartModel(this))
{
    widget = new Ui_AutostartConfig();
    widget->setupUi(this);

    QStringList lstHeader;
    lstHeader << i18n( "Name" )
              << i18n( "Command" )
              << i18n( "Status" )
              << i18nc("@title:column The name of the column that decides if the program is run on session startup, on session shutdown, etc", "Run On" );
    widget->listCMD->setHeaderLabels(lstHeader);
    widget->listCMD->setFocus();

    setButtons(Help);

    connect( widget->btnProperties, SIGNAL(clicked()), SLOT(slotEditCMD()) );
    connect( widget->listCMD, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotEditCMD(QTreeWidgetItem*)) );

    connect(widget->btnAddScript, &QPushButton::clicked, this, &Autostart::slotAddScript);
    connect(widget->btnAddProgram, &QPushButton::clicked, this, &Autostart::slotAddProgram);
    connect(widget->btnRemove, &QPushButton::clicked, this, &Autostart::slotRemoveCMD);
    connect(widget->btnAdvanced, &QPushButton::clicked, this, &Autostart::slotAdvanced);
    connect(widget->listCMD, &QTreeWidget::itemClicked, this, &Autostart::slotItemClicked);
    connect(widget->listCMD, &QTreeWidget::itemSelectionChanged, this, &Autostart::slotSelectionChanged);

    connect(m_model, &QAbstractItemModel::rowsInserted, this, &Autostart::slotRowInserted);
    connect(m_model, &QAbstractItemModel::dataChanged, this, &Autostart::slotDatachanged);

    KAboutData* about = new KAboutData(QStringLiteral("Autostart"),
                                       i18n("Session Autostart Manager"),
                                       QStringLiteral("1.0"),
                                       i18n("Session Autostart Manager Control Panel Module"),
                                       KAboutLicense::GPL,
                                       i18n("Copyright © 2006–2010 Autostart Manager team"));
    about->addAuthor(i18n("Stephen Leaf"), QString(), QStringLiteral("smileaf@gmail.com"));
    about->addAuthor(i18n("Montel Laurent"), i18n( "Maintainer" ), QStringLiteral("montel@kde.org"));
    setAboutData( about );

}

Autostart::~Autostart()
{
   delete widget;
}


void Autostart::slotItemClicked( QTreeWidgetItem *item, int col)
{
    if ( item && col == COL_STATUS ) {
        DesktopStartItem *entry = dynamic_cast<DesktopStartItem*>( item );
        if (entry) {
            const bool enabled = ( item->checkState( col ) == Qt::Checked );
            m_model->setData(indexFromWidget(item), enabled, AutostartModel::Roles::Enabled);
            if (enabled) {
                item->setText( COL_STATUS, i18nc( "The program will be run", "Enabled" ) );
            } else {
                item->setText( COL_STATUS, i18nc( "The program won't be run", "Disabled" ) );
            }
        }
    }
}

void Autostart::updateDesktopStartItem(DesktopStartItem *item, const QString &name, const QString &command, bool disabled, const QString &fileName)
{
    Q_ASSERT( item );
    item->setText( COL_NAME, name );
    item->setToolTip(COL_NAME, KShell::tildeCollapse(fileName));
    item->setText( COL_RUN, AutostartModel::listPathName()[0] /* Startup */ );
    item->setText( COL_COMMAND, command );
    item->setCheckState( COL_STATUS, disabled ? Qt::Unchecked : Qt::Checked );
    item->setText( COL_STATUS, disabled ? i18nc( "The program won't be run", "Disabled" ) : i18nc( "The program will be run", "Enabled" ));
}

void Autostart::updateScriptStartItem(ScriptStartItem *item, const QString &name, const QString &command, AutostartEntrySource type, const QString &fileName)
{
    Q_ASSERT( item );
    item->setText( COL_NAME, name );
    item->setToolTip(COL_NAME, KShell::tildeCollapse(fileName));
    item->setText( COL_COMMAND, command );
    item->changeStartup( type );
}

void Autostart::load()
{
    widget->listCMD->clear();

    m_programItem = new QTreeWidgetItem( widget->listCMD );
    m_programItem->setText( 0, i18n( "Desktop File" ));
    m_programItem->setFlags(m_programItem->flags()^Qt::ItemIsSelectable );

    QFont boldFont =  m_programItem->font(0);
    boldFont.setBold( true );
    m_programItem->setData ( 0, Qt::FontRole, boldFont );

    m_scriptItem = new QTreeWidgetItem( widget->listCMD );
    m_scriptItem->setText( 0, i18n( "Script File" ));
    m_scriptItem->setFlags(m_scriptItem->flags()^Qt::ItemIsSelectable);
    m_scriptItem->setData ( 0, Qt::FontRole, boldFont);

    widget->listCMD->expandItem( m_programItem );
    widget->listCMD->expandItem( m_scriptItem );

    m_model->load();

    for (int i = 0; i < m_model->rowCount(); i++) {
        slotRowInserted(QModelIndex(), i, i);
    }

    //Update button
    slotSelectionChanged();
    widget->listCMD->resizeColumnToContents(COL_NAME);
    widget->listCMD->resizeColumnToContents(COL_STATUS);
    widget->listCMD->resizeColumnToContents(COL_RUN);
}

void Autostart::slotAddProgram()
{
    KOpenWithDialog *owdlg = new KOpenWithDialog(this);
    connect(owdlg, &QDialog::finished, this, [this, owdlg] (int result) {
        if (result == QDialog::Accepted) {

            KService::Ptr service = owdlg->service();

            Q_ASSERT(service);
            if (!service) {
                return; // Don't crash if KOpenWith wasn't able to create service.
            }

            m_model->addEntry(service);
        }
    });
    owdlg->open();
}

void Autostart::slotAddScript()
{
    AddScriptDialog *addDialog = new AddScriptDialog(this);
    connect(addDialog, &QDialog::finished, this, [this, addDialog] (int result) {
        if (result == QDialog::Accepted) {
            m_model->addEntry(addDialog->importUrl(), addDialog->symLink());
        }
    });
    addDialog->open();
}

void Autostart::slotRemoveCMD()
{
    QTreeWidgetItem *widgetItem = widget->listCMD->currentItem();
    if (!widgetItem) {
        return;
    }

    if (m_model->removeEntry(indexFromWidget(widgetItem))) {
        if (m_scriptItem->indexOfChild(widgetItem) != -1) {
            m_scriptItem->removeChild(widgetItem);
        } else {
            m_programItem->removeChild(widgetItem);
        }
        delete widgetItem;
    }
}

void Autostart::slotRowInserted(const QModelIndex &parent, int first, int last)
{
    Q_ASSERT(!parent.isValid());

    for (int i = first; i <= last; i++) {
        QModelIndex idx = m_model->index(i, 0);

        const QString &name = m_model->data(idx, Qt::DisplayRole).toString();
        const QString &fileName = m_model->data(idx, AutostartModel::Roles::FileName).toString();
        const AutostartEntrySource source = AutostartModel::sourceFromInt(m_model->data(idx, AutostartModel::Roles::Source).toInt());
        const QString &command = KShell::tildeCollapse(m_model->data(idx, AutostartModel::Roles::Command).toString());

        if (source == AutostartEntrySource::XdgAutoStart) {
            const bool enabled = m_model->data(idx, AutostartModel::Roles::Enabled).toBool();

            DesktopStartItem *item = new DesktopStartItem(m_programItem);
            updateDesktopStartItem(item, name, command, !enabled, fileName);
        } else {
            ScriptStartItem *item = new ScriptStartItem(m_scriptItem, this);
            updateScriptStartItem(item, name, command, source, fileName);
        }
    }
}

void Autostart::slotDatachanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(roles)

    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {

        QModelIndex idx = m_model->index(row);

        const QString &name = m_model->data(idx, Qt::DisplayRole).toString();
        const QString &fileName = m_model->data(idx, AutostartModel::Roles::FileName).toString();
        const AutostartEntrySource source = AutostartModel::sourceFromInt(m_model->data(idx, AutostartModel::Roles::Source).toInt());
        const QString &command = KShell::tildeCollapse(m_model->data(idx, AutostartModel::Roles::Command).toString());

        if (row > (m_programItem->childCount() -1)) {
            // scriptItem
            QTreeWidgetItem *item = m_scriptItem->child(row - m_programItem->childCount());
            ScriptStartItem *scriptEntry = dynamic_cast<ScriptStartItem*>(item);
            updateScriptStartItem(scriptEntry, name, command, source, fileName);
        } else {
            // desktopItem
            const bool enabled = m_model->data(idx, AutostartModel::Roles::Enabled).toBool();

            QTreeWidgetItem *item = m_programItem->child(row);
            DesktopStartItem *desktopItem = dynamic_cast<DesktopStartItem*>(item);
            updateDesktopStartItem(desktopItem, name, command, !enabled, fileName);
        }
    }
}

void Autostart::slotEditCMD(QTreeWidgetItem* ent)
{
    if (!ent) return;
    DesktopStartItem *desktopItem = dynamic_cast<DesktopStartItem*>( ent );
    if (desktopItem) {
        const QModelIndex index = indexFromWidget(ent);
        const QString fileName = m_model->data(index, AutostartModel::Roles::FileName).toString();
        KFileItem kfi(QUrl::fromLocalFile(fileName));
        kfi.setDelayedMimeTypes(true);

        KPropertiesDialog *dlg = new KPropertiesDialog(kfi, this);
        connect(dlg, &QDialog::finished, this, [this, index, fileName, desktopItem, dlg] (int result) {
            if (result == QDialog::Accepted) {

                // Entry may have change of file
                m_model->reloadEntry(index, dlg->item().localPath());

                const QString name = m_model->data(index, Qt::DisplayRole).toString();
                const QString command = m_model->data(index, AutostartModel::Roles::Command).toString();
                const bool enabled = m_model->data(index, AutostartModel::Roles::Enabled).toBool();

                updateDesktopStartItem( desktopItem, name, command, !enabled, fileName);
            }
        });
        dlg->open();
    }
}

void Autostart::slotEditCMD()
{
    if (widget->listCMD->currentItem() == nullptr) {
        return;
    }
    slotEditCMD(dynamic_cast<AutoStartItem*>(widget->listCMD->currentItem()));
}

void Autostart::slotAdvanced()
{
    if (widget->listCMD->currentItem() == nullptr) {
        return;
    }

    const QModelIndex index = indexFromWidget(widget->listCMD->currentItem());
    const bool onlyInPlasma = m_model->data(index, AutostartModel::Roles::OnlyInPlasma).toBool();

    AdvancedDialog *dlg = new AdvancedDialog(this, onlyInPlasma);
    connect(dlg, &QDialog::finished, this, [this, index, dlg] (int result) {
        if (result == QDialog::Accepted) {
            const bool dialogOnlyInKde = dlg->onlyInKde();
            m_model->setData(index, dialogOnlyInKde, AutostartModel::Roles::OnlyInPlasma);
        };
    });
    dlg->open();
}

QModelIndex Autostart::indexFromWidget(QTreeWidgetItem *widget) const
{
    int index = m_programItem->indexOfChild(widget);
    if (index != -1) {
        // widget is part of m_programItem children
        return m_model->index(index);
    } else {
        // widget is part of m_scriptItem children
        return m_model->index(m_programItem->childCount() + m_scriptItem->indexOfChild(widget));
    }
}

void Autostart::slotChangeStartup( ScriptStartItem *item, int comboData )
{
    Q_ASSERT(item);

    const QModelIndex index = indexFromWidget(item);

    if (!m_model->setData(index, comboData, AutostartModel::Roles::Source)) {
        // the action was cancelled restore the previously selected source
        item->changeStartup(AutostartModel::sourceFromInt(m_model->data(index, AutostartModel::Roles::Source).toInt()));
    }
}

void Autostart::slotSelectionChanged()
{
    const bool hasItems = ( dynamic_cast<AutoStartItem*>( widget->listCMD->currentItem() )!=nullptr ) ;
    widget->btnRemove->setEnabled(hasItems);

    const bool isDesktopItem = (dynamic_cast<DesktopStartItem*>(widget->listCMD->currentItem() ) != nullptr) ;
    widget->btnProperties->setEnabled(isDesktopItem);
    widget->btnAdvanced->setEnabled(isDesktopItem) ;
}

void Autostart::defaults()
{
}

void Autostart::save()
{
}

#include "autostart.moc"
