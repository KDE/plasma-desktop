/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lookandfeelmodel.h"

#include <KPackage/PackageLoader>
#include <KPluginMetaData>

LookAndFeelModel::LookAndFeelModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void LookAndFeelModel::classBegin()
{
}

void LookAndFeelModel::componentComplete()
{
    m_complete = true;
    reload();
}

QHash<int, QByteArray> LookAndFeelModel::roleNames() const
{
    QHash<int, QByteArray> names = QAbstractListModel::roleNames();
    names.insert(Role::PackageId, QByteArrayLiteral("packageId"));
    names.insert(Role::Name, QByteArrayLiteral("name"));
    names.insert(Role::Preview, QByteArrayLiteral("preview"));
    return names;
}

QVariant LookAndFeelModel::data(const QModelIndex &modelIndex, int role) const
{
    const int index = modelIndex.row();
    if (index > m_lnfs.size()) {
        return QVariant();
    }

    const auto &lnf = m_lnfs[index];
    switch (role) {
    case Role::PackageId:
        return lnf.id;
    case Role::Name:
        return lnf.name;
    case Role::Preview:
        return lnf.preview;
    default:
        return QVariant();
    }
}

int LookAndFeelModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_lnfs.size();
}

KLookAndFeel::Variant LookAndFeelModel::variant() const
{
    return m_variant;
}

void LookAndFeelModel::setVariant(KLookAndFeel::Variant variant)
{
    if (m_variant != variant) {
        m_variant = variant;
        if (m_complete) {
            reload();
        }
        Q_EMIT variantChanged();
    }
}

int LookAndFeelModel::indexOf(const QString &packageId) const
{
    for (int i = 0; i < m_lnfs.size(); ++i) {
        if (m_lnfs[i].id == packageId) {
            return i;
        }
    }
    return -1;
}

void LookAndFeelModel::reload()
{
    const QList<KPluginMetaData> packagesMetaData = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/LookAndFeel"));

    QList<LookAndFeelData> lnfs;
    lnfs.reserve(packagesMetaData.count());

    for (const KPluginMetaData &metadata : packagesMetaData) {
        const KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"), metadata.pluginId());

        const auto variant = KLookAndFeel::colorSchemeVariant(pkg);
        if (variant != KLookAndFeel::Variant::Unknown && variant != m_variant) {
            continue;
        }

        lnfs.append({
            .id = metadata.pluginId(),
            .name = metadata.name(),
            .preview = pkg.fileUrl("preview"),
        });
    }

    std::sort(lnfs.begin(), lnfs.end(), [](const auto &a, const auto &b) {
        return a.name < b.name;
    });

    beginResetModel();
    m_lnfs = lnfs;
    endResetModel();
}

#include "moc_lookandfeelmodel.cpp"
