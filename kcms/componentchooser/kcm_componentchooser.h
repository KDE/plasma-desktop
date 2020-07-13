/***************************************************************************
                          kcm_componentchooser.h  -  description
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

#ifndef KCM_COMPONENTCHOOSER_H
#define KCM_COMPONENTCHOOSER_H

#include <KCModule>

#include "componentchooser.h"


class KCMComponentChooser: public KCModule
{
    Q_OBJECT
public:
    KCMComponentChooser(QWidget *parent, const QVariantList &args);

    void load() override;
    void save() override;
    void defaults() override;

private:
    ComponentChooser  *m_chooser;
};

#endif // KCM_COMPONENTCHOOSER_H
