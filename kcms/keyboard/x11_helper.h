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

#warning XEventNotifier needs porting to QAbstractNativeEventFilter
#if 0
class XEventNotifier : public QWidget {
	Q_OBJECT

Q_SIGNALS:
	void layoutChanged();
	void layoutMapChanged();

public:
	XEventNotifier(QWidget* parent=NULL);
	virtual ~XEventNotifier() {}

	virtual void start();
	virtual void stop();

protected:
    bool x11Event(XEvent * e);
    virtual bool processOtherEvents(XEvent* e);
    virtual bool processXkbEvents(XEvent* e);

private:
	int registerForXkbEvents(Display* display);
	bool isXkbEvent(XEvent* event);
	bool isGroupSwitchEvent(XEvent* event);
	bool isLayoutSwitchEvent(XEvent* event);

	int xkbOpcode;
};
#endif

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
	static int MAX_GROUP_COUNT;
	static int ARTIFICIAL_GROUP_LIMIT_COUNT;
	
	static const char* LEFT_VARIANT_STR;
	static const char* RIGHT_VARIANT_STR;

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
