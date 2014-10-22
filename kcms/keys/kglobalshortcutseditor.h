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
#ifndef KGLOBALSHORTCUTSEDITOR_H
#define KGLOBALSHORTCUTSEDITOR_H

#include <QWidget>

#include "kshortcutseditor.h"

class KActionCollection;
class KConfig;
class QDBusObjectPath;

/**
 * Combine a KShortcutsEditor with a KComboBox.
 *
 * @see KShortcutsEditor
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KGlobalShortcutsEditor : public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     *
     * @param parent parent widget
     */
    KGlobalShortcutsEditor(QWidget *parent,
                           KShortcutsEditor::ActionTypes actionTypes = KShortcutsEditor::AllActions);
    ~KGlobalShortcutsEditor();

    /**
     * Insert an action collection, i.e. add all it's actions to the ones already associated
     * with the KShortcutsEditor object.
     *
     * @param collection  the collection to add
     * @param component   title for the component
     * @param title       title for the subtree in the component
     */
    void addCollection(KActionCollection *, const QDBusObjectPath &path, const QString &id, const QString &name);

    /**
     * Clear all collections were currently hosting. Current changes are not
     * undone? Do that before calling this method.
     */
    void clear();


    /**
     * Revert all changes made since the last save.
     */
    void undo();


    /**
     * Load the shortcuts from the configuration.
     */
    void importConfiguration(KConfigBase *config);


    /**
     * Save the shortcuts to the configuration.
     */
    void exportConfiguration(QStringList componentsFriendly, KConfig *config) const;


    /**
     * Are the unsaved changes?
     */
    bool isModified() const;

    enum ComponentScope
        {
        AllComponents,
        CurrentComponent
        };

Q_SIGNALS:

    /**
     * Indicate that state of the modules contents has changed.
     *
     * @param state changed or not
     */
    void changed(bool);


public Q_SLOTS:

    /**
     * Activate the component \a component.
     *
     * @param component the component
     */
    void activateComponent(const QString &component);

    /**
     * Set all shortcuts to none.
     */
    void clearConfiguration();

    /**
     * Load/Reload the global shortcuts
     */
    void load();

    /**
     * Make the changes persistent.
     *
     * That's function is not really saving. Global shortcuts are saved immediately. This
     * prevent the undo on deleting the editor.
     */
    void save();

    /**
     * Set shortcuts to their default value;
     */
    void defaults(ComponentScope scope);

    virtual void importScheme();
    virtual void exportScheme();

private Q_SLOTS:
    void _k_key_changed();

private:

    friend class KGlobalShortcutsEditorPrivate;
    class KGlobalShortcutsEditorPrivate;
    KGlobalShortcutsEditorPrivate *const d;

    Q_DISABLE_COPY(KGlobalShortcutsEditor)
}; // class KGlobalShortcutsEditor

#endif // KGLOBALSHORTCUTSEDITOR_H
