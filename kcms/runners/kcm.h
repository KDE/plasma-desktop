/*
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KCModule>
#include <KConfig>
#include <KConfigGroup>
#include <QListWidget>

class QToolButton;
class KCModuleProxy;

namespace Plasma {
    class AbstractRunner;
}

class SearchConfigModule : public KCModule
{
    Q_OBJECT

public:
    enum Roles {
        RunnersRole = Qt::UserRole + 1
    };

    SearchConfigModule(QWidget* parent, const QVariantList& args);

public Q_SLOTS:
    void load();
    void save();
    void defaults();
    void configureClicked();

private:
    QListWidget* m_listWidget;

    KConfig m_config;
    KConfigGroup m_configGroup;
    QToolButton *m_configButton;
    QMultiHash <QString, Plasma::AbstractRunner *> m_runnerCategories;
};

#endif
