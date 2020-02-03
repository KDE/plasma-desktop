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
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#ifndef COMPONENTCHOOSERBROWSER_H
#define COMPONENTCHOOSERBROWSER_H

#include "componentchooser.h"
#include <QComboBox>

class CfgBrowser: public CfgPlugin
{
Q_OBJECT
public:
    CfgBrowser(QWidget *parent);
    ~CfgBrowser() override;
    void load(KConfig *cfg) override;
    void save(KConfig *cfg) override;

protected Q_SLOTS:
    void selectBrowser(int index);
};

#endif /* COMPONENTCHOOSERBROWSER_H */

