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

#ifndef URLITEMLAUNCHER_H
#define URLITEMLAUNCHER_H

#include <QObject>
#include <solid/storageaccess.h>

class QModelIndex;
class KUrl;

namespace Kickoff
{

/**
 * UrlItemHandler is an abstract base class for handlers which can open particular
 * types of URL.
 *
 * @see UrlItemLauncher
 */
class UrlItemHandler
{
public:
    virtual ~UrlItemHandler() {}
    virtual bool openUrl(const KUrl& url) = 0;
};

/**
 * UrlItemLauncher provides facilities to open a item from a Kickoff model based on its UrlRole
 * data.
 *
 * By default, a UrlItemLauncher opens all URLs using the KRun class.  Additional handlers can be created
 * to handle URLs with particular protocols or extensions differently.  Handlers can be
 * registered using the static addGlobalHandler() method.
 */
class UrlItemLauncher : public QObject
{
    Q_OBJECT

public:
    UrlItemLauncher(QObject *parent = 0);
    virtual ~UrlItemLauncher();

    enum HandlerType {
        ProtocolHandler,
        ExtensionHandler
    };
    static void addGlobalHandler(HandlerType type,
                                 const QString& name,
                                 UrlItemHandler *handler);

public Q_SLOTS:
    /** Open the specified @p index from a Kickoff model. */
    bool openItem(const QModelIndex& index);
    /** Open the specified @p url */
    bool openUrl(const QString& url);

private Q_SLOTS:
    void onSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

private:
    class Private;
    Private * const d;
};

}

#endif // URLITEMLAUNCHER_H

