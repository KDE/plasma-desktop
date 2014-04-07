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

//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <KPluginFactory>
#include <KLocalizedString>


K_PLUGIN_FACTORY(KCMComponentChooserFactory,
        registerPlugin<KCMComponentChooser>();
        )
K_EXPORT_PLUGIN(KCMComponentChooserFactory("kcmcomponentchooser"))

KCMComponentChooser::KCMComponentChooser(QWidget *parent, const QVariantList &):
	KCModule(parent) {

	QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin(0);

	m_chooser=new ComponentChooser(this);
	lay->addWidget(m_chooser);
	connect(m_chooser,SIGNAL(changed(bool)),this,SIGNAL(changed(bool)));
	setButtons( Default|Apply|Help );

	KAboutData *about =
	new KAboutData( "kcmcomponentchooser", 0, i18n("Component Chooser"),
			0, QString(), KAboutData::License_GPL,
			i18n("(c), 2002 Joseph Wenninger"));

	about->addAuthor(i18n("Joseph Wenninger"), QString() , "jowenn@kde.org");
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
