/***************************************************************************
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

#include "autostartitem.h"
#include "autostart.h"

#include <QComboBox>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QDir>
#include <QDebug>

#include <KLocalizedString>
#include <KIO/CopyJob>

AutoStartItem::AutoStartItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem( parent )
{
}

AutoStartItem::~AutoStartItem()
{

}

DesktopStartItem::DesktopStartItem(QTreeWidgetItem *parent )
    : AutoStartItem(parent )
{
    setCheckState( Autostart::COL_STATUS, Qt::Checked );
}

DesktopStartItem::~DesktopStartItem()
{
}

ScriptStartItem::ScriptStartItem( QTreeWidgetItem *parent, Autostart* autostart )
    : AutoStartItem( parent )
{
    m_comboBoxStartup = new QComboBox;
    // make the folder PathName match the AutoStartPaths that match AutostartEntrySource
    // skipping XdgAutoStart that is not available for script items
    m_comboBoxStartup->addItem(AutostartModel::listPathName()[0], AutostartEntrySource::XdgScripts);
    m_comboBoxStartup->addItem(AutostartModel::listPathName()[1], AutostartEntrySource::PlasmaShutdown);
    m_comboBoxStartup->addItem(AutostartModel::listPathName()[2], AutostartEntrySource::PlasmaStart);

    setText(Autostart::COL_STATUS, i18nc( "The program will be run", "Enabled" ) );
    connect(m_comboBoxStartup, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &ScriptStartItem::slotStartupChanged);
    connect( this, &ScriptStartItem::askChangeStartup, autostart, &Autostart::slotChangeStartup );
    treeWidget()->setItemWidget( this, Autostart::COL_RUN, m_comboBoxStartup );
}

ScriptStartItem::~ScriptStartItem()
{
}

void ScriptStartItem::slotStartupChanged(int index)
{
    Q_UNUSED(index)
    emit askChangeStartup(this, m_comboBoxStartup->currentData().toInt());
}

void ScriptStartItem::changeStartup(AutostartEntrySource type )
{
    switch( type )
    {
    case AutostartEntrySource::XdgScripts:
        m_comboBoxStartup->setCurrentIndex( 0 );
        break;
    case AutostartEntrySource::PlasmaShutdown:
        m_comboBoxStartup->setCurrentIndex( 1 );
        break;
    case AutostartEntrySource::PlasmaStart:
        m_comboBoxStartup->setCurrentIndex( 2 );
        break;
    default:
        qWarning() << " startup type is not defined :" << type;
        break;
    }
}
