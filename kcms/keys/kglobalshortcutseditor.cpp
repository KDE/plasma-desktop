/*
 *  Copyright 2008 Michael Jansen <kde@michael-jansen.biz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "kglobalshortcutseditor.h"

#include "ui_kglobalshortcutseditor.h"
#include "ui_select_application.h"
#include "export_scheme_dialog.h"
#include "select_scheme_dialog.h"
#include "globalshortcuts.h"
#include "kglobalaccel_interface.h"
#include "kglobalaccel_component_interface.h"

#include <KActionCollection>
#include <KConfig>
#include <QDebug>
#include <KGlobalAccel>
#include <KIconLoader>
#include <KMessageBox>
#include <KStringHandler>
#include <KLocalizedString>
#include <KService>
#include <KRecursiveFilterProxyModel>
#include <KServiceGroup>
#include <KDesktopFile>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>

#include <QStackedWidget>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QHash>

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>

#include <QFileDialog>

/*
 * README
 *
 * This class was created because the kshortcutseditor class has some shortcomings. That class uses
 * QTreeWidget and therefore makes it impossible for an outsider to switch the models. But the
 * global shortcuts editor did that. Each global component ( kded, krunner, kopete ... ) was
 * destined to be separately edited. If you selected another component the kshortcutseditor was
 * cleared and refilled. But the items take care of undoing. Therefore when switching the component
 * you lost the undo history.
 *
 * To solve that problem this class keeps one kshortcuteditor for each component. That is easier
 * than rewrite that dialog to a model/view framework.
 *
 * It perfectly covers a bug of KExtedableItemDelegate when clearing and refilling the associated
 * model.
 */

class ComponentData
    {

public:

    ComponentData(
            const QString &uniqueName,
            const QDBusObjectPath &path,
            KShortcutsEditor *_editor);

    ~ComponentData();

    QString uniqueName() const;
    KShortcutsEditor *editor();
    QDBusObjectPath dbusPath();

private:

    QString _uniqueName;
    QDBusObjectPath _path;
    QPointer<KShortcutsEditor> _editor;
    };


ComponentData::ComponentData(
        const QString &uniqueName,
        const QDBusObjectPath &path,
        KShortcutsEditor *editor)
    :   _uniqueName(uniqueName),
        _path(path),
        _editor(editor)
    {}


ComponentData::~ComponentData()
    {
    delete _editor; _editor = nullptr;
    }


QString ComponentData::uniqueName() const
    {
    return _uniqueName;
    }


QDBusObjectPath ComponentData::dbusPath()
    {
    return _path;
    }


KShortcutsEditor *ComponentData::editor()
    {
    return _editor;
    }


class KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate
{
public:

    KGlobalShortcutsEditorPrivate(KGlobalShortcutsEditor *q)
        :   q(q),
            bus(QDBusConnection::sessionBus())
    {}

    //! Setup the gui
    void initGUI();

    //! Load the component at @a componentPath
    bool loadComponent(const QDBusObjectPath &componentPath);

    //! Return the componentPath for component
    QDBusObjectPath componentPath(const QString &componentUnique);

    //! Remove the component
    void removeComponent(const QString &componentUnique);

    KGlobalShortcutsEditor *q;
    Ui::KGlobalShortcutsEditor ui;
    Ui::SelectApplicationDialog selectApplicationDialogUi;
    QDialog *selectApplicationDialog = nullptr;
    QStackedWidget *stack = nullptr;
    KShortcutsEditor::ActionTypes actionTypes;
    QHash<QString, ComponentData*> components;
    QDBusConnection bus;
    QStandardItemModel *model = nullptr;
    KCategorizedSortFilterProxyModel *proxyModel = nullptr;
};

