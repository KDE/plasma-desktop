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

#ifndef FINDPACKAGENAMEJOB_H
#define FINDPACKAGENAMEJOB_H

#include <PackageKit/Daemon>

#include <KJob>

class FindPackageJob : public KJob
{
    Q_OBJECT

    public:
        FindPackageJob(const QStringList &files, QObject *parent = 0);

        virtual void start();

        QStringList packageNames() const;

    private Q_SLOTS:
        void jobDone(PackageKit::Transaction::Exit ret, uint);
        void packageFound(PackageKit::Transaction::Info, const QString &packageId, const QString &);

    private:
        QStringList m_files;
        QStringList m_packages;
};

#endif
