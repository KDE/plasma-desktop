/***************************************************************************
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KIMPANEL_H
#define KIMPANEL_H

#include "ui_config.h"

// Plasma
#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <QTimer>

namespace Plasma
{
class IconWidget;
}

class QGraphicsLinearLayout;
class KimpanelInputPanel;
class KimpanelStatusBarGraphics;
class Kimpanel : public Plasma::Applet
{
    Q_OBJECT
public:
    Kimpanel(QObject* parent, const QVariantList& args);
    virtual ~Kimpanel();

    virtual void init();
    virtual void configChanged();
    virtual void constraintsEvent(Plasma::Constraints constraints);

    virtual void createConfigurationInterface(KConfigDialog *parent);
    virtual QList< QAction* > contextualActions();
signals:
    void configFontChanged();
public slots:
    void dataUpdated(const QString& source, const Plasma::DataEngine::Data &data);
protected slots:
    void lookupTablePageUp();
    void lookupTablePageDown();
    void selectCandidate(int index);
    void triggerProperty(const QString& property);
    void configAccepted();
    void configFont();
    void configure();
    void reloadConfig();
    void exitIM();
    void startIM();
    void selectIM();
    void iconSizeChanged();
    void updateInputPanel();
    void updateStatusBar();
protected:
    Plasma::DataEngine* m_engine;
    KimpanelInputPanel* m_inputpanel;
    KimpanelStatusBarGraphics* m_statusbar;
    QGraphicsLinearLayout* m_layout;
    Plasma::Service* m_inputpanelService;
    Plasma::Service* m_statusbarService;

    Ui::GeneralConfig m_generalUi;
    QFont m_font;
    quint64 m_menuTimeStamp;
    QTimer m_inputPanelTimer;
    QTimer m_statusBarTimer;
};

K_EXPORT_PLASMA_APPLET(kimpanel, Kimpanel)

#endif // KIMPANEL_H
