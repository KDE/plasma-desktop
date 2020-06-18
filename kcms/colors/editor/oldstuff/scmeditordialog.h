/* ColorEdit widget for KDE Display color scheme setup module
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __SCMEDITORDIALOG_H__
#define __SCMEDITORDIALOG_H__

#include <KColorScheme>
#include <KSharedConfig>

#include <QFrame>
#include <QPalette>
#include <QDialog>

#include "ui_scmeditordialog.h"

class SchemeEditorOptions;
class SchemeEditorColors;
class SchemeEditorEffects;

class SchemeEditorDialog : public QDialog, public Ui::ScmEditorDialog
{
    Q_OBJECT

public:
    SchemeEditorDialog(const QString &path, QWidget *parent = nullptr);
    SchemeEditorDialog(KSharedConfigPtr config, QWidget *parent = nullptr);

    bool showApplyOverwriteButton() const;
    void setShowApplyOverwriteButton(bool show);

Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:

    /** slot called when the upload scheme button is clicked */
    void on_schemeKnsUploadButton_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

    void updateTabs(bool byUser=false);

private:
    void init();
    /** save the current scheme */
    void saveScheme(bool overwrite);
    void setUnsavedChanges(bool changes);

    const QString m_filePath;
    QString m_schemeName;
    KSharedConfigPtr m_config;
    bool m_disableUpdates = false;
    bool m_unsavedChanges = false;

    SchemeEditorOptions *m_optionTab;
    SchemeEditorColors *m_colorTab;
    SchemeEditorEffects *m_disabledTab;
    SchemeEditorEffects *m_inactiveTab;

    bool m_showApplyOverwriteButton = false;
};

#endif
