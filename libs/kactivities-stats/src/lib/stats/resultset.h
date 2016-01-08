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

#ifndef KACTIVITIES_STATS_RESULTSET
#define KACTIVITIES_STATS_RESULTSET

#include "query.h"

#include <QDebug>

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultSetPrivate;
class ResultSet_ResultPrivate;
class ResultSet_IteratorPrivate;

/**
 * Class that can query the KActivities usage tracking mechanism
 * for resources.
 *
 * Note: It is important to note that you should not create a
 * long-living instance of ResultSet. It might lock the database
 * and break proper updating mechanisms.
 */
class KACTIVITIESSTATS_EXPORT ResultSet {
public:
    /**
     * Structure containing data of one of the results
     */
    class Result {
    public:
        Result();
        ~Result();

        Result(Result &&result);
        Result(const Result &result);
        Result &operator=(Result result);

        enum LinkStatus {
            NotLinked = 0,
            Unknown   = 1,
            Linked    = 2
        };

        QString resource() const;      ///< URL of the resource
        QString title() const;         ///< Title of the resource, or URL if title is not known
        QString mimetype() const;      ///< Mimetype of the resource, or URL if title is not known
        double score() const;          ///< The score calculated based on the usage statistics
        uint lastUpdate() const;       ///< Timestamp of the last update
        uint firstUpdate() const;      ///< Timestamp of the first update
        LinkStatus linkStatus() const; ///< Differentiates between linked and non-linked resources in mixed queries

        void setResource(QString resource);
        void setTitle(QString title);
        void setMimetype(QString mimetype);
        void setScore(double score);
        void setLastUpdate(uint lastUpdate);
        void setFirstUpdate(uint firstUpdate);
        void setLinkStatus(LinkStatus linkedStatus);

    private:
        ResultSet_ResultPrivate * d;
    };

    /**
     * ResultSet is a container. This notifies the generic algorithms
     * from STLboost, and others of the contained type.
     */
    typedef Result value_type;

    /**
     * Creates the ResultSet from the specified query
     */
    ResultSet(Query query);

    ResultSet(ResultSet && source);
    ResultSet(const ResultSet &source);
    ResultSet &operator= (ResultSet source);
    ~ResultSet();

    /**
     * @returns a result at the specified index
     * @param index of the result
     * @note You should use iterators instead
     */
    Result at(int index) const;

    // Iterators

    /**
     * An STL-style constant forward iterator for accessing the results in a ResultSet
     * TODO: Consider making this to be more than just forward iterator.
     *       Maybe even a random-access one.
     */
    class const_iterator {
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef int difference_type;

        typedef const Result value_type;
        typedef const Result& reference;
        typedef const Result* pointer;

        const_iterator();
        const_iterator(const const_iterator &source);
        const_iterator &operator=(const const_iterator &source);

        ~const_iterator();

        bool isSourceValid() const;

        reference operator*() const;
        pointer   operator->() const;

        // prefix
        const_iterator& operator++();
        // postfix
        const_iterator operator++(int);

        // prefix
        const_iterator& operator--();
        // postfix
        const_iterator operator--(int);

        const_iterator operator+(difference_type n) const;
        const_iterator& operator+=(difference_type n);

        const_iterator operator-(difference_type n) const;
        const_iterator& operator-=(difference_type n);

        reference operator[](difference_type n) const;

        friend bool operator==(const const_iterator &left, const const_iterator &right);
        friend bool operator!=(const const_iterator &left, const const_iterator &right);

        friend bool operator<(const const_iterator &left, const const_iterator &right);
        friend bool operator>(const const_iterator &left, const const_iterator &right);

        friend bool operator<=(const const_iterator &left, const const_iterator &right);
        friend bool operator>=(const const_iterator &left, const const_iterator &right);

        friend difference_type operator-(const const_iterator &left, const const_iterator &right);

    private:
        const_iterator(const ResultSet *resultSet, int currentRow);

        friend class ResultSet;

        ResultSet_IteratorPrivate* const d;
    };

    /**
     * @returns a constant iterator pointing to the start of the collection
     * (to the first item)
     * @note as usual in C++, the range of the collection is [begin, end)
     */
    const_iterator begin() const;
    /**
     * @returns a constant iterator pointing to the end of the collection
     * (after the last item)
     * @note as usual in C++, the range of the collection is [begin, end)
     */
    const_iterator end() const;

    /**
     * Alias for begin
     */
    inline const_iterator cbegin() const { return begin(); }
    /**
     * Alias for end
     */
    inline const_iterator cend() const   { return end(); }

    /**
     * Alias for begin
     */
    inline const_iterator constBegin() const { return cbegin(); }
    /**
     * Alias for end
     */
    inline const_iterator constEnd() const   { return cend(); }

private:
    friend class ResultSet_IteratorPrivate;
    ResultSetPrivate *d;
};

bool KACTIVITIESSTATS_EXPORT operator==(const ResultSet::const_iterator &left,
                                        const ResultSet::const_iterator &right);
bool KACTIVITIESSTATS_EXPORT operator!=(const ResultSet::const_iterator &left,
                                        const ResultSet::const_iterator &right);

bool KACTIVITIESSTATS_EXPORT operator<(const ResultSet::const_iterator &left,
                                       const ResultSet::const_iterator &right);
bool KACTIVITIESSTATS_EXPORT operator>(const ResultSet::const_iterator &left,
                                       const ResultSet::const_iterator &right);

bool KACTIVITIESSTATS_EXPORT operator<=(const ResultSet::const_iterator &left,
                                        const ResultSet::const_iterator &right);
bool KACTIVITIESSTATS_EXPORT operator>=(const ResultSet::const_iterator &left,
                                        const ResultSet::const_iterator &right);

ResultSet::const_iterator::difference_type KACTIVITIESSTATS_EXPORT
operator-(const ResultSet::const_iterator &left,
          const ResultSet::const_iterator &right);

inline QDebug operator<< (QDebug out, const ResultSet::Result &result)
{
    return out
        << (result.linkStatus() == ResultSet::Result::Linked ? "⊤" :
            result.linkStatus() == ResultSet::Result::NotLinked ? "⊥" : "?")
        << result.score()
        << result.title()
        << result.resource()
        ;
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTSET

