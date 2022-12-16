/*
    "Desktop Icons Options" Tab for KDesktop configuration

    SPDX-FileCopyrightText: 1996 Martin R. Jones
    SPDX-FileCopyrightText: 1998, 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <memory>

namespace Ui
{
class DesktopPathsView;
}

class KUrlRequester;
class DesktopPathsSettings;
class DesktopPathsData;

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

    std::unique_ptr<Ui::DesktopPathsView> m_ui;
    DesktopPathsData *const m_data;
};
