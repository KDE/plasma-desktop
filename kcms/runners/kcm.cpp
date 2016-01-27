/* This file is part of the KDE Project
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcm.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <QDebug>
#include <QStandardPaths>
#include <KLocalizedString>
#include <KRunner/RunnerManager>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KIconLoader>

#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPushButton>

K_PLUGIN_FACTORY(SearchConfigModuleFactory, registerPlugin<SearchConfigModule>();)


SearchConfigItemDelegate::SearchConfigItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    // From the calculation of 'size' in SearchConfigModule::load()
    m_margin = qRound(IconSize(KIconLoader::Panel) * 0.1);
}


void SearchConfigItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // from QStyledItemDelegate::paint()
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    opt.text.clear();					// draw everything else, but not text
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QRect textRect = option.rect;
    QRect styleRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    textRect.setLeft(styleRect.left());

    QString mainText = index.data(Qt::DisplayRole).toString();
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    textRect.translate(2 * m_margin, m_margin);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, mainText);

    // This is already available as option.fontMetrics, but to be absolutely
    // precise we use the metrics for the bold font.
    QFontMetrics metrics(font);
    textRect.translate(0, metrics.lineSpacing());

    const int availableWidth = textRect.width() - style->pixelMetric(QStyle::PM_LayoutRightMargin, &option, widget);
    QString subText = option.fontMetrics.elidedText(index.data(SearchConfigModule::DescriptionRole).toString(),
                                                    Qt::ElideRight, availableWidth);
    font.setBold(false);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, subText);
}


SearchConfigModule::SearchConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , m_config(QStringLiteral("krunnerrc"))
    , m_configGroup(m_config.group("PlasmaRunnerManager"))
{
    KAboutData* about = new KAboutData(QStringLiteral("kcm_search"), i18n("Configure Search"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    setAboutData(about);
    setButtons(Apply | Default);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout(this);

    QLabel *label = new QLabel(i18n("Select the search plugins"));

    QPushButton *clearHistoryButton = new QPushButton(i18n("Clear History"));
    clearHistoryButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    connect(clearHistoryButton, &QPushButton::clicked, this, [this] {
        KConfigGroup generalConfig(m_config.group("General"));
        generalConfig.deleteEntry("history");
        generalConfig.sync();
    });

    headerLayout->addWidget(label);
    headerLayout->addStretch();
    headerLayout->addWidget(clearHistoryButton);

    m_listWidget = new QListWidget(this);
    m_listWidget->setSortingEnabled(true);
    m_listWidget->setMouseTracking(true);
    m_listWidget->setItemDelegate(new SearchConfigItemDelegate(this));
    int size = IconSize(KIconLoader::Panel);
    m_listWidget->setIconSize(QSize(size, size));
    connect(m_listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(changed()));

    layout->addLayout(headerLayout);
    layout->addWidget(m_listWidget);

    m_configButton = new QToolButton(m_listWidget->viewport());
    m_configButton->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    m_configButton->resize(m_configButton->height(), m_configButton->height());
    m_configButton->hide();
    size = IconSize(KIconLoader::Small);
    m_configButton->setIconSize(QSize(size, size));

    connect(m_listWidget, &QListWidget::itemEntered, [=](QListWidgetItem * item) {
        QList<Plasma::AbstractRunner *> runners = item->data(RunnersRole).value<QList<Plasma::AbstractRunner *> >();

        if (!runners.isEmpty()) {
            const QRect rect = m_listWidget->visualItemRect(item);
            m_configButton->move(QPoint(rect.right(), rect.center().y()) - QPoint(m_configButton->width(), m_configButton->height()/2));
            m_configButton->setVisible(true);
            m_configButton->setProperty("runners", QVariant::fromValue(runners));
        } else {
            m_configButton->setVisible(false);
        }
    });

    connect (m_configButton, &QToolButton::clicked, this, &SearchConfigModule::configureClicked);

    Plasma::RunnerManager *manager = new Plasma::RunnerManager(this);
    manager->reloadConfiguration();
    Q_FOREACH (Plasma::AbstractRunner *runner, manager->runners()) {
        Q_FOREACH (const QString& category, runner->categories()) {
            m_runnerCategories.insert(category, runner);
        }
    }
}

void SearchConfigModule::configureClicked()
{
    QDialog configDialog(m_listWidget);

    // The number of KCModuleProxies in use determines whether to use a tabwidget
    QTabWidget *newTabWidget = 0;
    // Widget to use for the setting dialog's main widget,
    // either a QTabWidget or a KCModuleProxy
    QWidget *mainWidget = 0;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = &configDialog;

    QList<KCModuleProxy *> moduleProxyList;

    QList<Plasma::AbstractRunner *> runners = m_configButton->property("runners").value<QList<Plasma::AbstractRunner *> >();

    Q_FOREACH (Plasma::AbstractRunner *runner, runners) {
        const KPluginInfo pluginInfo = runner->metadata();
        Q_FOREACH (const KService::Ptr &servicePtr, pluginInfo.kcmServices()) {
            if (!servicePtr->noDisplay()) {
                KCModuleInfo moduleInfo(servicePtr);
                KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleInfo, moduleProxyParentWidget);
                if (currentModuleProxy->realModule()) {
                    moduleProxyList << currentModuleProxy;
                    if (mainWidget && !newTabWidget) {
                        // we already created one KCModuleProxy, so we need a tab widget.
                        // Move the first proxy into the tab widget and ensure this and subsequent
                        // proxies are in the tab widget
                        newTabWidget = new QTabWidget(&configDialog);
                        moduleProxyParentWidget = newTabWidget;
                        mainWidget->setParent(newTabWidget);
                        KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy *>(mainWidget);
                        if (moduleProxy) {
                            newTabWidget->addTab(mainWidget, moduleProxy->moduleInfo().moduleName());
                            mainWidget = newTabWidget;
                        } else {
                            delete newTabWidget;
                            newTabWidget = 0;
                            moduleProxyParentWidget = &configDialog;
                            mainWidget->setParent(0);
                        }
                    }

                    if (newTabWidget) {
                        newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                    } else {
                        mainWidget = currentModuleProxy;
                    }
                } else {
                    delete currentModuleProxy;
                }
            }
        }
    }

    // it could happen that we had services to show, but none of them were real modules.
    if (moduleProxyList.isEmpty()) {
        return;
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainWidget);
    const int marginHint = configDialog.style()->pixelMetric(QStyle::PM_DefaultChildMargin);
    layout->insertSpacing(-1, marginHint);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(&configDialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &configDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &configDialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    configDialog.setLayout(layout);

    if (configDialog.exec() == QDialog::Accepted) {
        foreach (KCModuleProxy *moduleProxy, moduleProxyList) {
            moduleProxy->save();
        }
    }

    qDeleteAll(moduleProxyList);
}

void SearchConfigModule::load()
{
    m_listWidget->clear();

    QStringList enabledCategories = m_configGroup.readEntry("enabledCategories", QStringList());

    QSet<QString> addedCategories;
    KPluginInfo::List list = Plasma::RunnerManager::listRunnerInfo();
    Q_FOREACH (const KPluginInfo& info, list) {
        QVariantList args;
        args << info.service()->storageId();

        Plasma::AbstractRunner* r = info.service()->createInstance<Plasma::AbstractRunner>(this, args);
        QScopedPointer<Plasma::AbstractRunner> runner(r);
        if (runner.isNull()) {
            continue;
        }

        QStringList categories = runner->categories();
        QString name = runner->name();

        Q_FOREACH (const QString& category, categories) {
            if (addedCategories.contains(category)) {
                continue;
            }

            QList<Plasma::AbstractRunner *> runnersWithConfig;
            Q_FOREACH (Plasma::AbstractRunner *runner, m_runnerCategories.values(category)) {
                if (!runner->metadata().kcmServices().isEmpty()) {
                    runnersWithConfig << runner;
                }
            }

            QListWidgetItem* item = new QListWidgetItem(category, m_listWidget);
            item->setIcon(runner->categoryIcon(category));

            bool enabled = enabledCategories.isEmpty() || enabledCategories.contains(category);
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

            item->setData(RunnersRole, QVariant::fromValue(runnersWithConfig));
            item->setData(DescriptionRole, runner->description());

            m_listWidget->addItem(item);
            const int size = qRound(IconSize(KIconLoader::Panel) * 1.2);
            item->setSizeHint(QSize(size, size));
            addedCategories.insert(category);
        }
    }
}


void SearchConfigModule::save()
{
    QStringList enabledCategories;
    for (int i = 0; i < m_listWidget->count(); i++) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString category = item->text();

        if (item->checkState() == Qt::Checked) {
            enabledCategories << category;
        }
    }

    m_configGroup.writeEntry("enabledCategories", enabledCategories);
    m_configGroup.sync();
}

void SearchConfigModule::defaults()
{
}

#include "kcm.moc"
