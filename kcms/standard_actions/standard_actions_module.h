#ifndef STANDARD_ACTIONS_MODULE_H
#define STANDARD_ACTIONS_MODULE_H
/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <KCModule>

class KActionCollection;
class KShortcutsEditor;

class StandardActionsModule : public KCModule
    {
    Q_OBJECT
public:

    StandardActionsModule(QWidget *parent, const QVariantList &args);
    ~StandardActionsModule();

    /*reimp*/ void save();
    /*reimp*/ void load();
    /*reimp*/ void defaults();

private Q_SLOTS:
    void keyChanged();

private:

    KShortcutsEditor *m_editor;
    KActionCollection *m_actionCollection;

}; // class StandardActionsModule


#endif /* #ifndef STANDARD_ACTIONS_MODULE_H */
