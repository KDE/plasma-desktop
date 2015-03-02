/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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
#ifndef DIALOG_PROXY_P
#define DIALOG_PROXY_P

#include <QQuickItem>
#include <QQuickWindow>
#include <QWeakPointer>
#include <QPoint>
#include <QQmlParserStatus>

#include <Plasma/Plasma>
#include <Plasma/Theme>

#include <netwm_def.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QQuickItem;
class QScreen;


namespace PlasmaQuick {

class DialogPrivate;

/**
 * QML wrapper for dialogs
 *
 * Exposed as `PlasmaCore.Dialog` in QML.
 */
class Dialog : public QQuickWindow, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

    /**
     * Margins of the dialog around the mainItem.
     * @see DialogMargins
     */
    Q_PROPERTY(QObject *margins READ margins CONSTANT)

    /**
     * Plasma Location of the dialog window. Useful if this dialog is a popup for a panel
     */
    Q_PROPERTY(Plasma::Types::Location location READ location WRITE setLocation NOTIFY locationChanged)

    /**
     * Type of the window
     */
    Q_PROPERTY(WindowType type READ type WRITE setType NOTIFY typeChanged)

    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

    /**
     * Whether the dialog is output only. Default value is @c false. If it is @c true
     * the dialog does not accept input and all pointer events are not accepted, thus the dialog
     * is click through.
     *
     * This property is currently only supported on the X11 platform. On any other platform the
     * property has no effect.
     **/
    Q_PROPERTY(bool outputOnly READ isOutputOnly WRITE setOutputOnly NOTIFY outputOnlyChanged)

    /**
     * This property holds the window flags of the window.
     * The window flags control the window's appearance in the windowing system,
     * whether it's a dialog, popup, or a regular window, and whether it should
     * have a title bar, etc.
     * Regardless to what the user sets, the flags will always have the
     * FramelessWindowHint flag set
     */
    Q_PROPERTY(Qt::WindowFlags flags READ flags WRITE setFramelessFlags NOTIFY flagsChanged)

    Q_CLASSINFO("DefaultProperty", "mainItem")

public:
    enum WindowType {
        Normal = NET::Normal,
        Dock = NET::Dock,
        DialogWindow = NET::Dialog,
        PopupMenu = NET::PopupMenu,
        Tooltip = NET::Tooltip,
        Notification = NET::Notification
    };
    Q_ENUMS(WindowType)

    Dialog(QQuickItem *parent = 0);
    ~Dialog();

    //PROPERTIES ACCESSORS
    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

    Plasma::Types::Location location() const;
    void setLocation(Plasma::Types::Location location);

    QObject *margins() const;

    void setFramelessFlags(Qt::WindowFlags flags);

    void setType(WindowType type);
    WindowType type() const;

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    void setOutputOnly(bool outputOnly);
    bool isOutputOnly() const;

    /**
     * @returns The suggested screen position for the popup
     * @arg item the item the popup has to be positioned relatively to. if null, the popup will be positioned in the center of the window
     * @arg alignment alignment of the popup compared to the item
     */
    virtual QPoint popupPosition(QQuickItem *item, const QSize &size);

Q_SIGNALS:
    void mainItemChanged();
    void locationChanged();
    void visualParentChanged();
    void typeChanged();
    void hideOnWindowDeactivateChanged();
    void outputOnlyChanged();
    void flagsChanged();

protected:
    /*
     * set the dialog position. subclasses may change it. ToolTipDialog adjusts the position in an animated way
     */
    virtual void adjustGeometry(const QRect &geom);

    //Reimplementations
    virtual void classBegin();
    virtual void componentComplete();
    virtual void resizeEvent(QResizeEvent *re);
    virtual void focusInEvent(QFocusEvent *ev);
    virtual void focusOutEvent(QFocusEvent *ev);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual bool event(QEvent *event);

private:
    friend class DialogPrivate;
    DialogPrivate *const d;

    Q_PRIVATE_SLOT(d, void syncBorders())
    Q_PRIVATE_SLOT(d, void updateContrast())
    Q_PRIVATE_SLOT(d, void updateVisibility(bool visible))
    Q_PRIVATE_SLOT(d, void updateInputShape())

    Q_PRIVATE_SLOT(d, void updateMinimumWidth())
    Q_PRIVATE_SLOT(d, void updateMinimumHeight())
    Q_PRIVATE_SLOT(d, void updateMaximumWidth())
    Q_PRIVATE_SLOT(d, void updateMaximumHeight())

    Q_PRIVATE_SLOT(d, void syncMainItemToSize())
    Q_PRIVATE_SLOT(d, void syncToMainItemSize())
    Q_PRIVATE_SLOT(d, void requestSyncToMainItemSize(bool delayed))
};

}

#endif
