/*
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

#ifndef COLORSCHEMEMODEL_H
#define COLORSCHEMEMODEL_H

#include <PlatformTheme>
#include <QModelIndex>

/**
 * @todo write docs
 */
class ColorSchemeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        CustomRole = Qt::UserRole
    };

public:
    /**
     * @todo write docs
     *
     * @param index TODO
     * @param role TODO
     * @return TODO
     */
    virtual QVariant data(const QModelIndex& index, int role) const = 0;

    /**
     * @todo write docs
     *
     * @param parent TODO
     * @return TODO
     */
    virtual int columnCount(const QModelIndex& parent) const = 0;

    /**
     * @todo write docs
     *
     * @param parent TODO
     * @return TODO
     */
    virtual int rowCount(const QModelIndex& parent) const = 0;

    /**
     * @todo write docs
     *
     * @param child TODO
     * @return TODO
     */
    virtual QModelIndex parent(const QModelIndex& child) const = 0;

    /**
     * @todo write docs
     *
     * @param row TODO
     * @param column TODO
     * @param parent TODO
     * @return TODO
     */
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const = 0;

    /**
     * @todo write docs
     *
     * @param index TODO
     * @param value TODO
     * @param role TODO
     * @return TODO
     */
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

    /**
     * @todo write docs
     *
     * @param section TODO
     * @param orientation TODO
     * @param role TODO
     * @return TODO
     */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /**
     * @todo write docs
     *
     * @param section TODO
     * @param orientation TODO
     * @param value TODO
     * @param role TODO
     * @return TODO
     */
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role);

    /**
     * @todo write docs
     *
     * @param index TODO
     * @return TODO
     */
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

};

#endif // COLORSCHEMEMODEL_H
