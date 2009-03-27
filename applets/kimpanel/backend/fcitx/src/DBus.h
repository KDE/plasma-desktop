#ifndef FCITX_DBUS_H
#define FCITX_DBUS_H

#include <dbus/dbus.h>
#include "main.h"
#include "InputWindow.h"
#include "ime.h"
#include "xim.h"

extern IM *im;
extern INT8 iIMIndex;

extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;

extern INT8 iState;
extern Bool bChnPunc;
extern Bool bCorner;
extern Bool bUseGBK;
extern Bool bUseGBKT;
extern Bool bUseLegend;
extern Bool bUseTable;
extern Bool bUseQW;
extern Bool bUseSP;
extern Bool bUseAA;
extern Bool bUseMatchingKey;
extern Bool bUsePinyin;
extern Bool bUseBold;

typedef struct Property_ {
    char *key;
    char *label;
    char *icon;
    char *tip;
} Property;

Property logo_prop;
Property state_prop;
Property punc_prop;
Property corner_prop;
Property gbk_prop;
Property gbkt_prop;
Property legend_prop;

extern DBusError err;
extern DBusConnection *conn;

Bool InitDBus();
void MyDBusEventHandler();

void KIMExecDialog(char *prop);
void KIMExecMenu(char *props[],int n);
void KIMRegisterProperties(char *props[],int n);
void KIMUpdateProperty(char *prop);
void KIMRemoveProperty(char *prop);
void KIMEnable(Bool toEnable);
void KIMShowAux(Bool toShow);
void KIMShowPreedit(Bool toShow);
void KIMShowLookupTable(Bool toShow);
void KIMUpdateLookupTable(char *labels[], int nLabel, char *texts[], int nText /* char *attrs[], int int0, int int1, int int2, Bool b0*/);
void KIMUpdatePreeditCaret(int position);
void KIMUpdatePreeditText(char *text);
void KIMUpdateAux(char *text);
void KIMUpdateSpotLocation(int x,int y);
void KIMUpdateScreen(int id);

void updateMessages();

void registerProperties();
void updateProperty(Property *prop);
void triggerProperty(char *propKey);

char* property2string(Property *prop);

char* g2u(char *instr);
char* u2g(char *instr);

#endif // FCITX_DBUS_H
