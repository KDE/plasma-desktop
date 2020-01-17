/***************************************************************************
                          componentchooserbrowser.h
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

#ifndef COMPONENTCHOOSERBROWSER_H
#define COMPONENTCHOOSERBROWSER_H

#include "ui_browserconfig_ui.h"
#include "componentchooser.h"

class CfgBrowser: public QWidget, public Ui::BrowserConfig_UI, public CfgPlugin
{
Q_OBJECT
public:
	CfgBrowser(QWidget *parent);
	~CfgBrowser() override;
	void load(KConfig *cfg) override;
	void save(KConfig *cfg) override;
	void defaults() override;
	bool isDefaults() const override;

protected Q_SLOTS:
    void selectBrowser(int index);

Q_SIGNALS:
	void changed(bool);
private:
    int m_currentIndex = -1;
    int m_falkonIndex = -1;
};

#endif /* COMPONENTCHOOSERBROWSER_H */

