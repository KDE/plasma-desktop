/***************************************************************************
                          componentchooserterminal.h  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundationi                            *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMPONENTCHOOSERTERMINAL_H_
#define _COMPONENTCHOOSERTERMINAL_H_

#include "ui_terminalemulatorconfig_ui.h"
#include "componentchooser.h"
class KConfig;
class CfgPlugin;

class CfgTerminalEmulator: public QWidget, public Ui::TerminalEmulatorConfig_UI, public CfgPlugin
{
    Q_OBJECT
public:
	CfgTerminalEmulator(QWidget *parent);
	virtual ~CfgTerminalEmulator();
	virtual void load(KConfig *cfg);
	virtual void save(KConfig *cfg);
	virtual void defaults();

protected Q_SLOTS:
	void selectTerminalApp();
	void configChanged();

Q_SIGNALS:
	void changed(bool);
};

#endif
