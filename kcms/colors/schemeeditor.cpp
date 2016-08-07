/* KDE Display scheme editor
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

#include "scmeditordialog.h"

#include <QApplication>

int main(int argc, char* argv[])
{

    if (argc < 2) {
        return -1;
    }
    QApplication app(argc, argv);


    const QString fileBaseName(argv[1]);
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                "color-schemes/" + fileBaseName + ".colors");

    if (path.isEmpty()) {
        //FIXME:: dialog + return
        return -1;
    }
    SchemeEditorDialog dialog(path);

    dialog.show();

    return app.exec();
}
