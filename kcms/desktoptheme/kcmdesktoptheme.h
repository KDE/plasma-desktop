/*
 * KCMDesktopTheme
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 *
 * Portions Copyright (C) TrollTech AS.
 *
 * Based on kcmstyle which is based on kcmdisplay
 * Copyright (C) 1997-2002 kcmdisplay Authors.
 * (see Help -> About Style Settings)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KCMDESKTOPTHEME_H
#define KCMDESKTOPTHEME_H

#include <kcmodule.h>


#include "ui_DesktopTheme.h"

#include <QProcess>

class QFileDialog;
class ThemeModel;

namespace Plasma
{
    class Theme;
}

class KCMDesktopTheme : public KCModule, public Ui::DesktopTheme
{
    Q_OBJECT

public:
    KCMDesktopTheme( QWidget* parent, const QVariantList& );
    ~KCMDesktopTheme();

    virtual void load();
    virtual void save();
    virtual void defaults();

protected Q_SLOTS:
    void loadDesktopTheme();

    void setDesktopThemeDirty();

    void getNewThemes();

    void detailChanged();
    void installFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void installError(QProcess::ProcessError e);

private:
    static QString toolbarButtonText(int index);
    static int toolbarButtonIndex(const QString &text);

    bool m_bDesktopThemeDirty;
    bool m_bDetailsDirty;

    void showFileDialog();
    QFileDialog *m_dialog;
    void installTheme(const QString &file);
    void fileBrowserCompleted();

    ThemeModel* m_themeModel;
    Plasma::Theme *m_defaultTheme;
};

#endif // KCMDESKTOPTHEME_H

// vim: set noet ts=4:
