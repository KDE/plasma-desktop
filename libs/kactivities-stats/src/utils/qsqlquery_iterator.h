/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UTILS_QSQLQUERYITERATOR_H
#define UTILS_QSQLQUERYITERATOR_H

#include <QSqlQuery>
#include <QVariant>

template <typename ResultSet>
class NextValueIterator {
public:
    enum Type {
        NormalIterator,
        EndIterator
    };

    NextValueIterator(ResultSet &query, Type type = NormalIterator)
        : m_query(query)
        , m_type(type)
    {
        if (type != EndIterator) {
            m_query.next();
        }
    }

    inline bool operator!= (const NextValueIterator<ResultSet> &other) const
    {
        Q_UNUSED(other);
        return m_query.isValid();
    }

    inline NextValueIterator<ResultSet> &operator*()
    {
        return *this;
    }

    inline QVariant operator[] (int index) const
    {
        return m_query.value(index);
    }

    inline QVariant operator[] (const QString &name) const
    {
        return m_query.value(name);
    }

    inline NextValueIterator<ResultSet> &operator ++()
    {
        m_query.next();
        return *this;
    }

private:
    ResultSet &m_query;
    Type m_type;

};

NextValueIterator<QSqlQuery> begin(QSqlQuery &query);
NextValueIterator<QSqlQuery> end(QSqlQuery &query);


#endif /* UTILS_QSQLQUERYITERATOR_H */

