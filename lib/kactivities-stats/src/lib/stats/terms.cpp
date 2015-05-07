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

#include "terms.h"
#include <QDebug>

namespace KActivities {
namespace Experimental {
namespace Stats {

// Term classes
#define IMPLEMENT_TERM_CONSTRUCTORS(TYPE)                     \
    Terms::TYPE::TYPE(QStringList values)                     \
        : values(values)                                      \
    {}                                                        \
                                                              \
    Terms::TYPE::TYPE(QString value)                          \
        : values(QStringList() << value)                      \
    {}

#define IMPLEMENT_SPECIAL_TERM_VALUE(TYPE, VALUE_NAME, VALUE) \
    Terms::TYPE Terms::TYPE::VALUE_NAME()                     \
    {                                                         \
        return Terms::TYPE(VALUE);                            \
    }

IMPLEMENT_TERM_CONSTRUCTORS(Type)
IMPLEMENT_SPECIAL_TERM_VALUE(Type, any, ":any")

IMPLEMENT_TERM_CONSTRUCTORS(Agent)
IMPLEMENT_SPECIAL_TERM_VALUE(Agent, any, ":any")
IMPLEMENT_SPECIAL_TERM_VALUE(Agent, global, ":global")
IMPLEMENT_SPECIAL_TERM_VALUE(Agent, current, ":current")

IMPLEMENT_TERM_CONSTRUCTORS(Activity)
IMPLEMENT_SPECIAL_TERM_VALUE(Activity, any, ":any")
IMPLEMENT_SPECIAL_TERM_VALUE(Activity, global, ":global")
IMPLEMENT_SPECIAL_TERM_VALUE(Activity, current, ":current")

IMPLEMENT_TERM_CONSTRUCTORS(Url)
IMPLEMENT_SPECIAL_TERM_VALUE(Url, localFile, "/*")
IMPLEMENT_SPECIAL_TERM_VALUE(Url, file, QStringList() << "/*" << "smb:*" << "fish:*" << "sftp:*" << "ftp:*")

#undef IMPLEMENT_TERM_CONSTRUCTORS
#undef IMPLEMENT_SPECIAL_TERM_VALUE

Terms::Limit::Limit(int value)
    : value(value)
{
}

Terms::Limit Terms::Limit::all()
{
    return Limit(0);
}

Terms::Offset::Offset(int value)
    : value(value)
{
}

Terms::Url Terms::Url::startsWith(const QString &prefix)
{
    return Url(prefix + "*");
}

Terms::Url Terms::Url::contains(const QString &infix)
{
    return Url("*" + infix + "*");
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

namespace KAStats = KActivities::Experimental::Stats;

#define QDEBUG_TERM_OUT(TYPE, OUT)                               \
    QDebug operator<<(QDebug dbg, const KAStats::Terms::TYPE &_) \
    {                                                            \
        using namespace KAStats::Terms;                          \
        dbg.nospace() << #TYPE << ": " << (OUT);                 \
        return dbg;                                              \
    }

QDEBUG_TERM_OUT(Order,    _ == HighScoredFirst      ? "HighScore"       :
                          _ == RecentlyUsedFirst    ? "RecentlyUsed"    :
                          _ == RecentlyCreatedFirst ? "RecentlyCreated" :
                                                      "Alphabetical"    )

QDEBUG_TERM_OUT(Select,   _ == LinkedResources ? "LinkedResources" :
                          _ == UsedResources   ? "UsedResources"   :
                                                 "AllResources"    )

QDEBUG_TERM_OUT(Type,     _.values)
QDEBUG_TERM_OUT(Agent,    _.values)
QDEBUG_TERM_OUT(Activity, _.values)
QDEBUG_TERM_OUT(Url,      _.values)

QDEBUG_TERM_OUT(Limit,    _.value)
QDEBUG_TERM_OUT(Offset,   _.value)

#undef QDEBUG_TERM_OUT

