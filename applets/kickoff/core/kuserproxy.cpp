/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
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

#include <unistd.h>

#include "kuserproxy.h"
#include <QFile>
#include <QTextStream>
#include <QUrl>

#include <KDirWatch>
#include <KLocalizedString>

#include <QDebug>

const QString etcPasswd = QStringLiteral("/etc/passwd");

KUserProxy::KUserProxy (QObject *parent)
    : QObject(parent),
    m_temporaryEmptyFaceIconPath(false)
{
    m_dirWatch.addFile(m_user.faceIconPath());
    if (QFile::exists(etcPasswd)) {
        m_dirWatch.addFile(etcPasswd);
    }

    connect(&m_dirWatch, &KDirWatch::dirty, this, &KUserProxy::update);
    connect(&m_dirWatch, &KDirWatch::created, this, &KUserProxy::update);
}

KUserProxy::~KUserProxy()
{
}

void KUserProxy::update(const QString &path)
{
        if (path == m_user.faceIconPath()) {
            // we need to force updates, even when the path doesn't change,
            // but the underlying image does. Change path temporarily, to
            // make the Image reload.
            // Needs cache: false in the Image item to actually reload
            m_temporaryEmptyFaceIconPath = true;
            emit faceIconPathChanged();
            m_temporaryEmptyFaceIconPath = false;
            emit faceIconPathChanged();
        } else if (path == etcPasswd) {
            m_user = KUser();
            emit nameChanged();
        }
    }
QString KUserProxy::fullName() const
{
    QString fullName = m_user.property(KUser::FullName).toString();
    if (!fullName.isEmpty()) {
        return fullName;
    }
    return loginName();
}

QString KUserProxy::loginName() const
{
    return m_user.loginName();
}

QString KUserProxy::faceIconPath() const
{
    if (m_temporaryEmptyFaceIconPath) {
        return QString();
    }
    const QString u = m_user.faceIconPath();
    const QFile f(u);
    if (f.exists(u)) {
        // We need to return a file URL, not a simple path
        return QUrl::fromLocalFile(u).toString();
    }
    return QString();
}

QString KUserProxy::os()
{
    if (m_os.isEmpty()) {
        m_os = i18n("Plasma by KDE");
        QFile osfile("/etc/os-release");
        if (osfile.exists()) {
            if (!osfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return QString();
            }

            QTextStream in(&osfile);
            while(!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("PRETTY_NAME")) {
                    QStringList fields = line.split("PRETTY_NAME=\"");
                    if (fields.count() == 2) {
                        osfile.close();
                        QString pretty = fields.at(1);
                        pretty.chop(1);
                        m_os = pretty;
                        return pretty;
                    }
                }
            }
            osfile.close();
        }
    }
    return m_os;
}

QString KUserProxy::host() const
{
    char hostname[256];
    hostname[0] = '\0';
    if (!gethostname(hostname, sizeof(hostname))) {
        hostname[sizeof(hostname)-1] = '\0';
    }
    return QString(hostname);
}

