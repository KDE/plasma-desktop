/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "urlitemlauncher.h"

#include <config-X11.h>

// Qt
#include <QFileInfo>
#include <QHash>
#include <QModelIndex>
#if HAVE_X11
#include <QX11Info>
#endif

// KDE
#include <KAuthorized>
#include <QDebug>
#include <KRun>
#include <KStartupInfo>
#include <KUrl>
#include <Solid/Device>
#include <Solid/StorageAccess>

// Local
#include "models.h"

// DBus
#include "krunner_interface.h"

using namespace Kickoff;

class HandlerInfo
{
public:
    HandlerInfo() : type(UrlItemLauncher::ProtocolHandler) , handler(0) {}
    UrlItemLauncher::HandlerType type;
    UrlItemHandler *handler;
};

class GenericItemHandler : public UrlItemHandler
{
public:
    virtual bool openUrl(const KUrl& url) {
        if (url.protocol() == "run" && KAuthorized::authorize("run_command")) {
            QString interface("org.kde.krunner");
            org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
            krunner.display();
            return true;
        }
        // fetch current timestamp for creating ASN id.
        // TODO: get timestamp from input event triggering the openUrl
        quint32 timestamp = 0;
#if HAVE_X11
        if (QX11Info::isPlatformX11()) {
            timestamp = QX11Info::appUserTime();
        }
#endif

        new KRun(url, 0, true, KStartupInfo::createNewStartupIdForTimestamp(timestamp));
        return true;
    }
};

class UrlItemLauncher::Private
{
public:
    static QHash<QString, HandlerInfo> globalHandlers;
    static GenericItemHandler genericHandler;

    static bool openUrl(const QString &urlString) {
        qDebug() << "Opening item with URL" << urlString;

        KUrl url(urlString);
        HandlerInfo protocolHandler = globalHandlers[url.scheme()];
        if (protocolHandler.type == ProtocolHandler && protocolHandler.handler != 0) {
            return protocolHandler.handler->openUrl(url);
        }

        QString extension = QFileInfo(url.path()).suffix();
        HandlerInfo extensionHandler = globalHandlers[extension];
        if (extensionHandler.type == ExtensionHandler && extensionHandler.handler != 0) {
            return extensionHandler.handler->openUrl(url);
        }

        return genericHandler.openUrl(url);
    }
};

QHash<QString, HandlerInfo> UrlItemLauncher::Private::globalHandlers;
GenericItemHandler UrlItemLauncher::Private::genericHandler;

UrlItemLauncher::UrlItemLauncher(QObject *parent)
        : QObject(parent)
        , d(new Private)
{
}

UrlItemLauncher::~UrlItemLauncher()
{
    delete d;
}

bool UrlItemLauncher::openItem(const QModelIndex& index)
{
    QString urlString = index.data(UrlRole).value<QString>();
    if (urlString.isEmpty()) {
        QString udi = index.data(DeviceUdiRole).toString();
        if (!udi.isEmpty()) {
            Solid::Device device(udi);
            Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

            if (access && !access->isAccessible()) {
                connect(access, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)),
                        this, SLOT(onSetupDone(Solid::ErrorType,QVariant,QString)));
                access->setup();
                return true;
            }
        }

        qDebug() << "Item" << index.data(Qt::DisplayRole) << "has no URL to open.";
        return false;
    }

    return Private::openUrl(urlString);
}

bool UrlItemLauncher::openUrl(const QString& url)
{
    return Private::openUrl(url);
}

void UrlItemLauncher::onSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
    Q_UNUSED(errorData);

    if (error != Solid::NoError) {
        return;
    }

    Solid::Device device(udi);
    Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

    Q_ASSERT(access);

    QString urlString = "file://" + access->filePath();
    Private::openUrl(urlString);
}

// FIXME: the handlers are leaked, as they are added with each new Kickoff instance,
//        but never deleted.
void UrlItemLauncher::addGlobalHandler(HandlerType type, const QString& name, UrlItemHandler *handler)
{
    HandlerInfo info;
    info.type = type;
    info.handler = handler;
    Private::globalHandlers.insert(name, info);
}


#include "urlitemlauncher.moc"
