/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2011 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "backendselection.h"

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QListWidget>

#include <KCModuleProxy>
#include <KConfig>
#include <KDE/KApplication>
#include <KDE/KIcon>
#include <KDE/KIconLoader>
#include <KDE/KRun>
#include <KDE/KServiceTypeProfile>
#include <KDE/KServiceTypeTrader>
#include <KLocalizedString>

BackendSelection::BackendSelection(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    m_messageWidget->setVisible(false);
    m_messageWidget->setCloseButtonVisible(false);
    m_messageWidget->setMessageType(KMessageWidget::Information);
    m_messageWidget->setText(i18nc("@info User changed Phonon backend",
                                   "To apply the backend change you will have "
                                   "to log out and back in again."));

    m_down->setIcon(KIcon("go-down"));
    m_up->setIcon(KIcon("go-up"));
    m_comment->setWordWrap(true);

    m_emptyPage = stackedWidget->addWidget(new QWidget());

    connect(m_select, SIGNAL(itemSelectionChanged()),
            SLOT(selectionChanged()));
    //connect(m_website, SIGNAL(leftClickedUrl(const QString  &)),
            //kapp, SLOT(invokeBrowser(const QString  &)));
    connect(m_up, SIGNAL(clicked()), SLOT(up()));
    connect(m_down, SIGNAL(clicked()), SLOT(down()));
}

void BackendSelection::load()
{
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
    // the offers are already sorted for preference
    loadServices(offers);
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->load();
        }
    }
}

void BackendSelection::showBackendKcm(const KService::Ptr &backendService)
{
    const QString parentComponent = backendService->library();
    if (!m_kcms.contains(parentComponent)) {
        const KService::List offers = KServiceTypeTrader::self()->query("KCModule",
                QString("'%1' in [X-KDE-ParentComponents]").arg(parentComponent));
        if (offers.isEmpty()) {
            m_kcms.insert(parentComponent, 0);
        } else {
            KCModuleProxy *proxy = new KCModuleProxy(offers.first());
            connect(proxy, SIGNAL(changed(bool)), SIGNAL(changed()));
            m_kcms.insert(parentComponent, proxy);
            stackedWidget->addWidget(proxy);
        }
    }
    QWidget *w = m_kcms.value(parentComponent);
    if (w) {
        stackedWidget->show();
        stackedWidget->setCurrentWidget(w);
    } else {
        stackedWidget->hide();
        stackedWidget->setCurrentIndex(m_emptyPage);
    }
}

void BackendSelection::loadServices(const KService::List &offers)
{
    m_services.clear();
    m_select->clear();

    KService::List::const_iterator it = offers.begin();
    const KService::List::const_iterator end = offers.end();
    for (; it != end; ++it)
    {
        KService::Ptr service = *it;
        m_select->addItem(service->name());
        m_services[service->name()] = service;
    }
    m_select->setItemSelected(m_select->item(0), true);
}

void BackendSelection::save()
{
    // save embedded KCMs
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->save();
        }
    }

    // save to servicetype profile
    KService::List services;
    unsigned int count = m_select->count();
    for (unsigned int i = 0; i < count; ++i) {
        QListWidgetItem *item = m_select->item(i);
        KService::Ptr service = m_services[item->text()];
        services.append(service);
    }

    // get the currently used list
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");

    // we have to compare the lists manually as KService::Ptr::operator== is not what we want for
    // comparison
    if (offers.size() == services.size()) {
        bool equal = true;
        for (int i = 0; i < offers.size(); ++i) {
            if (offers[i]->entryPath() != services[i]->entryPath()) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return;
        }
    }

    if (offers != services) {
        KServiceTypeProfile::writeServiceTypeProfile("PhononBackend", services);

        // If the user changed the backend order, show a message that they need to
        // log out and back in again to apply the change. This is because runtime
        // backend switching was considered not worth the effort to actually
        // maintain it (across backends).
        m_messageWidget->animatedShow();
    }
}

void BackendSelection::defaults()
{
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->defaults();
        }
    }

    loadServices(KServiceTypeTrader::self()->defaultOffers("PhononBackend"));
}

void BackendSelection::selectionChanged()
{
    KService::Ptr service;
    if (m_select->selectedItems().isEmpty()) {
        m_up->setEnabled(false);
        m_down->setEnabled(false);
    } else {
        const QListWidgetItem *const item = m_select->selectedItems().first();
        m_up->setEnabled(m_select->row(item) > 0);
        m_down->setEnabled(m_select->row(item) < m_select->count() - 1);
        service = m_services[item->text()];
        Q_ASSERT(service);

        // Have some icon other than "unknown" for backends which don't install an icon.
        QPixmap iconPixmap = KIconLoader::global()->loadIcon(service->icon(), KIconLoader::NoGroup, 128,
                                                             KIconLoader::DefaultState, QStringList(), 0L,
                                                             true /* return null */);
        if (iconPixmap.isNull())
            iconPixmap = KIconLoader::global()->loadIcon("preferences-desktop-sound", KIconLoader::NoGroup, 128);

        m_icon->setPixmap(iconPixmap);
        m_name->setText(QString());//service->name());
        m_comment->setText(service->comment());
        const QString website = service->property("X-KDE-PhononBackendInfo-Website").toString();
        m_website->setText(QString("<a href=\"%1\">%1</a>").arg(website));
        connect(m_website, SIGNAL(linkActivated(QString)), SLOT(openWebsite(QString)), Qt::UniqueConnection);
        m_version->setText(service->property("X-KDE-PhononBackendInfo-Version").toString());
        showBackendKcm(service);
    }
}

void BackendSelection::openWebsite(const QString &url)
{
    new KRun(QUrl(url), window());
}

void BackendSelection::up()
{
    QList<QListWidgetItem *> selectedList = m_select->selectedItems();
    foreach (QListWidgetItem *selected, selectedList) {
        const int row = m_select->row(selected);
        if (row > 0) {
            QListWidgetItem *taken = m_select->takeItem(row - 1);
            m_select->insertItem(row, taken);
            emit changed();
            selectionChanged();
        }
    }
}

void BackendSelection::down()
{
    QList<QListWidgetItem *> selectedList = m_select->selectedItems();
    foreach (QListWidgetItem *selected, selectedList) {
        const int row = m_select->row(selected);
        if (row + 1 < m_select->count()) {
            QListWidgetItem *taken = m_select->takeItem(row + 1);
            m_select->insertItem(row, taken);
            emit changed();
            selectionChanged();
        }
    }
}

#include "backendselection.moc"

// vim: sw=4 ts=4
