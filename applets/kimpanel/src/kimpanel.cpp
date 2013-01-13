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

#include "kimpanel.h"
#include "kimpanelinputpanel.h"
#include "kimpanelsettings.h"
#include "kimpanelstatusbargraphics.h"

// Qt
#include <QGraphicsLinearLayout>

// KDE
#include <KRun>
#include <KIcon>
#include <KConfigDialog>
#include <KFontDialog>
#include <KDesktopFile>
#include <KOpenWithDialog>

// Plasma
#include <Plasma/DataEngine>
#include <Plasma/Corona>
#include <Plasma/ServiceJob>
#include <Plasma/IconWidget>

Kimpanel::Kimpanel(QObject* parent, const QVariantList& args):
    Applet(parent, args),
    m_engine(0),
    m_inputpanel(new KimpanelInputPanel()),
    m_statusbar(new KimpanelStatusBarGraphics),
    m_layout(new QGraphicsLinearLayout()),
    m_inputpanelService(0),
    m_statusbarService(0),
    m_menuTimeStamp(QDateTime::currentMSecsSinceEpoch())
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addItem(m_statusbar);
    m_statusbar->show();
    setLayout(m_layout);

    m_inputPanelTimer.setInterval(1);
    m_inputPanelTimer.setSingleShot(true);

    m_statusBarTimer.setInterval(16);
    m_statusBarTimer.setSingleShot(true);

    connect(m_inputpanel, SIGNAL(selectCandidate(int)), this, SLOT(selectCandidate(int)));
    connect(m_inputpanel, SIGNAL(lookupTablePageUp()), this, SLOT(lookupTablePageUp()));
    connect(m_inputpanel, SIGNAL(lookupTablePageDown()), this, SLOT(lookupTablePageDown()));
    connect(m_statusbar, SIGNAL(triggerProperty(QString)), this, SLOT(triggerProperty(QString)));
    connect(m_statusbar, SIGNAL(reloadConfig()), this, SLOT(reloadConfig()));
    connect(m_statusbar, SIGNAL(configure()), this, SLOT(configure()));
    connect(m_statusbar, SIGNAL(exitIM()), this, SLOT(exitIM()));
    connect(m_statusbar, SIGNAL(startIM()), this, SLOT(startIM()));

    connect(&m_inputPanelTimer, SIGNAL(timeout()), this, SLOT(updateInputPanel()));
    connect(&m_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
}

Kimpanel::~Kimpanel()
{
    delete m_inputpanel;
}

void Kimpanel::init()
{
    m_engine = dataEngine("kimpanel");
    m_engine->connectSource("inputpanel", this);
    m_engine->connectSource("statusbar", this);
    m_inputpanelService = m_engine->serviceForSource("inputpanel");
    m_statusbarService = m_engine->serviceForSource("statusbar");
}

void Kimpanel::configChanged()
{
}

void Kimpanel::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor ff = formFactor();

        m_statusbar->setLayoutMode(
            ff == Plasma::Horizontal
            ? IconGridLayout::PreferRows
            : IconGridLayout::PreferColumns);

        // Apply icon size
        iconSizeChanged();

        m_layout->setOrientation(
            ff == Plasma::Vertical ? Qt::Vertical : Qt::Horizontal);
    }
}

void Kimpanel::iconSizeChanged()
{
    Plasma::FormFactor ff = formFactor();

    if (ff == Plasma::Planar || ff == Plasma::MediaCenter) {
        m_statusbar->setPreferredIconSize(IconSize(KIconLoader::Desktop));
    } else {
        m_statusbar->setPreferredIconSize(IconSize(KIconLoader::Panel));
    }
}

