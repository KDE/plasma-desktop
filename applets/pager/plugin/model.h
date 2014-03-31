/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MODEL_H
#define MODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QRectF>
#include <QtGui/QWidgetList> // For WId
#include <QtGui/QPixmap>

class RectangleModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RectangleRoles {
        WidthRole = Qt::UserRole + 1,
        HeightRole,
        XRole,
        YRole
    };

    RectangleModel(QObject *parent = 0);

    virtual QHash<int, QByteArray> roles() const;
    virtual void clear();
    void append(const QRectF &rect);
    QRectF &rectAt(int index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QList<QRectF> m_rects;
};


class WindowModel : public RectangleModel
{
    Q_OBJECT
public:
    enum WindowRole {
        IdRole = RectangleModel::YRole + 1,
        ActiveRole,
        IconRole,
        VisibleNameRole
    };

    WindowModel(QObject *parent = 0);

    QHash<int, QByteArray> roles() const;
    void clear();
    void append(WId, const QRectF &, bool active, const QPixmap &icon, const QString &name);
    WId idAt(int index) const;
    QString visibleNameAt(int index) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QList<WId> m_ids;
    QList<bool> m_active;
    QList<QPixmap> m_icons;
    QStringList m_visibleNames;
};


class PagerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum PagerRoles {
        WindowsRole = RectangleModel::YRole + 1,
        DesktopNameRole
    };

    PagerModel(QObject *parent = 0);

    QHash<int, QByteArray> roles() const;

    void clearDesktopRects();
    void appendDesktopRect(const QRectF &rect, const QString &name);
    QRectF &desktopRectAt(int index);

    void clearWindowRects();
    void appendWindowRect(int desktopId, WId, const QRectF &, bool active,
                          const QPixmap &icon, const QString &name);
    WindowModel *windowsAt(int index) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

private:
    RectangleModel m_desktops;
    QList<QObject *> m_windows;
    QStringList m_names;
};

#endif // MODEL_H
