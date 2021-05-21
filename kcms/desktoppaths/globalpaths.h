/*
    Copyright (c) Martin R. Jones 1996
    SPDX-FileCopyrightText: 1998-2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Summarized history:
//
// "Desktop Icons Options" Tab for KDesktop configuration
// Martin Jones
//
// Port to KControl, split from "Misc" Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000

#ifndef __GLOBALPATHS_H
#define __GLOBALPATHS_H

#include <KCModule>

namespace Ui
{
class DesktopPathsView;
}

class KUrlRequester;
class DesktopPathsSettings;
class DesktopPathsData;

//-----------------------------------------------------------------------------
// The "Path" Tab contains :
// The paths for Desktop and Documents

class DesktopPathConfig : public KCModule
{
    Q_OBJECT
public:
    DesktopPathConfig(QWidget *parent, const QVariantList &args);
    ~DesktopPathConfig() override;

private Q_SLOTS:
    void updateDefaultIndicator();

private:
    void setDefaultIndicatorVisible(KUrlRequester *widget, const QVariant &defaultValue);

    QScopedPointer<Ui::DesktopPathsView> m_ui;
    DesktopPathsData *m_data;
};

#endif
