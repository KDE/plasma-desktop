/*
    "Desktop Options" Tab for KDesktop configuration

    SPDX-FileCopyrightText: 1996 Martin R. Jones
    SPDX-FileCopyrightText: 1998 Bernd Wuebben
    SPDX-FileCopyrightText: 1998 Christian Tibirna
    SPDX-FileCopyrightText: 1998-2007 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "globalpaths.h"
#include "desktoppathsdata.h"
#include "desktoppathssettings.h"
#include "ui_globalpaths.h"

#include <KLineEdit>
#include <KPluginFactory>
#include <KUrlRequester>

K_PLUGIN_FACTORY(KcmDesktopPathsFactory, registerPlugin<DesktopPathConfig>(); registerPlugin<DesktopPathsData>();)

DesktopPathConfig::DesktopPathConfig(QWidget *parent, const QVariantList &)
    : KCModule(parent)
    , m_ui(new Ui::DesktopPathsView)
    , m_data(new DesktopPathsData(this))
{
    m_ui->setupUi(this);
    setQuickHelp(
        i18n("<h1>Paths</h1>\n"
             "This module allows you to choose where in the filesystem the "
             "files on your desktop should be stored.\n"
             "Use the \"Whats This?\" (Shift+F1) to get help on specific options."));
    addConfig(m_data->settings(), this);

    connect(this, &DesktopPathConfig::defaultsIndicatorsVisibleChanged, this, &DesktopPathConfig::updateDefaultIndicator);
    connect(m_data->settings(), &DesktopPathsSettings::widgetChanged, this, &DesktopPathConfig::updateDefaultIndicator);
}

DesktopPathConfig::~DesktopPathConfig()
{
}

void DesktopPathConfig::updateDefaultIndicator()
{
    setDefaultIndicatorVisible(m_ui->kcfg_desktopLocation, m_data->settings()->defaultDesktopLocation());
    setDefaultIndicatorVisible(m_ui->kcfg_documentsLocation, m_data->settings()->defaultDocumentsLocation());
    setDefaultIndicatorVisible(m_ui->kcfg_downloadsLocation, m_data->settings()->defaultDownloadsLocation());
    setDefaultIndicatorVisible(m_ui->kcfg_musicLocation, m_data->settings()->defaultMusicLocation());
    setDefaultIndicatorVisible(m_ui->kcfg_picturesLocation, m_data->settings()->defaultPicturesLocation());
    setDefaultIndicatorVisible(m_ui->kcfg_videosLocation, m_data->settings()->defaultVideosLocation());
}

void DesktopPathConfig::setDefaultIndicatorVisible(KUrlRequester *widget, const QVariant &defaultValue)
{
    bool isDefault = widget->url() == defaultValue.toUrl();
    widget->lineEdit()->setProperty("_kde_highlight_neutral", defaultsIndicatorsVisible() && !isDefault);
    widget->update();
}

#include "globalpaths.moc"