void Kimpanel::createConfigurationInterface(KConfigDialog* parent)
{
    QWidget *widget = new QWidget();
    m_generalUi.setupUi(widget);
    parent->addPage(widget, i18nc("General configuration page", "General"), icon());
    m_generalUi.verticalListCheckBox->setChecked(KimpanelSettings::self()->verticalPreeditBar());
    m_generalUi.reverseCheckBox->setChecked(KimpanelSettings::self()->useReverse());

    m_font = KimpanelSettings::self()->font();
    m_generalUi.fontPreviewLabel->setText(QString("%1 %2").arg(m_font.family()).arg(m_font.pointSize()));
    m_generalUi.fontPreviewLabel->setFont(m_font);

    connect(m_generalUi.fontButton, SIGNAL(clicked(bool)), this, SLOT(configFont()));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    connect(this, SIGNAL(configFontChanged()), parent, SLOT(settingsModified()));
    connect(m_generalUi.verticalListCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(m_generalUi.reverseCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(m_generalUi.selectIMButton, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    connect(m_generalUi.selectIMButton, SIGNAL(clicked(bool)), this, SLOT(selectIM()));
}

void Kimpanel::configFont()
{
    int result = KFontDialog::getFont(m_font);
    if (result == KFontDialog::Accepted) {
        m_generalUi.fontPreviewLabel->setText(QString("%1 %2").arg(m_font.family()).arg(m_font.pointSize()));
        m_generalUi.fontPreviewLabel->setFont(m_font);
        emit configFontChanged();
    }
}


void Kimpanel::configAccepted()
{
    KimpanelSettings::self()->setVerticalPreeditBar(m_generalUi.verticalListCheckBox->isChecked());
    KimpanelSettings::self()->setUseReverse(m_generalUi.reverseCheckBox->isChecked());
    KimpanelSettings::self()->setFont(m_font);
    KimpanelSettings::self()->writeConfig();
}

void Kimpanel::updateStatusBar()
{
    const Plasma::DataEngine::Data& data = m_engine->query("statusbar");
    m_statusbar->updateProperties(data["Properties"]);
}

void Kimpanel::updateInputPanel()
{
    const Plasma::DataEngine::Data& data = m_engine->query("inputpanel");
    m_inputpanel->setShowAux(data["AuxVisible"].toBool());
    m_inputpanel->setShowPreedit(data["PreeditVisible"].toBool());
    m_inputpanel->setLookupTableCursor(data["LookupTableCursor"].toInt());
    m_inputpanel->setLookupTableLayout(data["LookupTableLayout"].toInt());
    m_inputpanel->setShowLookupTable(data["LookupTableVisible"].toBool());
    m_inputpanel->setAuxText(data["AuxText"].toString());
    m_inputpanel->setPreeditText(data["PreeditText"].toString());
    m_inputpanel->setPreeditCaret(data["CaretPos"].toInt());
    QStringList label;
    QStringList text;
    QList<QVariant> lookupTable = data["LookupTable"].toList();
    Q_FOREACH(const QVariant & item, lookupTable) {
        label << item.toMap()["label"].toString();
        text << item.toMap()["text"].toString();
    }
    m_inputpanel->setLookupTable(
        label,
        text,
        data["HasPrev"].toBool(),
        data["HasNext"].toBool());
    m_inputpanel->setSpotLocation(data["Position"].toRect());
    m_inputpanel->updateSizeVisibility();
}

void Kimpanel::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    if (source == "inputpanel") {
        if (!m_inputPanelTimer.isActive())
            m_inputPanelTimer.start();
    } else if (source == "statusbar") {
        if (!m_statusBarTimer.isActive()) {
            m_statusBarTimer.start();
        }
        updateStatusBar();
        if (data["Menu"].isValid()) {
            QMap< QString, QVariant > map = data["Menu"].toMap();
            quint64 timestamp = map["timestamp"].toULongLong();
            if (timestamp > m_menuTimeStamp) {
                m_menuTimeStamp = timestamp;
                m_statusbar->execMenu(map["props"]);
            }
        }
    }
}

void Kimpanel::lookupTablePageUp()
{
    if (m_inputpanelService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("LookupTablePageUp");
        m_inputpanelService->startOperationCall(arg);
    }
}

void Kimpanel::lookupTablePageDown()
{
    if (m_inputpanelService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("LookupTablePageDown");
        m_inputpanelService->startOperationCall(arg);
    }
}

void Kimpanel::selectCandidate(int index)
{
    if (m_inputpanelService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("SelectCandidate");
        arg.writeEntry("candidate", index);
        m_inputpanelService->startOperationCall(arg);
    }
}

void Kimpanel::triggerProperty(const QString& property)
{
    if (m_statusbarService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("TriggerProperty");
        arg.writeEntry("key", property);
        m_statusbarService->startOperationCall(arg);
    }
}

QList<QAction*> Kimpanel::contextualActions()
{
    return m_statusbar->contextualActions();
}

void Kimpanel::configure()
{
    if (m_statusbarService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("Configure");
        m_statusbarService->startOperationCall(arg);
    }
}

void Kimpanel::reloadConfig()
{
    if (m_statusbarService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("ReloadConfig");
        m_statusbarService->startOperationCall(arg);
    }
}

void Kimpanel::exitIM()
{
    if (m_statusbarService) {
        KConfigGroup arg = m_inputpanelService->operationDescription("Exit");
        m_statusbarService->startOperationCall(arg);
    }
}

void Kimpanel::startIM()
{
    KUrl url = KimpanelSettings::self()->inputMethodLauncher();
    if (url.isLocalFile() && KDesktopFile::isDesktopFile(url.toLocalFile())) {
        new KRun(url, 0);
    } else {
        KService::Ptr service;
        KOpenWithDialog owdlg;
        if (owdlg.exec() != QDialog::Accepted)
            return;
        service = owdlg.service();
        if (service && service->isApplication()) {
            KUrl url(service->entryPath());
            if (url.isLocalFile() && KDesktopFile::isDesktopFile(url.toLocalFile())) {
                KimpanelSettings::self()->setInputMethodLauncher(url);
                KimpanelSettings::self()->writeConfig();
                new KRun(url, 0);
            }
        }
    }
}

void Kimpanel::selectIM()
{
    KService::Ptr service;
    KOpenWithDialog owdlg;
    if (owdlg.exec() != QDialog::Accepted)
        return;
    service = owdlg.service();
    if (service && service->isApplication()) {
        KUrl url(service->entryPath());
        if (url.isLocalFile() && KDesktopFile::isDesktopFile(url.toLocalFile())) {
            KimpanelSettings::self()->setInputMethodLauncher(url);
        }
    }
}

#include "moc_kimpanel.cpp"
