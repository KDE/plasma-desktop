/*
 * Copyright 2014 Ivan Cukic <ivan.cukic@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_DATABASE_H
#define COMMON_DATABASE_H

#include <utils/d_ptr.h>
#include <memory>
#include <QSqlQuery>

namespace Common {

class Database {
public:
    typedef std::shared_ptr<Database> Ptr;

    enum Source {
        ResourcesDatabase
    };

    enum OpenMode {
        ReadWrite,
        ReadOnly
    };

    static Ptr instance(Source source, OpenMode openMode);

    QSqlQuery execQueries(const QStringList &queries) const;
    QSqlQuery execQuery(const QString &query, bool ignoreErrors = false) const;
    QSqlQuery createQuery() const;

    void setPragma(const QString &pragma);
    QVariant pragma(const QString &pragma) const;
    QVariant value(const QString &query) const;

    // For debugging purposes only
    QString lastQuery() const;

    ~Database();
    Database();

    friend class Locker;
    class Locker {
    public:
        Locker(Database &database);
        ~Locker();

    private:
        QSqlDatabase &m_database;
    };

    #define DATABASE_TRANSACTION(A) \
        /* enable this for debugging only: qDebug() << "Location:" << __FILE__ << __LINE__; */ \
        Common::Database::Locker lock(A)

private:
    D_PTR;
};

} // namespace Common

#endif // COMMON_DATABASE_H

