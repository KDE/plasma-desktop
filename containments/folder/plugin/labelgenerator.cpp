/***************************************************************************
 *   Copyright © 2008, 2009 Fredrik Höglund <fredrik@kde.org>              *
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "labelgenerator.h"

#include <KFilePlacesModel>
#include <KLocalizedString>
#include <KShell>

LabelGenerator::LabelGenerator(QObject* parent) : QObject(parent),
    m_placesModel(new KFilePlacesModel(this)),
    m_rtl(false)
{
}

LabelGenerator::~LabelGenerator()
{
}

QString LabelGenerator::url() const
{
    return m_url;
}

void LabelGenerator::setUrl(const QString &url)
{
    if (url != m_url) {
        m_url = url;
        emit urlChanged();
        emit displayLabelChanged();
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
        emit rtlChanged();
        emit displayLabelChanged();
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
        emit labelModeChanged();
        emit displayLabelChanged();
    }
}

QString LabelGenerator::labelText() const
{
    return m_labelText;
}

void LabelGenerator::setLabelText(const QString& text)
{
    if (text != m_labelText) {
        m_labelText = text;
        emit labelTextChanged();
        emit displayLabelChanged();
    }
}

QString LabelGenerator::displayLabel()
{
    if (m_labelMode == 1 /* Default */) {
        if (m_url == "desktop:/") {
            return i18n("Desktop Folder");
        } else {
            QUrl url(m_url);

            if (m_url.startsWith('~')) {
                url = QUrl::fromLocalFile(KShell::tildeExpand(m_url));
            }

            QString label(url.toString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash));

            const QModelIndex index = m_placesModel->closestItem(url);

            if (index.isValid()) {
                QString root = m_placesModel->url(index).toString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);

                label = label.right(label.length() - root.length());

                if (!label.isEmpty()) {
                    if (label.at(0) == '/') {
                        label.remove(0, 1);
                    }

                    if (m_rtl) {
                        label.prepend(" < ");
                    } else {
                        label.prepend(" > ");
                    }
                }

                label.prepend(m_placesModel->text(index));
            }

            return label;
        }
    } else if (m_labelMode == 2 /* Full path */) {
        return QUrl(m_url).toString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
    } else if (m_labelMode == 3 /* Custom title */) {
        return m_labelText;
    }

    return QString();
}
