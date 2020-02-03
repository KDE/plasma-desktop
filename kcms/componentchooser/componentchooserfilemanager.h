/*  This file is part of the KDE project
    Copyright (C) 2008 David Faure <faure@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef COMPONENTCHOOSERFILEMANAGER_H
#define COMPONENTCHOOSERFILEMANAGER_H

#include "componentchooser.h"
#include <QComboBox>

class CfgFileManager: public CfgPlugin
{
    Q_OBJECT
public:
    CfgFileManager(QWidget *parent);
    ~CfgFileManager() override;
    void load(KConfig *cfg) override;
    void save(KConfig *cfg) override;

protected Q_SLOTS:
    void selectFileManager(int index);
};

#endif
