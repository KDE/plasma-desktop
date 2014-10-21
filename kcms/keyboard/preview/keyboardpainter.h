/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEYBOARDPAINTER_H
#define KEYBOARDPAINTER_H

#include "kbpreviewframe.h"

#include <QDialog>

class QPushButton;
class QComboBox;

class KeyboardPainter : public QDialog
{
    Q_OBJECT

public:
    explicit KeyboardPainter();
    ~KeyboardPainter();
    void generateKeyboardLayout(const QString &layout, const QString &variant, const QString &model, const QString &title);
    int getHeight();
    int getWidth();

public Q_SLOTS:
    void levelChanged(int l_id);

private:
    QDialog *kbDialog;
    KbPreviewFrame *kbframe;
    QPushButton *exitButton;
    QComboBox *levelBox;
};

#endif // KEYBOARDPAINTER_H
