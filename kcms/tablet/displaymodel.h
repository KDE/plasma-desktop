/*
    SPDX-FileCopyrightText: 2023 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QStandardItemModel>
#include <ddcutil_c_api.h>
#include <memory>

class DDCDisplay
{
public:
    DDCA_Display_Ref ref;
    QString manufacturer;
    QString name;
};

class DDCA_Display_Ref_Wrapper
{
public:
    DDCA_Display_Ref ref;
};

Q_DECLARE_METATYPE(DDCA_Display_Ref_Wrapper)

class DisplayModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool requirementsSatisfied READ requirementsStatisfied CONSTANT)
    Q_PROPERTY(QString requirementsDescription READ requirementsDescription CONSTANT)
public:
    enum Roles { Name = Qt::UserRole + 1, Manufacturer, DDCRef };
    DisplayModel(QObject *parent = nullptr);
    DisplayModel(const DisplayModel &) = delete;
    DisplayModel(DisplayModel &&) = delete;
    DisplayModel &operator=(const DisplayModel &) = delete;
    DisplayModel &operator=(DisplayModel &&) = delete;
    ~DisplayModel() = default;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool requirementsSatisfied() const;
    QString requirementsDescription() const;

    Q_INVOKABLE QVariant displayAt(int index);

private:
    enum class Requirements { Group, Module, Success };
    Requirements m_requirement = Requirements::Success;
    std::vector<std::unique_ptr<DDCDisplay>> m_displays;
};

class ColorSpaceModel : public QStandardItemModel
{
    Q_OBJECT
public:
    ColorSpaceModel();
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE int indexFromColorspaceValue(int value) const;
};