void loadAppsCategory(KServiceGroup::Ptr group, QStandardItemModel *model, QStandardItem *item)
{
    if (group && group->isValid()) {
        KServiceGroup::List list = group->entries();

        for( KServiceGroup::List::ConstIterator it = list.constBegin();
             it != list.constEnd(); ++it) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KService)) {
                const KService::Ptr service(static_cast<KService*>(p.data()));

                if (!service->noDisplay()) {
                    QString genericName = service->genericName();
                    if (genericName.isNull()) {
                        genericName = service->comment();
                    }
                    QString description;
                    if (!service->genericName().isEmpty() && service->genericName() != service->name()) {
                        description = service->genericName();
                    } else if (!service->comment().isEmpty()) {
                        description = service->comment();
                    }

                    QStandardItem *subItem = new QStandardItem(QIcon::fromTheme(service->icon()), service->name());
                    subItem->setData(service->entryPath());
                    if (item) {
                        item->appendRow(subItem);
                    } else {
                        model->appendRow(subItem);
                    }
                }

            } else if (p->isType(KST_KServiceGroup)) {
                KServiceGroup::Ptr subGroup(static_cast<KServiceGroup*>(p.data()));

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    if (item) {
                        loadAppsCategory(subGroup, model, item);
                    } else {
                        QStandardItem *subItem = new QStandardItem(QIcon::fromTheme(subGroup->icon()), subGroup->caption());
                        model->appendRow(subItem);
                        loadAppsCategory(subGroup, model, subItem);
                    }
                }
            }
        }
    }
}

void KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::initGUI()
{
    ui.setupUi(q);
    selectApplicationDialog = new QDialog();
    selectApplicationDialogUi.setupUi(selectApplicationDialog);
    // Create a stacked widget.
    stack = new QStackedWidget(q);
    ui.currentComponentLayout->addWidget(stack);
    //HACK to make those two un-alignable components, aligned
    ui.componentLabel->setMinimumHeight(ui.lineEditSpacer->sizeHint().height());
    ui.lineEditSpacer->setVisible(false);
    ui.addButton->setIcon(QIcon::fromTheme("list-add"));
    ui.removeButton->setIcon(QIcon::fromTheme("list-remove"));
    ui.components->setCategoryDrawer(new KCategoryDrawer(ui.components));
    ui.components->setModelColumn(0);

    // Build the menu
    QMenu *menu = new QMenu(q);
    menu->addAction( QIcon::fromTheme(QStringLiteral("document-import")), i18n("Import Scheme..."), q, SLOT(importScheme()));
    menu->addAction( QIcon::fromTheme(QStringLiteral("document-export")), i18n("Export Scheme..."), q, SLOT(exportScheme()));
    menu->addAction( i18n("Set All Shortcuts to None"), q, SLOT(clearConfiguration()));
    
    connect(ui.addButton, &QToolButton::clicked, [this]() {
        if (!selectApplicationDialogUi.treeView->model()) {
            KRecursiveFilterProxyModel *filterModel = new KRecursiveFilterProxyModel(selectApplicationDialogUi.treeView);
            QStandardItemModel *appModel = new QStandardItemModel(selectApplicationDialogUi.treeView);
            selectApplicationDialogUi.kfilterproxysearchline->setProxy(filterModel);
            filterModel->setSourceModel(appModel);
            appModel->setHorizontalHeaderLabels({i18n("Applications")});

            loadAppsCategory(KServiceGroup::root(), appModel, nullptr);

            selectApplicationDialogUi.treeView->setModel(filterModel);
        }
        selectApplicationDialog->show();
    });

    connect(selectApplicationDialog, &QDialog::accepted, [this]() {
        if (selectApplicationDialogUi.treeView->selectionModel()->selectedIndexes().length() == 1) {
            const QString desktopPath = selectApplicationDialogUi.treeView->model()->data(selectApplicationDialogUi.treeView->selectionModel()->selectedIndexes().first(), Qt::UserRole+1).toString();

            if (!desktopPath.isEmpty() &&QFile::exists(desktopPath) ) {
                const QString desktopFile = desktopPath.split(QLatin1Char('/')).last();

                if (!desktopPath.isEmpty()) {
                    KDesktopFile sourceDF(desktopPath);
                    KDesktopFile *destinationDF = sourceDF.copyTo(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kglobalaccel/") + desktopFile);
                    qWarning()<<QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kglobalaccel/") + desktopFile;
                    destinationDF->sync();
                    //TODO: a DBUS call to tell the daemon to refresh desktop files


                    // Create a action collection for our current component:context
                    KActionCollection *col = new KActionCollection(q, desktopFile);

                    foreach(const QString &actionId, sourceDF.readActions()) {

                        const QString friendlyName = sourceDF.actionGroup(actionId).readEntry(QStringLiteral("Name"));
                        QAction *action = col->addAction(actionId);
                        action->setProperty("isConfigurationAction", QVariant(true)); // see KAction::~KAction
                        action->setProperty("componentDisplayName", friendlyName);
                        action->setText(friendlyName);

                        KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>());
                        
                        QStringList sequencesStrings = sourceDF.actionGroup(actionId).readEntry(QStringLiteral("X-KDE-Shortcuts"), QString()).split(QLatin1Char('/'));
                        QList<QKeySequence> sequences;
                        if (!sequencesStrings.isEmpty()) {
                            Q_FOREACH (const QString &seqString, sequencesStrings) {
                                sequences.append(QKeySequence(seqString));
                            }
                        }

                        if (!sequences.isEmpty()) {
                            KGlobalAccel::self()->setDefaultShortcut(action, sequences);
                        }
                    }
                    //Global launch action
                    {
                        const QString friendlyName = i18n("Launch %1", sourceDF.readName());
                        QAction *action = col->addAction(QStringLiteral("_launch"));
                        action->setProperty("isConfigurationAction", QVariant(true)); // see KAction::~KAction
                        action->setProperty("componentDisplayName", friendlyName);
                        action->setText(friendlyName);

                        KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>());
                        
                        QStringList sequencesStrings = sourceDF.desktopGroup().readEntry(QStringLiteral("X-KDE-Shortcuts"), QString()).split(QLatin1Char('/'));
                        QList<QKeySequence> sequences;
                        if (!sequencesStrings.isEmpty()) {
                            Q_FOREACH (const QString &seqString, sequencesStrings) {
                                sequences.append(QKeySequence(seqString));
                            }
                        }

                        if (!sequences.isEmpty()) {
                            KGlobalAccel::self()->setDefaultShortcut(action, sequences);
                        }
                    }
                    q->addCollection(col, QDBusObjectPath(), desktopFile, sourceDF.readName());
                }
            }
        }
    });

    connect(ui.removeButton, &QToolButton::clicked, [this]() {
        //TODO: different way to remove components that are desktop files
        //disabled desktop files need Hidden=true key
        QString name = proxyModel->data(ui.components->currentIndex()).toString();
        QString componentUnique = components.value(name)->uniqueName();

        // The confirmation text is different when the component is active
        if (KGlobalAccel::isComponentActive(componentUnique)) {
            if (KMessageBox::questionYesNo(
                        q,
                        i18n("Component '%1' is currently active. Only global shortcuts currently not active will be removed from the list.\n"
                            "All global shortcuts will reregister themselves with their defaults when they are next started.", componentUnique),
                        i18n("Remove component")) != KMessageBox::Yes) {
                return;
            }
        } else {
            if (KMessageBox::questionYesNo(
                        q,
                        i18n("Are you sure you want to remove the registered shortcuts for component '%1'? "
                            "The component and shortcuts will reregister themselves with their default settings"
                            " when they are next started.",
                            componentUnique),
                        i18n("Remove component")) != KMessageBox::Yes) {
                return;
            }
        }

        // Initiate the removing of the component.
        if (KGlobalAccel::cleanComponent(componentUnique)) {

            // Get the objectPath BEFORE we delete the source of it
            QDBusObjectPath oPath = components.value(name)->dbusPath();
            // Remove the component from the gui
            removeComponent(componentUnique);

            // Load it again
            // #############
            if (loadComponent(oPath)) {
                // Active it
                q->activateComponent(name);
            }
        }
    });

    ui.menu_button->setMenu(menu);

    proxyModel = new KCategorizedSortFilterProxyModel(q);
    proxyModel->setCategorizedModel(true);
    model = new QStandardItemModel(0, 1, proxyModel);
    proxyModel->setSourceModel(model);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui.components->setModel(proxyModel);

    connect(ui.components->selectionModel(),
        &QItemSelectionModel::currentChanged,
        q, [this](const QModelIndex &index) {
            QString name = proxyModel->data(index).toString();
            q->activateComponent(name);
        });
}


