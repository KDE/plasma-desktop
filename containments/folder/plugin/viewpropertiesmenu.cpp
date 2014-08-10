/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "viewpropertiesmenu.h"

#include <QMenu>

#include <KDirModel>
#include <KLocalizedString>

ViewPropertiesMenu::ViewPropertiesMenu(QObject *parent) : QObject(parent)
{
    m_menu = new QMenu();

    QMenu *menu = m_menu->addMenu(i18n("Arrange In"));
    m_arrangement = new QActionGroup(this);
    connect(m_arrangement, SIGNAL(triggered(QAction*)), this, SIGNAL(arrangementChanged()));
    QAction *action = menu->addAction(i18n("Rows"));
    action->setCheckable(true);
    action->setData(0);
    m_arrangement->addAction(action);
    action = menu->addAction(i18n("Columns"));
    action->setData(1);
    action->setCheckable(true);
    m_arrangement->addAction(action);

    menu = m_menu->addMenu(i18n("Align"));
    m_alignment = new QActionGroup(this);
    connect(m_alignment, SIGNAL(triggered(QAction*)), this, SIGNAL(alignmentChanged()));
    action = menu->addAction(i18n("Left"));
    action->setCheckable(true);
    action->setData(0);
    m_alignment->addAction(action);
    action = menu->addAction(i18n("Right"));
    action->setCheckable(true);
    action->setData(1);
    m_alignment->addAction(action);

    menu = m_menu->addMenu(i18n("Sort By"));
    m_sortMode = new QActionGroup(this);
    connect(m_sortMode, SIGNAL(triggered(QAction*)), this, SIGNAL(sortModeChanged()));
    action = menu->addAction(i18n("Unsorted"));
    action->setCheckable(true);
    action->setData(-1);
    m_sortMode->addAction(action);
    action = menu->addAction(i18n("Name"));
    action->setCheckable(true);
    action->setData(int(KDirModel::Name));
    m_sortMode->addAction(action);
    action = menu->addAction(i18n("Size"));
    action->setCheckable(true);
    action->setData(int(KDirModel::Size));
    m_sortMode->addAction(action);
    action = menu->addAction(i18n("Type"));
    action->setCheckable(true);
    action->setData(int(KDirModel::Type));
    m_sortMode->addAction(action);
    action = menu->addAction(i18n("Date"));
    action->setCheckable(true);
    action->setData(int(KDirModel::ModifiedTime));
    m_sortMode->addAction(action);
    menu->addSeparator();
    m_sortDesc = menu->addAction(i18n("Descending"), this, SIGNAL(sortDescChanged()));
    m_sortDesc->setCheckable(true);
    m_sortDirsFirst = menu->addAction(i18n("Folders First"), this, SIGNAL(sortDirsFirstChanged()));
    m_sortDirsFirst->setCheckable(true);

    m_locked = m_menu->addAction(i18n("Locked"), this, SIGNAL(lockedChanged()));
    m_locked->setCheckable(true);
}

ViewPropertiesMenu::~ViewPropertiesMenu()
{
    delete m_menu;
}

QObject* ViewPropertiesMenu::menu() const
{
    return m_menu;
}

int ViewPropertiesMenu::arrangement() const
{
    return m_arrangement->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setArrangement(int arrangement)
{
    if (!m_arrangement->checkedAction()
        || m_arrangement->checkedAction()->data().toInt() != arrangement) {
        foreach (QAction *action, m_arrangement->actions()) {
            if (action->data().toInt() == arrangement) {
                action->setChecked(true);
                break;
            }
        }
    }
}

int ViewPropertiesMenu::alignment() const
{
    return m_alignment->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setAlignment(int alignment)
{
    if (!m_alignment->checkedAction()
        || m_alignment->checkedAction()->data().toInt() != alignment) {
        foreach (QAction *action, m_alignment->actions()) {
            if (action->data().toInt() == alignment) {
                action->setChecked(true);
                break;
            }
        }
    }
}

bool ViewPropertiesMenu::locked() const
{
    return m_locked->isChecked();
}

void ViewPropertiesMenu::setLocked(bool locked)
{
    if (m_locked->isChecked() != locked) {
        m_locked->setChecked(locked);
    }
}

int ViewPropertiesMenu::sortMode() const
{
    return m_sortMode->checkedAction()->data().toInt();
}

void ViewPropertiesMenu::setSortMode(int sortMode)
{
    if (!m_sortMode->checkedAction()
        || m_sortMode->checkedAction()->data().toInt() != sortMode) {
        foreach (QAction *action, m_sortMode->actions()) {
            if (action->data().toInt() == sortMode) {
                action->setChecked(true);
                break;
            }
        }
    }
}

bool ViewPropertiesMenu::sortDesc() const
{
    return m_sortDesc->isChecked();
}

void ViewPropertiesMenu::setSortDesc(bool sortDesc)
{
    if (m_sortDesc->isChecked() != sortDesc) {
        m_sortDesc->setChecked(sortDesc);
    }
}

bool ViewPropertiesMenu::sortDirsFirst() const
{
    return m_sortDirsFirst->isChecked();
}

void ViewPropertiesMenu::setSortDirsFirst(bool sortDirsFirst)
{
    if (m_sortDirsFirst->isChecked() != sortDirsFirst) {
        m_sortDirsFirst->setChecked(sortDirsFirst);
    }
}
