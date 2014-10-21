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
    delete _editor; _editor = 0;
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
            stack(0),
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
    QStackedWidget *stack;
    KShortcutsEditor::ActionTypes actionTypes;
    QHash<QString, ComponentData*> components;
    QDBusConnection bus;
    QStandardItemModel *model;
};


void KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::initGUI()
{
    ui.setupUi(q);
    // Create a stacked widget.
    stack = new QStackedWidget(q);
    q->layout()->addWidget(stack);

    // Connect our components
    connect(ui.components, SIGNAL(activated(QString)),
            q, SLOT(activateComponent(QString)));

    // Build the menu
    QMenu *menu = new QMenu(q);
    menu->addAction( QIcon::fromTheme("document-import"), i18n("Import Scheme..."), q, SLOT(importScheme()));
    menu->addAction( QIcon::fromTheme("document-export"), i18n("Export Scheme..."), q, SLOT(exportScheme()));
    menu->addAction( i18n("Set All Shortcuts to None"), q, SLOT(clearConfiguration()));
    QAction *action = menu->addAction( QIcon::fromTheme("edit-delete"), i18n("Remove Component"));
    connect(action, &QAction::triggered, [this]() {
        QString name = ui.components->currentText();
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

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(q);
    model = new QStandardItemModel(0, 1, proxyModel);
    proxyModel->setSourceModel(model);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui.components->setModel(proxyModel);
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
        int index = d->ui.components->findText(component);
        Q_ASSERT(index != -1);
        if (index > -1) {
            // Known component. Get it.
            d->ui.components->setCurrentIndex(index);
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
                                  KIconLoader::DefaultState, QStringList(), 0, true);
        // if NULL pixmap is returned, use the F.D.O "system-run" icon
        if (pixmap.isNull()) {
            pixmap = KIconLoader::global()->loadIcon("system-run", KIconLoader::Small);
        }

        // Add to the component combobox
        //FIXME: QCombobox.addItem apparently breaks with sort() in Qt5
        d->model->appendRow(new QStandardItem(pixmap, friendlyName));
        d->ui.components->model()->sort(0);

        // Add to our component registry
        ComponentData *cd = new ComponentData(id, objectPath, editor);
        d->components.insert(friendlyName, cd);

        connect(editor, SIGNAL(keyChange()), this, SLOT(_k_key_changed()));
    } else {
        // Known component.
        editor = (*iter)->editor();
    }

    // Add the collection to the editor of the component
    editor->addCollection(collection, friendlyName);

    if (d->ui.components->count() > -1) {
        d->ui.components->setCurrentIndex(0);
        activateComponent(d->ui.components->itemText(0));
    }
}


void KGlobalShortcutsEditor::clearConfiguration()
{
    QString name = d->ui.components->currentText();
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
            QString name = d->ui.components->currentText();
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
    d->ui.components->clear();
}


static bool compare(const QString &a, const QString &b)
    {
    return a.toLower().localeAwareCompare(b.toLower()) < 0;
    }


void KGlobalShortcutsEditor::exportScheme()
{
    QStringList keys = d->components.keys();
    qSort(keys.begin(), keys.end(), compare);
    ExportSchemeDialog dia(keys);

    if (dia.exec() != KMessageBox::Ok) {
        return;
    }

    QUrl url = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), QStringLiteral("*.kksrc"));
    if (!url.isEmpty()) {
        KConfig config(url.path(), KConfig::SimpleConfig);
        // TODO: Bug ossi to provide a method for this
        Q_FOREACH(const QString &group, config.groupList())
            {
            // do not overwrite the Settings group. That makes it possible to
            // update the standard scheme kksrc file with the editor.
            if (group == "Settings") continue;
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
            "org.kde.kglobalaccel",
            "/kglobalaccel",
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
    return QDBusObjectPath(QLatin1String("/component/") + componentUnique);
}


bool KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::loadComponent(const QDBusObjectPath &componentPath)
{
    // Get the component
    org::kde::kglobalaccel::Component component(
            "org.kde.kglobalaccel",
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
        if (shortcutContext != "default") {
            componentContextId += QString("|") + shortcutContext;
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

        if (shortcuts[0].contextUniqueName() != "default")
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
            int index = ui.components->findText(text);
            Q_ASSERT(index != -1);
            ui.components->removeItem(index);

            // Remove from QStackedWidget
            stack->removeWidget(components[text]->editor());

            // Remove the componentData
            delete components.take(text);
            }
        }
    }


