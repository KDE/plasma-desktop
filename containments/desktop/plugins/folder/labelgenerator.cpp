/*
    SPDX-FileCopyrightText: 2008, 2009 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "labelgenerator.h"

#include <KFilePlacesModel>
#include <KShell>

#include "foldermodel.h"

int LabelGenerator::s_instanceCount = 0;
KFilePlacesModel *LabelGenerator::s_placesModel = nullptr;

LabelGenerator::LabelGenerator(QObject *parent)
    : QObject(parent)
    , m_rtl(false)
    , m_labelMode(1)
{
    ++s_instanceCount;
}

LabelGenerator::~LabelGenerator()
{
    --s_instanceCount;
    if (!s_instanceCount) {
        delete s_placesModel;
        s_placesModel = nullptr;
    }
}

FolderModel *LabelGenerator::folderModel() const
{
    return m_folderModel.data();
}

void LabelGenerator::setFolderModel(FolderModel *folderModel)
{
    if (m_folderModel.data() != folderModel) {
        if (m_folderModel.data()) {
            disconnect(m_folderModel.data(), nullptr, this, nullptr);
        }

        m_folderModel = folderModel;

        connect(m_folderModel.data(), &FolderModel::listingCompleted, this, &LabelGenerator::updateDisplayLabel);
        connect(m_folderModel.data(), &FolderModel::listingCanceled, this, &LabelGenerator::updateDisplayLabel);

        Q_EMIT folderModelChanged();
        updateDisplayLabel();
    }
}

bool LabelGenerator::rtl() const
{
    return m_rtl;
}

void LabelGenerator::setRtl(bool rtl)
{
    if (rtl != m_rtl) {
        m_rtl = rtl;
        Q_EMIT rtlChanged();
        updateDisplayLabel();
    }
}

int LabelGenerator::labelMode() const
{
    return m_labelMode;
}

void LabelGenerator::setLabelMode(int mode)
{
    if (mode != m_labelMode) {
        m_labelMode = mode;
        Q_EMIT labelModeChanged();
        updateDisplayLabel();
    }
}

QString LabelGenerator::labelText() const
{
    return m_labelText;
}

void LabelGenerator::setLabelText(const QString &text)
{
    if (text != m_labelText) {
        m_labelText = text;
        Q_EMIT labelTextChanged();
        updateDisplayLabel();
    }
}

QString LabelGenerator::displayLabel() const
{
    return m_displayLabel;
}

void LabelGenerator::updateDisplayLabel()
{
    const QString displayLabel = generatedDisplayLabel();
    if (m_displayLabel != displayLabel) {
        m_displayLabel = displayLabel;
        Q_EMIT displayLabelChanged();
    }
}

QString LabelGenerator::generatedDisplayLabel()
{
    if (!m_folderModel) {
        return QString();
    }

    QUrl url = m_folderModel->resolvedUrl();

    if (m_labelMode == 1 /* Default */) {
        if (url.path().length() <= 1) {
            const KFileItem &rootItem = m_folderModel->rootItem();

            if (rootItem.text() != QLatin1String(".")) {
                return rootItem.text();
            }
        }

        QString label(url.toDisplayString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash));

        if (!s_placesModel) {
            s_placesModel = new KFilePlacesModel();
        }

        connectPlacesModel();

        const QModelIndex index = s_placesModel->closestItem(url);

        if (index.isValid()) {
            QString root = s_placesModel->url(index).toDisplayString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);

            label = label.right(label.length() - root.length());

            if (!label.isEmpty()) {
                if (label.at(0) == QLatin1Char('/')) {
                    label.remove(0, 1);
                }

                if (m_rtl) {
                    label.prepend(QLatin1String(" < "));
                } else {
                    label.prepend(QLatin1String(" > "));
                }
            }

            label.prepend(s_placesModel->text(index));
        }

        return label;
    } else if (m_labelMode == 2 /* Full path */) {
        return QUrl(url).toDisplayString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
    } else if (m_labelMode == 3 /* Custom title */) {
        return m_labelText;
    }

    return QString();
}

void LabelGenerator::connectPlacesModel()
{
    connect(s_placesModel, &KFilePlacesModel::rowsInserted, this, &LabelGenerator::updateDisplayLabel, Qt::UniqueConnection);
    connect(s_placesModel, &KFilePlacesModel::rowsRemoved, this, &LabelGenerator::updateDisplayLabel, Qt::UniqueConnection);
    connect(s_placesModel, &KFilePlacesModel::dataChanged, this, &LabelGenerator::updateDisplayLabel, Qt::UniqueConnection);
}
