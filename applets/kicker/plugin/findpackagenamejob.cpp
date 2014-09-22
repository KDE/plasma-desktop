/***************************************************************************
 *   Copyright (C) 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>     *
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

#include "findpackagenamejob.h"

FindPackageJob::FindPackageJob(const QStringList &files, QObject *parent)
: KJob(parent)
, m_files(files)
{
}

void FindPackageJob::start()
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::searchFiles(m_files, PackageKit::Transaction::FilterInstalled);
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)), SLOT(jobDone(PackageKit::Transaction::Exit,uint)));
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)), SLOT(packageFound(PackageKit::Transaction::Info,QString,QString)));
}

QStringList FindPackageJob::packageNames() const
{
    return m_packages;
}

void FindPackageJob::jobDone(PackageKit::Transaction::Exit ret, uint)
{
    if (ret != PackageKit::Transaction::ExitSuccess) {
        setError(ret);
    }

    emitResult();
}

void FindPackageJob::packageFound(PackageKit::Transaction::Info, const QString &packageId, const QString &)
{
    m_packages += PackageKit::Transaction::packageName(packageId);
}
