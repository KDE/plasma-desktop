/*
 *  kcmformats.h
 *  Copyright 2014 Sebastian Kuegler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */
#ifndef __kcmformats_h__
#define __kcmformats_h__

#include <kcmodule.h>

namespace Ui {
    class KCMFormatsWidget;
}
class KCMFormats : public KCModule
{
    Q_OBJECT

public:
    explicit KCMFormats( QWidget *parent=0, const QVariantList &list=QVariantList() );

    void load();
    void save();
    void defaults();
private:
    Ui::KCMFormatsWidget* m_ui;
};

#endif
