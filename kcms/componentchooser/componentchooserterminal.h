/***************************************************************************
                          componentchooserterminal.h  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger <jowenn@kde.org>
    copyright            : (C) 2020 by Méven Car <meven.car@enioka.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMPONENTCHOOSERTERMINAL_H_
#define _COMPONENTCHOOSERTERMINAL_H_

#include "componentchooser.h"
#include <QComboBox>

class KConfig;
class CfgPlugin;

class CfgTerminalEmulator: public CfgPlugin
{
    Q_OBJECT
public:
    CfgTerminalEmulator(QWidget *parent);
    ~CfgTerminalEmulator() override;
    void load(KConfig *cfg) override;
    void save(KConfig *cfg) override;
protected Q_SLOTS:
    void selectTerminalApp();
    void selectTerminalEmulator(int index);
};

#endif
