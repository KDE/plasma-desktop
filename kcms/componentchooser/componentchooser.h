/***************************************************************************
                          componentchooser.h  -  description
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

#ifndef _COMPONENTCHOOSER_H_
#define _COMPONENTCHOOSER_H_

#include "ui_componentchooser_ui.h"
#include <QHash>

#include <QComboBox>

#include <KService>

class QWidget;
class KConfig;

class CfgPlugin : public QComboBox
{
    Q_OBJECT

public:
    CfgPlugin(QWidget *parent): QComboBox(parent) {}
	virtual ~CfgPlugin(){}
	virtual void load(KConfig *cfg)=0;
    virtual void save(KConfig *cfg)=0;

    bool hasChanged() const
    {
        return count() > 1 && m_currentIndex != currentIndex();
    }

    void defaults()
    {
        if (m_defaultIndex != -1) {
            setCurrentIndex(m_defaultIndex);
        }
    }

    int validLastCurrentIndex() const
    {
        // m_currentIndex == -1 means there are no previously saved value
        // or maybe there were no choices in the combobox
        // return 0 in those cases
        return m_currentIndex == -1 ? 0 : m_currentIndex;
    }

    bool isDefaults() const
    {
        return m_defaultIndex == -1 || m_defaultIndex == currentIndex();
    }

Q_SIGNALS:
    void changed(bool);

protected:
    // the currently saved selected option
    int m_currentIndex = -1;
    // the index default of the default option
    int m_defaultIndex = -1;
};

class ComponentChooser : public QWidget, public Ui::ComponentChooser_UI
{

Q_OBJECT

public:
	ComponentChooser(QWidget *parent=nullptr);
	~ComponentChooser() override;
	void load();
	void save();
	void restoreDefault();

private:
    QMap<QString, CfgPlugin*> configWidgetMap;

    CfgPlugin *loadConfigWidget(const QString &cfgType);

protected Q_SLOTS:
    void emitChanged();

Q_SIGNALS:
	void changed(bool);
	void defaulted(bool);
};


#endif
