/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    QString m_category;
    QString m_defaultApplication;
    QString m_previousApplication;
    QString m_dialogText;
};

#endif
