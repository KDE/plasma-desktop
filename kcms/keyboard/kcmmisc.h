/*
 * keyboard.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * https://www.qt.io/
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

#ifndef __KCMMISC_H__
#define __KCMMISC_H__

#include <QMap>
#include <QString>
#include <QWidget>

class QButtonGroup;
class Ui_KeyboardConfigWidget;

const int DEFAULT_REPEAT_DELAY = 600;
const double DEFAULT_REPEAT_RATE = 25.0;

enum KeyBehaviour {
    AccentMenu = 0,
    RepeatKey = 1,
    DoNothing = 2,
};

const QMap<KeyBehaviour, QString> keybehaviourNames = {
    {AccentMenu, QStringLiteral("accent")},
    {RepeatKey, QStringLiteral("repeat")},
    {DoNothing, QStringLiteral("nothing")},
};

enum TriState {
    STATE_ON = 0,
    STATE_OFF = 1,
    STATE_UNCHANGED = 2,
};

class TriStateHelper
{
public:
    static void setTriState(QButtonGroup *group, TriState state);
    static TriState getTriState(const QButtonGroup *group);

    static TriState getTriState(int state)
    {
        return static_cast<TriState>(state);
    }
    static int getInt(TriState state)
    {
        return static_cast<int>(state);
    }
    static const char *getString(TriState state)
    {
        return state == STATE_ON ? "0" : state == STATE_OFF ? "1" : "2";
    }
};

class KCMiscKeyboardWidget : public QWidget
{
    Q_OBJECT
public:
    KCMiscKeyboardWidget(QWidget *parent);
    ~KCMiscKeyboardWidget() override;

    void save();
    void load();
    void defaults();

    QString quickHelp() const;

private Q_SLOTS:
    void changed();

    void delaySliderChanged(int value);
    void delaySpinboxChanged(int value);
    void rateSliderChanged(int value);
    void rateSpinboxChanged(double value);
    void keyboardRepeatStateChanged(int selection);

Q_SIGNALS:
    void changed(bool state);

private:
    void setRepeat(KeyBehaviour flag, int delay, double rate);

    int sliderMax;
    int clickVolume;
    KeyBehaviour keyboardRepeat;
    enum TriState numlockState;

    QButtonGroup *_numlockButtonGroup;
    QButtonGroup *_keyboardRepeatButtonGroup;
    Ui_KeyboardConfigWidget &ui;
};

#endif
