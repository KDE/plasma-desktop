/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KLocalizedString>
#include <QString>
#include <QVariant>

#include "applicationmodel.h"

class ApplicationModel;

class PairQml
{
    Q_GADGET

    Q_PROPERTY(QVariant first MEMBER first CONSTANT FINAL)
    Q_PROPERTY(QVariant second MEMBER second CONSTANT FINAL)

public:
    PairQml() = default;
    PairQml(QVariant f, QVariant s)
        : first(f)
        , second(s)
    {
    }

    QVariant first;
    QVariant second;
};

class ComponentChooser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ApplicationModel *applications MEMBER m_model CONSTANT)
    Q_PROPERTY(int index MEMBER m_index NOTIFY indexChanged)
    Q_PROPERTY(bool isDefaults READ isDefaults NOTIFY isDefaultsChanged)

    Q_PROPERTY(QStringList unsupportedMimeTypes READ unsupportedMimeTypes NOTIFY modelChanged)
    Q_PROPERTY(QList<PairQml> mimeTypesNotAssociated READ mimeTypesNotAssociated NOTIFY modelChanged)
    Q_PROPERTY(bool isSaveNeeded READ isSaveNeeded NOTIFY indexChanged)

public:
    ComponentChooser(QObject *parent, const QString &mimeType, const QString &type, const QString &defaultApplication, const QString &dialogText);

    virtual void load();
    virtual void save();

    void defaults();
    bool isDefaults() const;
    bool isSaveNeeded() const;

    Q_INVOKABLE QString currentStorageId() const;
    Q_INVOKABLE QString applicationName() const;
    Q_INVOKABLE QString applicationIcon() const;

    /// return the mimeTypes not supported by the currently selected application
    QStringList unsupportedMimeTypes() const;

    /// Mimetypes associated with another application than the current
    QList<PairQml> mimeTypesNotAssociated() const;

    Q_INVOKABLE void select(int index);
    Q_INVOKABLE void saveAssociationUnsuportedMimeTypes();
    Q_INVOKABLE void saveMimeTypesNotAssociated();

    void saveMimeTypeAssociations(const QString &storageId, const QStringList &mimes, bool forceUnsupportedMimeType = false);

    virtual QStringList mimeTypes() const;

    static void forceReloadServiceCache();
    static bool serviceSupportsMimeType(KService::Ptr service, const QString &mimeType);

Q_SIGNALS:
    void indexChanged();
    void isDefaultsChanged();
    void isSaveNeededChanged();
    void modelChanged();

public slots:
    void onSaved();

protected:
    ApplicationModel *m_model;

    int m_index = -1;
    QString m_mimeType;
    QString m_applicationCategory;
    QString m_defaultApplication;
    QString m_currentApplication;
    QString m_dialogText;

private:
    void reBuildCacheAndLoad();
};
