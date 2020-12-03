/*
   Copyright (c) 2020 Cyril Rossi <cyril.rossi@enioka.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KRUNNERDATA_H
#define KRUNNERDATA_H

#include <QObject>

#include <KCModuleData>
#include <KSharedConfig>

class KRunnerSettings;

class KRunnerData : public KCModuleData
{
public:
    KRunnerData(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    bool isDefaults() const override;

private:
    KSharedConfigPtr m_krunnerConfig;
    KRunnerSettings *m_settings;
};

#endif // KRUNNERDATA_H
