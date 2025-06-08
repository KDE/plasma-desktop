/**
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "tabletmoduledata.h"

#include "tabletsmodel.h"

TabletModuleData::TabletModuleData(QObject *parent)
    : KCModuleData(parent)
{
    m_db = libwacom_database_new();
    if (m_db) {
        m_tabletsModel = new TabletsModel(m_db, this);
        connect(m_tabletsModel, &QAbstractItemModel::rowsInserted, this, &TabletModuleData::updateRelevance);
        connect(m_tabletsModel, &QAbstractItemModel::rowsRemoved, this, &TabletModuleData::updateRelevance);
        connect(m_tabletsModel, &QAbstractItemModel::modelReset, this, &TabletModuleData::updateRelevance);
    }
    updateRelevance();
}

TabletModuleData::~TabletModuleData()
{
    if (m_db) {
        libwacom_database_destroy(m_db);
    }
}

void TabletModuleData::updateRelevance()
{
    bool relevant = false;
    if (m_tabletsModel) {
        relevant = m_tabletsModel->rowCount(QModelIndex()) > 0;
    }
    setRelevant(relevant);
}

#include "moc_tabletmoduledata.cpp"
