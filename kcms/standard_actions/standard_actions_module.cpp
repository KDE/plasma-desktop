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
#include "standard_actions_module.h"


#include <KAboutData>
#include <QAction>
#include <KActionCollection>
#include <KConfigGroup>
#include <QDebug>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KShortcutsEditor>
#include <KStandardAction>
#include <KMessageBox>
#include <KLocalizedString>

#include <QVBoxLayout>
#include <QSet>

K_PLUGIN_FACTORY(StandardActionsModuleFactory, registerPlugin<StandardActionsModule>();)
K_EXPORT_PLUGIN(StandardActionsModuleFactory("kcm_standard_actions"))

static void dressUpAction(QAction *action, KStandardShortcut::StandardShortcut shortcutId)
    {
    // Remember the shortcutId so we know where to save changes.
    action->setData(shortcutId);
    // We have to manually adjust the action. We want to show the
    // hardcoded default and the user set shortcut. But action currently
    // only contain the active shortcuts as default shortcut. So we
    // have to fill it correctly
    const auto hardcoded = KStandardShortcut::hardcodedDefaultShortcut(shortcutId);
    const auto active    = KStandardShortcut::shortcut(shortcutId);

    auto shortcuts = active;
    shortcuts << hardcoded;
    // Set the hardcoded default shortcut as default shortcut
    action->setShortcuts(shortcuts);
    }

StandardActionsModule::StandardActionsModule(
        QWidget *parent,
        const QVariantList &args )
    : KCModule(parent, args )
      ,m_editor(NULL)
      ,m_actionCollection(NULL)
    {
    KAboutData *about = new KAboutData(QStringLiteral("kcm_standard_actions"), i18n("Standard Shortcuts"), QStringLiteral("0.1"), QString(), KAboutLicense::GPL);
    setAboutData(about);

    // Configure the KCM
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply | KCModule::Help));

    // Create and configure the editor
    m_editor = new KShortcutsEditor(this, KShortcutsEditor::WidgetAction | KShortcutsEditor::WindowAction | KShortcutsEditor::ApplicationAction); // there will be no global actions, so make sure that column is hidden
    connect(m_editor, SIGNAL(keyChange()), this, SLOT(keyChanged()));

    // Make a layout
    QVBoxLayout *global = new QVBoxLayout;
    global->addWidget(m_editor);
    setLayout(global);
    }


StandardActionsModule::~StandardActionsModule()
    {}


void StandardActionsModule::defaults()
    {
    m_editor->allDefault();
    }


void StandardActionsModule::keyChanged()
    {
    emit changed(true);
    }


void StandardActionsModule::load()
    {
    // Create a collection to handle the shortcuts
    m_actionCollection = new KActionCollection(
            this,
            QStringLiteral("kcm_standard_actions"));

    // Keeps track of which shortcut IDs have been added
    QSet<int> shortcutIdsAdded;

    // Put all shortcuts for standard actions into the collection
    Q_FOREACH(KStandardAction::StandardAction id, KStandardAction::actionIds())
        {
        KStandardShortcut::StandardShortcut shortcutId = KStandardAction::shortcutForActionId(id);
        // If the StandardShortcutId is AccelNone skip configuration for this
        // action.
        if (shortcutId == KStandardShortcut::AccelNone || shortcutIdsAdded.contains(shortcutId))
            {
            continue;
            }
        // Create the action
        QAction *action = KStandardAction::create(id, NULL, NULL, m_actionCollection);
        dressUpAction(action, shortcutId);
        shortcutIdsAdded << shortcutId;
        }

    // Put in the remaining standard shortcuts too...
    for(int i = int(KStandardShortcut::AccelNone) + 1; i < KStandardShortcut::StandardShortcutCount; ++i)
        {
        KStandardShortcut::StandardShortcut shortcutId = static_cast<KStandardShortcut::StandardShortcut>(i);
        if(!shortcutIdsAdded.contains(shortcutId))
            {
            QAction *action = new QAction(KStandardShortcut::label(shortcutId), this);
            action->setWhatsThis(KStandardShortcut::whatsThis(shortcutId));
            dressUpAction(action, shortcutId);
            m_actionCollection->addAction(KStandardShortcut::name(shortcutId), action);
            }
        }

    // Hand the collection to the editor
    m_editor->addCollection(m_actionCollection, i18n("Standard Shortcuts"));
    }


void StandardActionsModule::save()
    {
    m_editor->commit();

    Q_FOREACH(QAction* action, m_actionCollection->actions())
        {
        KStandardShortcut::saveShortcut(
                static_cast<KStandardShortcut::StandardShortcut>(action->data().toInt())
                , action->shortcuts());
        }

    KSharedConfig::openConfig()->sync();
    KConfigGroup cg(KSharedConfig::openConfig(), "Shortcuts");
    cg.sync();

    QString title = i18n("Standard Actions successfully saved");
    QString message = i18n(
        "The changes have been saved. Please note that:"
        "<ul><li>Applications need to be restarted to see the changes.</li>"
        "    <li>This change could introduce shortcut conflicts in some applications.</li>"
        "</ul>" );
    KMessageBox::information(this, message, title, "shortcuts_saved_info");
    }

#include "standard_actions_module.moc"
