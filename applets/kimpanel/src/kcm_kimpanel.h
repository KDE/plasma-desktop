/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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

#ifndef KCM_KIMPANEL_H
#define KCM_KIMPANEL_H

#include "ui_kcm_kimpanel.h"
#include "kimthemepackage.h"

#include <KCModule>
#include <KMenu>
#include <KFileDialog>
#include <QWidget>

namespace KIM
{
    class PanelCm : public KCModule, protected Ui::kimpanelConfig
    {
        Q_OBJECT
    public:
        PanelCm(QWidget *parent,const QVariantList &);
        ~PanelCm();
        virtual void load();
        virtual void save();
        virtual void defaults();

    private Q_SLOTS:
        void populateWidgetsMenu();

        void downloadThemes();
        void openThemeFile();

        void reloadThemes();

        void themeComboxIndexChanged(int index);
        void checkChanged();

    private:
        KMenu *m_widgetsMenu;
        ThemePackage *m_installer;
    };
} // namespace KIM

#endif //KCM_KIMPANEL_H
