/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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

#ifndef KCM_KEYBOARD_H_
#define KCM_KEYBOARD_H_

#include "workspace_options.h"
#include <KCModule>

class KCMKeyboardWidget;
class KeyboardConfig;
struct Rules;

class KCMKeyboard : public KCModule
{
    Q_OBJECT

public:
    KCMKeyboard(QWidget *parent, const QVariantList &);
    ~KCMKeyboard() override;

    void save() override;
    void load() override;
    void defaults() override;

private:
    Rules *rules;
    KeyboardConfig *keyboardConfig;
    WorkspaceOptions m_workspaceOptions;
    KCMKeyboardWidget *widget;
};

#endif /* KCM_KEYBOARD_H_ */
