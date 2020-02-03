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
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#ifndef COMPONENTCHOOSEREMAIL_H
#define COMPONENTCHOOSEREMAIL_H

#include "componentchooser.h"
#include <QComboBox>

class KEMailSettings;

class CfgEmailClient: public CfgPlugin
{
    Q_OBJECT
public:
    CfgEmailClient(QWidget *parent);
    ~CfgEmailClient() override;
    void load(KConfig *cfg) override;
    void save(KConfig *cfg) override;

private:
    KEMailSettings *pSettings;

protected Q_SLOTS:
    void selectEmailClient(int index);
};

#endif
