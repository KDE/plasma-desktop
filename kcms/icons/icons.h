/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __icons_h__
#define __icons_h__

#include <QColor>
#include <QImage>
#include <QDialog>

#include <KCModule>
#include <KConfig>
#include <KSharedConfig>

class QCheckBox;
class QColor;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;
class QSlider;
class QTabWidget;
class QWidget;

class KColorButton;
class KIconEffect;
class KIconLoader;

struct Effect
{
    int type;
    float value;
    QColor color;
    QColor color2;
    bool transparent;
};


/**
 * The General Icons tab in kcontrol.
 */
class KIconConfig: public KCModule
{
    Q_OBJECT

public:
    KIconConfig(QWidget *parent);
    ~KIconConfig();

    virtual void load();
    virtual void save();
    virtual void defaults();
    void preview();

private Q_SLOTS:
    void slotEffectSetup0() { EffectSetup(0); }
    void slotEffectSetup1() { EffectSetup(1); }
    void slotEffectSetup2() { EffectSetup(2); }

    void slotUsage(int index);
    void slotSize(int index);
    void slotAnimatedCheck(bool check);

private:
    void preview(int i);
    void EffectSetup(int state);
    QPushButton *addPreviewIcon(int i, const QString &str, QWidget *parent, QGridLayout *lay);
    void init();
    void initDefaults();
    void read();
    void apply();

    void exportToKDE4();


    bool mbChanged[6], mbAnimated[6];
    int mSizes[6];
    QList<int> mAvSizes[6];

    Effect mEffects[6][3];
    Effect mDefaultEffect[3];

    int mUsage;
    QString mTheme, mExample;
    QStringList mGroups, mStates;

    KIconEffect *mpEffect;
    KIconLoader *mpLoader;
    KSharedConfigPtr mpConfig;

    QLabel *mpPreview[3];

    QListWidget *mpUsageList;
    QComboBox *mpSizeBox;
    QCheckBox *wordWrapCB, *underlineCB, *mpAnimatedCheck;
    QTabWidget *m_pTabWidget;
    QWidget *m_pTab1;
};

class KIconEffectSetupDialog: public QDialog
{
    Q_OBJECT

public:
    KIconEffectSetupDialog(const Effect &, const Effect &,
                           const QString &, const QImage &,
			   QWidget *parent=0L, char *name=0L);
    ~KIconEffectSetupDialog();
    Effect effect() { return mEffect; }

protected:
    void preview();
    void init();

protected Q_SLOTS:
    void slotEffectValue(int value);
    void slotEffectColor(const QColor &col);
    void slotEffectColor2(const QColor &col);
    void slotEffectType(int type);
    void slotSTCheck(bool b);
    void slotDefault();

private:
    KIconEffect *mpEffect;
    QListWidget *mpEffectBox;
    QCheckBox *mpSTCheck;
    QSlider *mpEffectSlider;
    KColorButton *mpEColButton;
    KColorButton *mpECol2Button;
    Effect mEffect;
    Effect mDefaultEffect;
    QImage mExample;
    QGroupBox *mpEffectGroup;
    QLabel *mpPreview, *mpEffectLabel, *mpEffectColor, *mpEffectColor2;
};

#endif