KGlobalShortcutsEditor::KGlobalShortcutsEditor(QWidget *parent, KShortcutsEditor::ActionTypes actionTypes)
    :   QWidget(parent),
        d(new KGlobalShortcutsEditorPrivate(this))
{
    d->actionTypes = actionTypes;
    // Setup the ui
    d->initGUI();
}


KGlobalShortcutsEditor::~KGlobalShortcutsEditor()
{
    // Before closing the door, undo all changes
    undo();
    delete d->selectApplicationDialog;
    qDeleteAll(d->components);
    delete d;
}


void KGlobalShortcutsEditor::activateComponent(const QString &component)
{
    QHash<QString, ComponentData*>::Iterator iter = d->components.find(component);
    if (iter == d->components.end()) {
        Q_ASSERT(iter != d->components.end());
        return;
    } else {
        QModelIndexList results = d->proxyModel->match(d->proxyModel->index(0, 0), Qt::DisplayRole, component);
        Q_ASSERT(!results.isEmpty());
        if (results.first().isValid()) {
            // Known component. Get it.
            d->ui.components->setCurrentIndex(results.first());
            d->stack->setCurrentWidget((*iter)->editor());
        }
    }
}


void KGlobalShortcutsEditor::addCollection(
        KActionCollection *collection,
        const QDBusObjectPath &objectPath,
        const QString &id,
        const QString &friendlyName)
{
    KShortcutsEditor *editor;
    // Check if this component is known
    QHash<QString, ComponentData*>::Iterator iter = d->components.find(friendlyName);
    if (iter == d->components.end()) {
        // Unknown component. Create an editor.
        editor = new KShortcutsEditor(this, d->actionTypes);
        d->stack->addWidget(editor);

        // try to find one appropriate icon ( allowing NULL pixmap to be returned)
        QPixmap pixmap = KIconLoader::global()->loadIcon(id, KIconLoader::Small, 0,
                                  KIconLoader::DefaultState, QStringList(), nullptr, true);
        if (pixmap.isNull()) {
            KService::Ptr service = KService::serviceByStorageId(id);
            if(service) {
                pixmap = KIconLoader::global()->loadIcon(service->icon(), KIconLoader::Small, 0,
                                  KIconLoader::DefaultState, QStringList(), nullptr, true);
            }
        }
        // if NULL pixmap is returned, use the F.D.O "system-run" icon
        if (pixmap.isNull()) {
            pixmap = KIconLoader::global()->loadIcon(QStringLiteral("system-run"), KIconLoader::Small);
        }

        // Add to the component list
        QStandardItem *item = new QStandardItem(pixmap, friendlyName);
        if (id.endsWith(QLatin1String(".desktop"))) {
            item->setData(i18n("Application Launchers"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
        } else {
            item->setData(i18n("Other Shortcuts"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            item->setData(1, KCategorizedSortFilterProxyModel::CategorySortRole);
        }
        d->model->appendRow(item);
        d->proxyModel->sort(0);

        // Add to our component registry
        ComponentData *cd = new ComponentData(id, objectPath, editor);
        d->components.insert(friendlyName, cd);

        connect(editor, &KShortcutsEditor::keyChange, this, &KGlobalShortcutsEditor::_k_key_changed);
    } else {
        // Known component.
        editor = (*iter)->editor();
    }

    // Add the collection to the editor of the component
    editor->addCollection(collection, friendlyName);

    if (d->proxyModel->rowCount() > -1) {
        d->ui.components->setCurrentIndex(d->proxyModel->index(0, 0));
        QString name = d->proxyModel->data(d->proxyModel->index(0, 0)).toString();
        activateComponent(name);
    }
}


void KGlobalShortcutsEditor::clearConfiguration()
{
    QString name = d->proxyModel->data(d->ui.components->currentIndex()).toString();
    d->components[name]->editor()->clearConfiguration();
}


void KGlobalShortcutsEditor::defaults(ComponentScope scope)
{
    switch (scope)
        {
        case AllComponents:
            Q_FOREACH (ComponentData *cd, d->components) {
                // The editors are responsible for the reset
                cd->editor()->allDefault();
            }
            break;

        case CurrentComponent: {
            QString name = d->proxyModel->data(d->ui.components->currentIndex()).toString();
            // The editors are responsible for the reset
            d->components[name]->editor()->allDefault();
            }
            break;

        default:
            Q_ASSERT(false);
        };
}


void KGlobalShortcutsEditor::clear()
{
    // Remove all components and their associated editors
    qDeleteAll(d->components);
    d->components.clear();
    d->model->clear();
}


static bool compare(const QString &a, const QString &b)
    {
    return a.toLower().localeAwareCompare(b.toLower()) < 0;
    }


void KGlobalShortcutsEditor::exportScheme()
{
    QStringList keys = d->components.keys();
    std::sort(keys.begin(), keys.end(), compare);
    ExportSchemeDialog dia(keys);

    if (dia.exec() != KMessageBox::Ok) {
        return;
    }

    const QUrl url = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), QStringLiteral("*.kksrc"));
    if (!url.isEmpty()) {
        KConfig config(url.path(), KConfig::SimpleConfig);
        // TODO: Bug ossi to provide a method for this
        Q_FOREACH(const QString &group, config.groupList())
            {
            // do not overwrite the Settings group. That makes it possible to
            // update the standard scheme kksrc file with the editor.
            if (group == QLatin1String("Settings")) continue;
            config.deleteGroup(group);
            }
        exportConfiguration(dia.selectedComponents(), &config);
    }
}


void KGlobalShortcutsEditor::importScheme()
{
    // Check for unsaved modifications
    if (isModified()) {
        int choice = KMessageBox::warningContinueCancel(
                         this,
                         i18n("Your current changes will be lost if you load another scheme before saving this one"),
                         i18n("Load Shortcut Scheme"),
                         KGuiItem(i18n("Load")));
        if (choice != KMessageBox::Continue) {
            return;
        }
    }

    SelectSchemeDialog dialog(this);
    if (dialog.exec() != KDialog::Accepted) {
        return;
    }

    QUrl url = dialog.selectedScheme();
    if (!url.isLocalFile()) {
        KMessageBox::sorry(this, i18n("This file (%1) does not exist. You can only select local files.",
                           url.url()));
        return;
    }
    KConfig config(url.path(), KConfig::SimpleConfig);
    importConfiguration(&config);
}


void KGlobalShortcutsEditor::load()
{
    // Connect to kglobalaccel. If that fails there is no need to continue.
    qRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo> >();
    qDBusRegisterMetaType<KGlobalShortcutInfo>();

    org::kde::KGlobalAccel kglobalaccel(
            QStringLiteral("org.kde.kglobalaccel"),
            QStringLiteral("/kglobalaccel"),
            d->bus);

    if (!kglobalaccel.isValid()) {
        QString errorString;
        QDBusError error = kglobalaccel.lastError();
        // The global shortcuts DBus service manages all global shortcuts and we
        // can't do anything useful without it.
        if (error.isValid()) {
            errorString = i18n("Message: %1\nError: %2", error.message(), error.name());
        }

        KMessageBox::sorry(
            this,
            i18n("Failed to contact the KDE global shortcuts daemon\n")
                + errorString );
        return;
    }

    // Undo all changes not yet applied
    undo();
    clear();

    QDBusReply< QList<QDBusObjectPath> > componentsRc = kglobalaccel.allComponents();
    if (!componentsRc.isValid())
        {
        // Sometimes error pop up only after the first real call.
        QString errorString;
        QDBusError error = componentsRc.error();
        // The global shortcuts DBus service manages all global shortcuts and we
        // can't do anything useful without it.
        if (error.isValid()) {
            errorString = i18n("Message: %1\nError: %2", error.message(), error.name());
        }

        KMessageBox::sorry(
            this,
            i18n("Failed to contact the KDE global shortcuts daemon\n")
                + errorString );
        return;
        }
    QList<QDBusObjectPath> components = componentsRc;

    Q_FOREACH(const QDBusObjectPath &componentPath, components) {
        d->loadComponent(componentPath);
    } // Q_FOREACH(component)
}


void KGlobalShortcutsEditor::save()
{
    // The editors are responsible for the saving
    //qDebug() << "Save the changes";
    Q_FOREACH (ComponentData *cd, d->components) {
        cd->editor()->commit();
    }
}


void KGlobalShortcutsEditor::importConfiguration(KConfigBase *config)
{
    //qDebug() << config->groupList();

    // In a first step clean out the current configurations. We do this
    // because we want to minimize the chance of conflicts.
    Q_FOREACH (ComponentData *cd, d->components) {
        KConfigGroup group(config, cd->uniqueName());
        //qDebug() << cd->uniqueName() << group.name();
        if (group.exists()) {
            //qDebug() << "Removing" << cd->uniqueName();
            cd->editor()->clearConfiguration();
        }
    }

    // Now import the new configurations.
    Q_FOREACH (ComponentData *cd, d->components) {
        KConfigGroup group(config, cd->uniqueName());
        if (group.exists()) {
            //qDebug() << "Importing" << cd->uniqueName();
            cd->editor()->importConfiguration(&group);
        }
    }
}

void KGlobalShortcutsEditor::exportConfiguration(QStringList components, KConfig *config) const
    {
    Q_FOREACH (const QString &componentFriendly, components)
        {
        QHash<QString, ComponentData*>::Iterator iter = d->components.find(componentFriendly);
        if (iter == d->components.end())
            {
            Q_ASSERT(iter != d->components.end());
            continue;
            }
        else
            {
            KConfigGroup group(config, (*iter)->uniqueName());
            (*iter)->editor()->exportConfiguration(&group);
            }
        }
    }


void KGlobalShortcutsEditor::undo()
{
    // The editors are responsible for the undo
    //qDebug() << "Undo the changes";
    Q_FOREACH (ComponentData *cd, d->components) {
        cd->editor()->undoChanges();
    }
}


bool KGlobalShortcutsEditor::isModified() const
{
    Q_FOREACH (ComponentData *cd, d->components) {
        if (cd->editor()->isModified()) {
            return true;
        }
    }
    return false;
}


void KGlobalShortcutsEditor::_k_key_changed()
{
    emit changed(isModified());
}


QDBusObjectPath KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::componentPath(const QString &componentUnique)
{
    return QDBusObjectPath(QStringLiteral("/component/") + componentUnique);
}


bool KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::loadComponent(const QDBusObjectPath &componentPath)
{
    // Get the component
    org::kde::kglobalaccel::Component component(
            QStringLiteral("org.kde.kglobalaccel"),
            componentPath.path(),
            bus);
    if (!component.isValid()) {
        //qDebug() << "Component " << componentPath.path() << "not valid! Skipping!";
        return false;
    }

    // Get the shortcut contexts.
    QDBusReply<QStringList> shortcutContextsRc = component.getShortcutContexts();
    if (!shortcutContextsRc.isValid()) {
        //qDebug() << "Failed to get contexts for component "
                 //<< componentPath.path() <<"! Skipping!";
        //qDebug() << shortcutContextsRc.error();
        return false;
    }
    QStringList shortcutContexts = shortcutContextsRc;

    // We add the shortcuts for all shortcut contexts to the editor. This
    // way the user keeps full control of it's shortcuts.
    Q_FOREACH (const QString &shortcutContext, shortcutContexts) {

        QDBusReply< QList<KGlobalShortcutInfo> > shortcutsRc =
            component.allShortcutInfos(shortcutContext);
        if (!shortcutsRc.isValid())
            {
            //qDebug() << "allShortcutInfos() failed for " << componentPath.path() << shortcutContext;
            continue;
            }
        QList<KGlobalShortcutInfo> shortcuts = shortcutsRc;
        // Shouldn't happen. But you never know
        if (shortcuts.isEmpty()) {
            //qDebug() << "Got shortcut context" << shortcutContext << "without shortcuts for"
                //<< componentPath.path();
            continue;
        }

        // It's safe now
        const QString componentUnique = shortcuts[0].componentUniqueName();
        QString componentContextId = componentUnique;
        // kglobalaccel knows that '|' is our separator between
        // component and context
        if (shortcutContext != QLatin1String("default")) {
            componentContextId += QStringLiteral("|") + shortcutContext;
        }

        // Create a action collection for our current component:context
        KActionCollection* col = new KActionCollection(
                q,
                componentContextId);

        // Now add the shortcuts.
        Q_FOREACH (const KGlobalShortcutInfo &shortcut, shortcuts) {

            const QString &objectName = shortcut.uniqueName();
            QAction *action = col->addAction(objectName);
            action->setProperty("isConfigurationAction", QVariant(true)); // see KAction::~KAction
            action->setProperty("componentDisplayName", shortcut.componentFriendlyName());
            action->setText(shortcut.friendlyName());

            // Always call this to enable global shortcuts for the action. The editor widget
            // checks it.
            // Also actually loads the shortcut using the KAction::Autoloading mechanism.
            // Avoid setting the default shortcut; it would just be written to the global
            // configuration so we would not get the real one below.
            KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>());

            // The default shortcut will never be loaded because it's pointless in a real
            // application. There are no scarce resources [i.e. physical keys] to manage
            // so applications can set them at will and there's no autoloading.
            QList<QKeySequence> sc = shortcut.defaultKeys();
            if (sc.count()>0) {
                KGlobalAccel::self()->setDefaultShortcut(action, sc);
            }
        } // Q_FOREACH(shortcut)

        QString componentFriendlyName = shortcuts[0].componentFriendlyName();

        if (shortcuts[0].contextUniqueName() != QLatin1String("default"))
            {
            componentFriendlyName +=
                QString('[') + shortcuts[0].contextFriendlyName() + QString(']');
            }

        q->addCollection(col, componentPath, componentContextId, componentFriendlyName );

    } // Q_FOREACH(context)

    return true;
}


void KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::removeComponent(
        const QString &componentUnique )
    {
    // TODO: Remove contexts too.

    Q_FOREACH (const QString &text, components.keys())
        {
        if (components.value(text)->uniqueName() == componentUnique)
            {
            // Remove from QComboBox
            QModelIndexList results = proxyModel->match(proxyModel->index(0, 0), Qt::DisplayRole, text);
            Q_ASSERT(!results.isEmpty());
            model->removeRow(proxyModel->mapToSource(results.first()).row());

            // Remove from QStackedWidget
            stack->removeWidget(components[text]->editor());

            // Remove the componentData
            delete components.take(text);
            }
        }
    }


