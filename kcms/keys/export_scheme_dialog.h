#ifndef EXPORT_SCHEME_DIALOG_H
#define EXPORT_SCHEME_DIALOG_H
/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ui_export_scheme_dialog.h"

#include <QButtonGroup>
#include <KDialog>

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class ExportSchemeDialog : public KDialog
    {
    Q_OBJECT

public:

    ExportSchemeDialog (QStringList components, QWidget *parent=NULL);

    virtual ~ExportSchemeDialog();

    // Get the list of currently selected components
    QStringList selectedComponents() const;

private:

    Ui::ExportSchemeDialog  ui;

    // list of components to choose from
    QStringList mComponents;

    // list of buttons for the components
    QButtonGroup mButtons;

    }; // ExportSchemeDialog




#endif /* EXPORT_SCHEME_DIALOG_H */

