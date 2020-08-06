/***************************************************************************
                          kcm_componentchooser.cpp  -  description
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

#include "kcm_componentchooser.h"


#include <QVBoxLayout>

#include <KAboutData>
#include <KComponentData>
#include <KPluginFactory>
#include <KLocalizedString>


K_PLUGIN_FACTORY(KCMComponentChooserFactory,
        registerPlugin<KCMComponentChooser>();
        )

KCMComponentChooser::KCMComponentChooser(QWidget *parent, const QVariantList &):
	KCModule(parent) {

	QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

	m_chooser=new ComponentChooser(this);
	lay->addWidget(m_chooser);
	connect(m_chooser,SIGNAL(changed(bool)),this,SIGNAL(changed(bool)));
	connect(m_chooser, &ComponentChooser::defaulted, this, &KCModule::defaulted);

    KAboutData *about = new KAboutData( QStringLiteral("kcmcomponentchooser"), i18n("Component Chooser"), QStringLiteral("1.0"),
            QString(), KAboutLicense::GPL,
            i18n("(c), 2002 Joseph Wenninger"));

    about->addAuthor(i18n("Joseph Wenninger"), QString() , QStringLiteral("jowenn@kde.org"));
    about->addAuthor(i18n("Méven Car"), QString() , QStringLiteral("meven.car@kdemail.net"));
    setAboutData( about );
}

void KCMComponentChooser::load(){
	m_chooser->load();
}

void KCMComponentChooser::save(){
	m_chooser->save();
}

void KCMComponentChooser::defaults(){
	m_chooser->restoreDefault();
}

#include "kcm_componentchooser.moc"
