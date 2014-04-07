/***************************************************************************
 *   Copyright (C) 2007 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef COUNTRYSELECTORDIALOG_H
#define COUNTRYSELECTORDIALOG_H

#include <KDialog>

class CSDListView;

class KControlLocale;

class QModelIndex;

class CountrySelectorDialog : public KDialog
{
Q_OBJECT
    public:
        CountrySelectorDialog(QWidget *parent);

        bool editCountry(KControlLocale *locale);

    private slots:
        void regionChanged(const QModelIndex &current);
        void regionActivated();

    private:
        CSDListView *m_countriesView;
};

#endif
