/***************************************************************************
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef COMPONENTCHOOSER_H
#define COMPONENTCHOOSER_H

#include <QString>
#include <QVariant>

#include <optional>

#include <KLocalizedString>

class ComponentChooser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList applications MEMBER m_applications NOTIFY applicationsChanged)
    Q_PROPERTY(int index MEMBER m_index NOTIFY indexChanged)
    Q_PROPERTY(bool isDefaults READ isDefaults NOTIFY isDefaultsChanged)

public:
    ComponentChooser(QObject *parent, const QString &mimeType, const QString &type, const QString &defaultApplication, const QString &dialogText);

    void defaults();
    virtual void load();
    bool isDefaults() const;
    bool isSaveNeeded() const;

    Q_INVOKABLE void select(int index);

    virtual void save() = 0;
    void saveMimeTypeAssociation(const QString &mime, const QString &storageId);

    bool isDefault() const;

Q_SIGNALS:
    void applicationsChanged();
    void indexChanged();
    void isDefaultsChanged();

protected:
    QVariantList m_applications;
    int m_index = -1;
    std::optional<int> m_defaultIndex;
    QString m_mimeType;
    QString m_type;
    QString m_defaultApplication;
    QString m_previousApplication;
    QString m_dialogText;
};

#endif
