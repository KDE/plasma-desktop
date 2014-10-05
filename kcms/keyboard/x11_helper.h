/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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


#ifndef X11_HELPER_H_
#define X11_HELPER_H_

#include <QKeySequence>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QX11Info>
#include <QAbstractNativeEventFilter>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(KCM_KEYBOARD)


#include <xcb/xcb.h>
//#include <xcb/xkb.h>

//union _xkb_event;
//class xcb_generic_event_t;

// TODO: remove this when we can include xcb/xkb.h
namespace
{
typedef struct _xcb_xkb_map_notify_event_t {
    uint8_t         response_type;
    uint8_t         xkbType;
    uint16_t        sequence;
    xcb_timestamp_t time;
    uint8_t         deviceID;
    uint8_t         ptrBtnActions;
    uint16_t        changed;
    xcb_keycode_t   minKeyCode;
    xcb_keycode_t   maxKeyCode;
    uint8_t         firstType;
    uint8_t         nTypes;
    xcb_keycode_t   firstKeySym;
    uint8_t         nKeySyms;
    xcb_keycode_t   firstKeyAct;
    uint8_t         nKeyActs;
    xcb_keycode_t   firstKeyBehavior;
    uint8_t         nKeyBehavior;
    xcb_keycode_t   firstKeyExplicit;
    uint8_t         nKeyExplicit;
    xcb_keycode_t   firstModMapKey;
    uint8_t         nModMapKeys;
    xcb_keycode_t   firstVModMapKey;
    uint8_t         nVModMapKeys;
    uint16_t        virtualMods;
    uint8_t         pad0[2];
} _xcb_xkb_map_notify_event_t;
typedef struct _xcb_xkb_state_notify_event_t {
    uint8_t         response_type;
    uint8_t         xkbType;
    uint16_t        sequence;
    xcb_timestamp_t time;
    uint8_t         deviceID;
    uint8_t         mods;
    uint8_t         baseMods;
    uint8_t         latchedMods;
    uint8_t         lockedMods;
    uint8_t         group;
    int16_t         baseGroup;
    int16_t         latchedGroup;
    uint8_t         lockedGroup;
    uint8_t         compatState;
    uint8_t         grabMods;
    uint8_t         compatGrabMods;
    uint8_t         lookupMods;
    uint8_t         compatLoockupMods;
    uint16_t        ptrBtnState;
    uint16_t        changed;
    xcb_keycode_t   keycode;
    uint8_t         eventType;
    uint8_t         requestMajor;
    uint8_t         requestMinor;
} _xcb_xkb_state_notify_event_t;
typedef union {
    /* All XKB events share these fields. */
    struct {
        uint8_t response_type;
        uint8_t xkbType;
        uint16_t sequence;
        xcb_timestamp_t time;
        uint8_t deviceID;
    } any;
    _xcb_xkb_map_notify_event_t map_notify;
    _xcb_xkb_state_notify_event_t state_notify;
} _xkb_event;
}

class XEventNotifier : public QObject, public QAbstractNativeEventFilter {
	Q_OBJECT

Q_SIGNALS:
	void layoutChanged();
	void layoutMapChanged();

public:
	XEventNotifier();
	virtual ~XEventNotifier() {}

	virtual void start();
	virtual void stop();

protected:
//    bool x11Event(XEvent * e);
    virtual bool processOtherEvents(xcb_generic_event_t* e);
    virtual bool processXkbEvents(xcb_generic_event_t* e);
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;

private:
	int registerForXkbEvents(Display* display);
	bool isXkbEvent(xcb_generic_event_t* event);
	bool isGroupSwitchEvent(_xkb_event* event);
	bool isLayoutSwitchEvent(_xkb_event* event);

	int xkbOpcode;
};

struct XkbConfig {
	QString keyboardModel;
	QStringList layouts;
	QStringList variants;
	QStringList options;

	bool isValid() { return ! layouts.empty(); }
};


struct LayoutUnit {
	static const int MAX_LABEL_LENGTH;

	//TODO: move these to private?
	QString layout;
	QString variant;

	LayoutUnit() {}
	explicit LayoutUnit(const QString& fullLayoutName);
	LayoutUnit(const QString& layout_, const QString& variant_) {
		layout = layout_;
		variant = variant_;
	}
	/*explicit*/ LayoutUnit(const LayoutUnit& layoutUnit) {
		layout = layoutUnit.layout;
		variant = layoutUnit.variant;
		displayName = layoutUnit.displayName;
		shortcut = layoutUnit.shortcut;
	}

	QString getRawDisplayName() const { return displayName; }
	QString getDisplayName() const { return !displayName.isEmpty() ? displayName :  layout; }
	void setDisplayName(const QString& name) { displayName = name; }

	void setShortcut(const QKeySequence& shortcut) { this->shortcut = shortcut; }
	QKeySequence getShortcut() const { return shortcut; }

	bool isEmpty() const { return layout.isEmpty(); }
	bool isValid() const { return ! isEmpty(); }
	bool operator==(const LayoutUnit& layoutItem) const {
		return layout==layoutItem.layout && variant==layoutItem.variant;
	}
	bool operator!=(const LayoutUnit& layoutItem) const {
		return ! (*this == layoutItem);
	}
	QString toString() const;

private:
	QString displayName;
	QKeySequence shortcut;
};

struct LayoutSet {
	QList<LayoutUnit> layouts;
	LayoutUnit currentLayout;

	LayoutSet() {}

	LayoutSet(const LayoutSet& currentLayouts) {
		this->layouts = currentLayouts.layouts;
		this->currentLayout = currentLayouts.currentLayout;
	}

	bool isValid() const {
		return currentLayout.isValid() && layouts.contains(currentLayout);
	}

	bool operator == (const LayoutSet& currentLayouts) const {
		return this->layouts == currentLayouts.layouts
				&& this->currentLayout == currentLayouts.currentLayout;
	}

	LayoutSet& operator = (const LayoutSet& currentLayouts) {
		this->layouts = currentLayouts.layouts;
		this->currentLayout = currentLayouts.currentLayout;
		return *this;
	}

	QString toString() const {
		QString str(currentLayout.toString());
		str += ": ";
		foreach(const LayoutUnit& layoutUnit, layouts) {
			str += layoutUnit.toString() + " ";
		}
		return str;
	}

	static QString toString(const QList<LayoutUnit>& layoutUnits) {
		QString str;
		foreach(const LayoutUnit& layoutUnit, layoutUnits) {
			str += layoutUnit.toString() + ",";
		}
		return str;
	}
};

class X11Helper
{
public:
	static const int MAX_GROUP_COUNT;
	static const int ARTIFICIAL_GROUP_LIMIT_COUNT;
	
	static const char LEFT_VARIANT_STR[];
	static const char RIGHT_VARIANT_STR[];

	static bool xkbSupported(int* xkbOpcode);

	static void switchToNextLayout();
	static void scrollLayouts(int delta);
	static bool isDefaultLayout();
	static bool setDefaultLayout();
	static bool setLayout(const LayoutUnit& layout);
	static LayoutUnit getCurrentLayout();
	static LayoutSet getCurrentLayouts();
	static QList<LayoutUnit> getLayoutsList();
	static QStringList getLayoutsListAsString(const QList<LayoutUnit>& layoutsList);

	enum FetchType { ALL, LAYOUTS_ONLY, MODEL_ONLY };
	static bool getGroupNames(Display* dpy, XkbConfig* xkbConfig, FetchType fetchType);

private:
	static unsigned int getGroup();
	static bool setGroup(unsigned int group);
};

#endif /* X11_HELPER_H_ */
