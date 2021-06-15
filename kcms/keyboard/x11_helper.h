/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef X11_HELPER_H_
#define X11_HELPER_H_

#include <QAbstractNativeEventFilter>
#include <QKeySequence>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QX11Info>

#define explicit explicit_is_keyword_in_cpp
#include <xcb/xcb.h>
#include <xcb/xkb.h>
#undef explicit

namespace
{
typedef union {
    /* All XKB events share these fields. */
    struct {
        uint8_t response_type;
        uint8_t xkbType;
        uint16_t sequence;
        xcb_timestamp_t time;
        uint8_t deviceID;
    } any;
    xcb_xkb_new_keyboard_notify_event_t new_keyboard_notify;
    xcb_xkb_map_notify_event_t map_notify;
    xcb_xkb_state_notify_event_t state_notify;
} _xkb_event;
}

class XEventNotifier : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

Q_SIGNALS:
    void layoutChanged();
    void layoutMapChanged();

public:
    XEventNotifier();
    ~XEventNotifier() override
    {
    }

    virtual void start();
    virtual void stop();

protected:
    virtual bool processOtherEvents(xcb_generic_event_t *e);
    virtual bool processXkbEvents(xcb_generic_event_t *e);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;

private:
    int registerForXkbEvents(Display *display);
    bool isXkbEvent(xcb_generic_event_t *event);
    bool isGroupSwitchEvent(_xkb_event *event);
    bool isLayoutSwitchEvent(_xkb_event *event);

    int xkbOpcode;
};

struct XkbConfig {
    QString keyboardModel;
    QStringList layouts;
    QStringList variants;
    QStringList options;

    bool isValid()
    {
        return !layouts.empty();
    }
};

class LayoutUnit
{
public:
    static const int MAX_LABEL_LENGTH;

    LayoutUnit()
    {
    }
    explicit LayoutUnit(const QString &fullLayoutName);
    LayoutUnit(const QString &layout, const QString &variant)
    {
        m_layout = layout;
        m_variant = variant;
    }
    /*explicit*/ LayoutUnit(const LayoutUnit &other)
    {
        operator=(other);
    }

    LayoutUnit &operator=(const LayoutUnit &other)
    {
        if (this != &other) {
            m_layout = other.m_layout;
            m_variant = other.m_variant;
            displayName = other.displayName;
            shortcut = other.shortcut;
        }
        return *this;
    }

    QString getRawDisplayName() const
    {
        return displayName;
    }
    QString getDisplayName() const
    {
        return !displayName.isEmpty() ? displayName : m_layout;
    }
    void setDisplayName(const QString &name)
    {
        displayName = name;
    }

    void setShortcut(const QKeySequence &shortcut)
    {
        this->shortcut = shortcut;
    }
    QKeySequence getShortcut() const
    {
        return shortcut;
    }
    QString layout() const
    {
        return m_layout;
    }
    void setLayout(const QString &layout)
    {
        m_layout = layout;
    }
    QString variant() const
    {
        return m_variant;
    }
    void setVariant(const QString &variant)
    {
        m_variant = variant;
    }

    bool isEmpty() const
    {
        return m_layout.isEmpty();
    }
    bool isValid() const
    {
        return !isEmpty();
    }
    bool operator==(const LayoutUnit &layoutItem) const
    {
        // FIXME: why not compare the other properties?
        return m_layout == layoutItem.m_layout && m_variant == layoutItem.m_variant;
    }
    bool operator!=(const LayoutUnit &layoutItem) const
    {
        return !(*this == layoutItem);
    }
    QString toString() const;

private:
    QString displayName;
    QKeySequence shortcut;
    QString m_layout;
    QString m_variant;
};

struct LayoutSet {
    QList<LayoutUnit> layouts;
    LayoutUnit currentLayout;

    LayoutSet()
    {
    }

    LayoutSet(const LayoutSet &other)
    {
        operator=(other);
    }

    bool isValid() const
    {
        return currentLayout.isValid() && layouts.contains(currentLayout);
    }

    bool operator==(const LayoutSet &currentLayouts) const
    {
        return this->layouts == currentLayouts.layouts && this->currentLayout == currentLayouts.currentLayout;
    }

    LayoutSet &operator=(const LayoutSet &currentLayouts)
    {
        this->layouts = currentLayouts.layouts;
        this->currentLayout = currentLayouts.currentLayout;
        return *this;
    }

    QString toString() const
    {
        QString str(currentLayout.toString());
        str += QLatin1String(": ");
        for (const auto &layoutUnit : qAsConst(layouts)) {
            str += layoutUnit.toString() + QLatin1Char(' ');
        }
        return str;
    }

    static QString toString(const QList<LayoutUnit> &layoutUnits)
    {
        QString str;
        for (const auto &layoutUnit : layoutUnits) {
            str += layoutUnit.toString() + QLatin1Char(',');
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

    static bool xkbSupported(int *xkbOpcode);

    static void switchToNextLayout();
    static void scrollLayouts(int delta);
    static bool isDefaultLayout();
    static bool setDefaultLayout();
    static bool setLayout(const LayoutUnit &layout);
    static LayoutUnit getCurrentLayout();
    static LayoutSet getCurrentLayouts();
    static QList<LayoutUnit> getLayoutsList();
    static QStringList getLayoutsListAsString(const QList<LayoutUnit> &layoutsList);

    enum FetchType { ALL, LAYOUTS_ONLY, MODEL_ONLY };
    static bool getGroupNames(Display *dpy, XkbConfig *xkbConfig, FetchType fetchType);

    static unsigned int getGroup();
    static bool setGroup(unsigned int group);
};

#endif /* X11_HELPER_H_ */
