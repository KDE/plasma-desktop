/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lookandfeelcrawler.h"

#include <KPackage/PackageLoader>
#include <KPluginMetaData>

LookAndFeelCrawler::LookAndFeelCrawler(QObject *parent)
    : QAbstractListModel(parent)
{
}

void LookAndFeelCrawler::classBegin()
{
}

void LookAndFeelCrawler::componentComplete()
{
    m_complete = true;
    reload();
}

QHash<int, QByteArray> LookAndFeelCrawler::roleNames() const
{
    QHash<int, QByteArray> names = QAbstractListModel::roleNames();
    names.insert(Role::PackageId, QByteArrayLiteral("packageId"));
    names.insert(Role::Name, QByteArrayLiteral("name"));
    names.insert(Role::Preview, QByteArrayLiteral("preview"));
    return names;
}

QVariant LookAndFeelCrawler::data(const QModelIndex &modelIndex, int role) const
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

int LookAndFeelCrawler::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_lnfs.size();
}

KLookAndFeel::Variant LookAndFeelCrawler::variant() const
{
    return m_variant;
}

void LookAndFeelCrawler::setVariant(KLookAndFeel::Variant variant)
{
    if (m_variant != variant) {
        m_variant = variant;
        if (m_complete) {
            reload();
        }
        Q_EMIT variantChanged();
    }
}

void LookAndFeelCrawler::reload()
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

    beginResetModel();
    m_lnfs = lnfs;
    endResetModel();
}

#include "moc_lookandfeelcrawler.cpp"
