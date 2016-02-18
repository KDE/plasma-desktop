/*
 *   Copyright (C) 2016 by Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KACTIVITIES_STATS_QFLATSET_H
#define KACTIVITIES_STATS_QFLATSET_H

#include <QVector>
#include <QPair>

namespace KActivities {

template <typename T, typename Comparator>
class QFlatSet: public QVector<T> {
public:
    inline
    QPair<typename QVector<T>::iterator, bool> insert(const T &value)
    {
        auto comparator = Comparator();
        auto begin      = this->begin();
        auto end        = this->end();

        // We want small sets, so a binary search
        // will be slower than a serial search
        auto iterator = std::find_if(begin, end,
            [&] (const T &current) {
                return comparator(value, current);
            });

        if (iterator != end) {
            if (comparator(*iterator, value)) {
                // Already present
                return { iterator, false };
            }
        }

        QVector<T>::insert(iterator, value);

        return { iterator, true };
    }
};


} // namespace KActivities

#endif // KACTIVITIES_STATS_QFLATSET_H

