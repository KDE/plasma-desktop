/* This file is part of the KDE Project
   Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

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

#ifndef _BALOO_FILE_KCM_H_
#define _BALOO_FILE_KCM_H_

#include <KCModule>
#include "ui_configwidget.h"

namespace Baloo
{

class ServerConfigModule : public KCModule, private Ui::ConfigWidget
{
    Q_OBJECT

public:
    ServerConfigModule(QWidget* parent, const QVariantList& args);
    ~ServerConfigModule();

public Q_SLOTS:
    void load();
    void save();
    void defaults();

    void folderSelectionChanged();
private:
    bool m_previouslyEnabled;
};
}

#endif
