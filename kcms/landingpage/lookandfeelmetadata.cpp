/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lookandfeelmetadata.h"

#include <KPackage/PackageLoader>

LookAndFeelMetaData::LookAndFeelMetaData(QObject *parent)
    : QObject(parent)
{
}

QString LookAndFeelMetaData::package() const
{
    return m_package;
}

void LookAndFeelMetaData::setPackage(const QString &packageId)
{
    if (m_package == packageId) {
        return;
    }

    auto package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    package.setPath(packageId);

    m_package = packageId;
    setName(package.metadata().name());
    setPreview(package.fileUrl("preview"));

    Q_EMIT packageChanged();
}

QString LookAndFeelMetaData::name() const
{
    return m_name;
}

void LookAndFeelMetaData::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged();
    }
}

QUrl LookAndFeelMetaData::preview() const
{
    return m_preview;
}

void LookAndFeelMetaData::setPreview(const QUrl &url)
{
    if (m_preview != url) {
        m_preview = url;
        Q_EMIT previewChanged();
    }
}

#include "moc_lookandfeelmetadata.cpp"
