/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KQuickConfigModule>

class ActivitiesModule : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(bool isNewActivityAuthorized READ isNewActivityAuthorized CONSTANT)

public:
    ActivitiesModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~ActivitiesModule() override;

    bool isNewActivityAuthorized() const;

    Q_INVOKABLE void configureActivity(const QString &id);
    Q_INVOKABLE void newActivity();
    Q_INVOKABLE void deleteActivity(const QString &id);

    void load() override;
    void handleArgument(const QString &argument);

private:
    bool m_isNewActivityAuthorized;
    QString m_firstArgument;
};
