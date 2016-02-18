/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef KACTIVITIES_IMPORTS_UTILS_P_H
#define KACTIVITIES_IMPORTS_UTILS_P_H

// -----------------------------------------
// RAII classes for model updates ----------
// -----------------------------------------

#define DECLARE_RAII_MODEL_UPDATERS(Class)                                     \
    template <typename T> class _model_reset {                                 \
        T *model;                                                              \
                                                                               \
    public:                                                                    \
        _model_reset(T *m) : model(m)                                          \
        {                                                                      \
            model->beginResetModel();                                          \
        }                                                                      \
        ~_model_reset()                                                        \
        {                                                                      \
            model->endResetModel();                                            \
        }                                                                      \
    };                                                                         \
    template <typename T> class _model_insert {                                \
        T *model;                                                              \
                                                                               \
    public:                                                                    \
        _model_insert(T *m, const QModelIndex &parent, int first, int last)    \
            : model(m)                                                         \
        {                                                                      \
            model->beginInsertRows(parent, first, last);                       \
        }                                                                      \
        ~_model_insert()                                                       \
        {                                                                      \
            model->endInsertRows();                                            \
        }                                                                      \
    };                                                                         \
    template <typename T> class _model_remove {                                \
        T *model;                                                              \
                                                                               \
    public:                                                                    \
        _model_remove(T *m, const QModelIndex &parent, int first, int last)    \
            : model(m)                                                         \
        {                                                                      \
            model->beginRemoveRows(parent, first, last);                       \
        }                                                                      \
        ~_model_remove()                                                       \
        {                                                                      \
            model->endRemoveRows();                                            \
        }                                                                      \
    };                                                                         \
    typedef _model_reset<Class> model_reset;                                   \
    typedef _model_remove<Class> model_remove;                                 \
    typedef _model_insert<Class> model_insert;

// -----------------------------------------

#endif // KACTIVITIES_IMPORTS_UTILS_P_H

