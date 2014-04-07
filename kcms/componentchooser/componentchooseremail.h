/***************************************************************************
                          componentchooseremail.h
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

#ifndef COMPONENTCHOOSEREMAIL_H
#define COMPONENTCHOOSEREMAIL_H

class KEMailSettings;

#include "ui_emailclientconfig_ui.h"
#include "componentchooser.h"

class CfgEmailClient: public QWidget, public Ui::EmailClientConfig_UI, public CfgPlugin
{
    Q_OBJECT
public:
    CfgEmailClient(QWidget *parent);
    virtual ~CfgEmailClient();
    virtual void load(KConfig *cfg);
    virtual void save(KConfig *cfg);
    virtual void defaults();

private:
    KEMailSettings *pSettings;

protected Q_SLOTS:
    void selectEmailClient();
    void configChanged();
Q_SIGNALS:
    void changed(bool);
};

#endif
