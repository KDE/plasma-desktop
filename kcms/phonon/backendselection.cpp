/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2011-2014 Harald Sitter <sitter@kde.org>

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

#include <KConfig>
#include <KIconLoader>
#include <KRun>
#include <KLocalizedString>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QListWidget>
#include <QPluginLoader>
#include <QSettings>

BackendDescriptor::BackendDescriptor(const QString &path)
    : isValid(false)
{
    QPluginLoader loader(path);

    iid = loader.metaData().value(QLatin1Literal("IID")).toString();

    const QJsonObject metaData = loader.metaData().value(QLatin1Literal("MetaData")).toObject();
    name = metaData.value(QLatin1Literal("Name")).toString();
    icon = metaData.value(QLatin1Literal("Icon")).toString();
    version = metaData.value(QLatin1Literal("Version")).toString();
    website = metaData.value(QLatin1Literal("Website")).toString();
    preference = metaData.value(QLatin1Literal("InitialPreference")).toDouble();

    pluginPath = path;

    if (name.isEmpty())
        name = QFileInfo(path).baseName();

    if (iid.isEmpty())
        return; // Not valid.

    isValid = true;
}

bool BackendDescriptor::operator <(const BackendDescriptor &rhs) const
{
    return this->preference < rhs.preference;
}

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

    m_down->setIcon(QIcon::fromTheme("go-down"));
    m_up->setIcon(QIcon::fromTheme("go-up"));
    m_comment->setWordWrap(true);

    m_emptyPage = stackedWidget->addWidget(new QWidget());

    connect(m_select, &QListWidget::itemSelectionChanged, this, &BackendSelection::selectionChanged);
    connect(m_up, &QToolButton::clicked, this, &BackendSelection::up);
    connect(m_down, &QToolButton::clicked, this, &BackendSelection::down);
}

void BackendSelection::load()
{
    // NOTE: for phonon5 this should move into the library in some form.
    m_backends.clear();

    // Read already configured order.
    QList<QString> iidPreference;
    QSettings settings("kde.org", "libphonon");
    const int size = settings.beginReadArray("Backends");
    qDebug() << settings.fileName();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        iidPreference.append(settings.value("iid").toString());
    }
    settings.endArray();

    const QLatin1Literal suffix("/" PHONON_LIB_SONAME "_backend/");
    const QStringList paths = QCoreApplication::libraryPaths();
    qDebug() << "libpaths" << paths;

    QList<struct BackendDescriptor> backendList;

    foreach (const QString &path, paths) {
        const QString libPath = path + suffix;
        const QDir dir(libPath);
        if (!dir.exists()) {
            qDebug() << Q_FUNC_INFO << dir.absolutePath() << "does not exist";
            continue;
        }

        QStringList plugins(dir.entryList(QDir::Files));

        foreach (const QString &plugin, plugins) {
            BackendDescriptor bd = BackendDescriptor(libPath + plugin);
            if (bd.isValid) {
                int preference = iidPreference.indexOf(bd.iid);
                if (preference != -1)
                    bd.preference = preference;
                backendList.append(bd);
            }
        }

        qSort(backendList);
    }

    /// -------------- LOAD

    m_select->clear();

    foreach (const struct BackendDescriptor &bd, backendList) {
        m_select->addItem(bd.name);
        m_backends.insert(bd.name, bd);
    }
    m_select->setItemSelected(m_select->item(0), true);
}

void BackendSelection::save()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "~~~~~~~~~~~~~~";
    QSettings settings("kde.org", "libphonon");
    settings.beginWriteArray("Backends", m_select->count());
    for (int i = 0; i < m_select->count(); ++i) {
        settings.setArrayIndex(i);
        const QListWidgetItem *item = m_select->item(i);
        const BackendDescriptor backend = m_backends.value(item->text());
        settings.setValue("iid", backend.iid);
    }
    settings.endArray();
    settings.sync();

    qDebug() << settings.fileName();
}

void BackendSelection::defaults()
{
    load();
}

void BackendSelection::selectionChanged()
{
    qDebug() << "qooooooooo";
    if (m_select->selectedItems().isEmpty()) {
        m_up->setEnabled(false);
        m_down->setEnabled(false);
    } else {
        const QListWidgetItem *const item = m_select->selectedItems().first();
        m_up->setEnabled(m_select->row(item) > 0);
        m_down->setEnabled(m_select->row(item) < m_select->count() - 1);
        BackendDescriptor backend = m_backends[item->text()];

        // Have some icon other than "unknown" for backends which don't install an icon.
        QIcon icon = QIcon::fromTheme(backend.icon);
        if (icon.isNull()) {
            QPixmap iconPixmap = KIconLoader::global()->loadIcon("preferences-desktop-sound", KIconLoader::NoGroup, 128);
            m_icon->setPixmap(iconPixmap);
        } else {
            m_icon->setPixmap(icon.pixmap(128, 128));
        }

        m_name->setText(backend.name);
        m_website->setText(QString("<a href=\"%1\">%1</a>").arg(backend.website));
        connect(m_website, &QLabel::linkActivated, this, &BackendSelection::openWebsite, Qt::UniqueConnection);
        m_version->setText(backend.version);
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
