/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

#ifndef RECENTAPPLICATIONS_H
#define RECENTAPPLICATIONS_H

// Qt
#include <QtCore/QObject>
#include <QDateTime>

// KDE
#include <KService>

namespace Kickoff
{

/**
 * Singleton class which can be used to keep track of recently started applications
 * in the Kickoff launcher.
 *
 * RecentApplications information persists between sessions.
 */
class RecentApplications : public QObject
{
    Q_OBJECT
public:
    class Private;
    friend class Private;

    static RecentApplications *self();

    /**
     * List of service pointers for recently started applications in the order in which
     * they were started, with the most recently used application first.
     */
    QList<KService::Ptr> recentApplications() const;
    /** Returns the number of times an application represented by @p service has been started. */
    int startCount(KService::Ptr service) const;
    /** Returns the last time and date with the application represented by @p service was started. */
    QDateTime lastStartedTime(KService::Ptr service) const;

    /** Sets the maximum number of recently used applications to remember. */
    void setMaximum(int max);
    /** Returns the maximum number of recently used applications that are remembered. */
    int maximum() const;
    /** Returns the default maximum number of recently used applications that are remembered as defined
    either in the configfile as "MaxApplications" or via the DEFAULT_MAX_SERVICES macro. */
    int defaultMaximum() const;

public Q_SLOTS:
    /**
     * Registers the startup of an application.  This should be called whenever a new application
     * or service is started.
     */
    void add(KService::Ptr service);
    /** Clear the list of recent applications. */
    void clear();

Q_SIGNALS:
    /** Emitted after add() has been called. */
    void applicationAdded(KService::Ptr service, int startCount);
    /** Emitted after remove() has been called. */
    void applicationRemoved(KService::Ptr service);
    /** Emitted after clear() has been called. */
    void cleared();

private:
    RecentApplications();
};

}

#endif // RECENTAPPLICATIONS_H


