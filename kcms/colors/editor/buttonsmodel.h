/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
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
#ifndef KDECOARTIONS_PREVIEW_BUTTONS_MODEL_H
#define KDECOARTIONS_PREVIEW_BUTTONS_MODEL_H

#include <KDecoration2/DecorationButton>
#include <QAbstractListModel>

namespace KDecoration2
{

namespace Preview
{
class PreviewBridge;

class ButtonsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ButtonsModel(const QVector< DecorationButtonType > &buttons, QObject *parent = nullptr);
    explicit ButtonsModel(QObject *parent = nullptr);
    ~ButtonsModel() override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash< int, QByteArray > roleNames() const override;

    QVector< DecorationButtonType > buttons() const {
        return m_buttons;
    }

    Q_INVOKABLE void clear();
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void up(int index);
    Q_INVOKABLE void down(int index);
    Q_INVOKABLE void move(int sourceIndex, int targetIndex);

    void replace(const QVector< DecorationButtonType > &buttons);
    void add(DecorationButtonType type);
    Q_INVOKABLE void add(int index, int type);

private:
    QVector< DecorationButtonType > m_buttons;
};

}
}

#endif

