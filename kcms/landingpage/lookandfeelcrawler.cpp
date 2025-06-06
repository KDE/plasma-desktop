/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lookandfeelcrawler.h"

#include <KConfigGroup>
#include <KPackage/PackageLoader>
#include <KPluginMetaData>
#include <KSharedConfig>

#include <QColor>
#include <QDir>

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

LookAndFeelCrawler::Variant LookAndFeelCrawler::variant() const
{
    return m_variant;
}

void LookAndFeelCrawler::setVariant(Variant variant)
{
    if (m_variant != variant) {
        m_variant = variant;
        if (m_complete) {
            reload();
        }
        Q_EMIT variantChanged();
    }
}

static QString colorSchemeFilePath(const QString &schemeName)
{
    QString colorScheme(schemeName);
    colorScheme.remove(QLatin1Char('\'')); // So Foo's does not become FooS
    QRegularExpression fixer(QStringLiteral("[\\W,.-]+(.?)"));
    for (auto match = fixer.match(colorScheme); match.hasMatch(); match = fixer.match(colorScheme)) {
        colorScheme.replace(match.capturedStart(), match.capturedLength(), match.captured(1).toUpper());
    }
    colorScheme.replace(0, 1, colorScheme.at(0).toUpper());

    // NOTE: why this loop trough all the scheme files?
    // the scheme theme name is an heuristic, there is no plugin metadata whatsoever.
    // is based on the file name stripped from weird characters or the
    // eventual id- prefix store.kde.org puts, so we can just find a
    // theme that ends as the specified name
    const QStringList schemeDirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes"), QStandardPaths::LocateDirectory);
    for (const QString &dir : schemeDirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.colors"));
        for (const QString &file : fileNames) {
            if (file.endsWith(colorScheme + QStringLiteral(".colors"))) {
                return dir + QLatin1Char('/') + file;
            }
        }
    }
    return QString();
}

static std::optional<LookAndFeelCrawler::Variant> determineLookAndFeelVariant(const KPackage::Package &package)
{
    QString colorScheme = package.filePath("colors");
    if (colorScheme.isEmpty()) {
        KSharedConfigPtr conf = KSharedConfig::openConfig(package.filePath("defaults"));
        KConfigGroup group(conf, QStringLiteral("kdeglobals"));
        group = KConfigGroup(&group, QStringLiteral("General"));
        const QString schemeName = group.readEntry("ColorScheme", QString());
        if (!schemeName.isEmpty()) {
            colorScheme = colorSchemeFilePath(schemeName);
        }
    }

    if (colorScheme.isEmpty()) {
        return std::nullopt;
    }

    const KSharedConfigPtr colorSchemeConfig = KSharedConfig::openConfig(colorScheme);
    const KConfigGroup group(colorSchemeConfig, QStringLiteral("Colors:Window"));
    if (!group.hasKey(QStringLiteral("BackgroundNormal"))) {
        return std::nullopt;
    }

    const QColor backgroundNormal = group.readEntry(QStringLiteral("BackgroundNormal"), QColor());
    const int backgroundGray = qGray(backgroundNormal.rgb());
    if (backgroundGray < 192) {
        return LookAndFeelCrawler::Variant::Dark;
    } else {
        return LookAndFeelCrawler::Variant::Light;
    }
}

void LookAndFeelCrawler::reload()
{
    const QList<KPluginMetaData> packagesMetaData = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/LookAndFeel"));

    QList<LookAndFeelData> lnfs;
    lnfs.reserve(packagesMetaData.count());

    for (const KPluginMetaData &metadata : packagesMetaData) {
        const KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"), metadata.pluginId());

        const std::optional<Variant> variant = determineLookAndFeelVariant(pkg);
        if (variant && variant != m_variant) {
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
