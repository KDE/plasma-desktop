/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "query.h"
#include <QDebug>

namespace KActivities {
namespace Experimental {
namespace Stats {

namespace details {
    inline void validateTypes(QStringList &types)
    {
        // Nothing at the moment
    }

    inline void validateAgents(QStringList &agents)
    {
        // Nothing at the moment
    }

    inline void validateActivities(QStringList &activities)
    {
        // Nothing at the moment
    }

    inline void validateUrlFilters(QStringList &urlFilters)
    {
        auto i = urlFilters.begin();
        const auto end = urlFilters.end();

        for (; i != end ; ++i) {
            i->replace("'", "");
        }
    }

} // namespace details

class Query::Private {
public:
    Terms::Select   selection;
    QStringList     types;
    QStringList     agents;
    QStringList     activities;
    QStringList     urlFilters;
    Terms::Order    ordering;
};

Query::Query(Terms::Select selection)
    : d(new Private())
{
    d->selection = selection;
}

Query::Query(Query &&source)
    : d(nullptr)
{
    std::swap(d, source.d);
}

Query::Query(const Query &source)
    : d(new Private(*source.d))
{
}

Query &Query::operator= (Query source)
{
    std::swap(d, source.d);
    return *this;
}


Query::~Query()
{
    delete d;
}

bool Query::operator== (const Query &right) const
{
    return selection()  == right.selection() &&
           types()      == right.types() &&
           agents()     == right.agents() &&
           activities() == right.activities() &&
           selection()  == right.selection() &&
           urlFilters() == right.urlFilters();
}

bool Query::operator!= (const Query &right) const
{
    return !(*this == right);
}

#define IMPLEMENT_QUERY_LIST_FIELD(WHAT, What, Default)                        \
    void Query::add##WHAT(const QStringList &What)                             \
    {                                                                          \
        d->What << What;                                                       \
        details::validate##WHAT(d->What);                                      \
    }                                                                          \
                                                                               \
    QStringList Query::What() const                                            \
    {                                                                          \
        return d->What.size() ? d->What : Default;                             \
    }                                                                          \
                                                                               \
    void Query::clear##WHAT()                                                  \
    {                                                                          \
        d->What.clear();                                                       \
    }

IMPLEMENT_QUERY_LIST_FIELD(Types,      types,      QStringList(":any"))
IMPLEMENT_QUERY_LIST_FIELD(Agents,     agents,     QStringList(":current"))
IMPLEMENT_QUERY_LIST_FIELD(Activities, activities, QStringList(":current"))
IMPLEMENT_QUERY_LIST_FIELD(UrlFilters, urlFilters, QStringList("*"))

#undef IMPLEMENT_QUERY_LIST_FIELD

void Query::setOrdering(Terms::Order ordering)
{
    d->ordering = ordering;
}

void Query::setSelection(Terms::Select selection)
{
    d->selection = selection;
}

Terms::Order Query::ordering() const
{
    return d->ordering;
}

Terms::Select Query::selection() const
{
    return d->selection;
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

namespace KAStats = KActivities::Experimental::Stats;

QDebug operator<<(QDebug dbg, const KAStats::Query &query)
{
    using namespace KAStats::Terms;

    dbg.nospace()
        << "Query { "
        << query.selection()
        << ", " << Type(query.types())
        << ", " << Agent(query.agents())
        << ", " << Activity(query.activities())
        << ", " << Url(query.urlFilters())
        << ", " << query.ordering()
        << " }";
    return dbg;
}
