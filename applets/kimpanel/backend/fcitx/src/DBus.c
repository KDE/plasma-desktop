#ifndef DBUS_C
#define DBUS_C

#include "DBus.h"

#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <strings.h> // bzero()

DBusError err;
DBusConnection *conn;
int ret;

//extern IMProtocol * current_call_data;
extern INT8 iIMCount;
extern INT8 iIMIndex;
extern CARD16 connect_id;
extern Bool bIsInLegend;
extern int iLegendCandWordCount;

void fixProperty(Property *prop);

Bool InitDBus()
{
    // first init dbus
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) { 
        fprintf(stderr, "Connection Error (%s)\n", err.message); 
        dbus_error_free(&err); 
    }
    if (NULL == conn) { 
        return False;
    }

    // request a name on the bus
    ret = dbus_bus_request_name(conn, "org.kde.fcitx", 
            DBUS_NAME_FLAG_REPLACE_EXISTING,
            &err);
    if (dbus_error_is_set(&err)) { 
        fprintf(stderr, "Name Error (%s)\n", err.message); 
        dbus_error_free(&err); 
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
        return False;
    }

    dbus_connection_flush(conn);
    // add a rule to receive signals from kimpanel
    dbus_bus_add_match(conn, 
            "type='signal',interface='org.kde.impanel'", 
            &err);
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err)) { 
        fprintf(stderr, "Match Error (%s)\n", err.message);
        return False;
    }

    return True;
}

void TestSendSignal()
{
}

void MyDBusEventHandler()
{
    dbus_connection_read_write(conn, 10);
    DBusMessage *msg;
    DBusMessageIter args;

    int int0, int1, int2, int3;
    char *s0, *s1, *s2, *s3;
   
    while ((msg = dbus_connection_pop_message(conn))) {
        if (dbus_message_is_signal(msg, "org.kde.impanel", "MovePreeditCaret")) {
            fprintf(stderr,"MovePreeditCaret\n");
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n"); 
            else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) 
                fprintf(stderr, "Argument is not INT32!\n"); 
            else {
                dbus_message_iter_get_basic(&args, &int0);
                printf("Got Signal with value %d\n", int0);
            }
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "SelectCandidate")) {
            fprintf(stderr,"SelectCandidate: ");
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n"); 
            else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) 
                fprintf(stderr, "Argument is not INT32!\n"); 
            else {
                dbus_message_iter_get_basic(&args, &int0);

                char *pstr;

                if (!bIsInLegend) {
                    pstr = im[iIMIndex].GetCandWord(int0);
                    if (pstr) {
                        SendHZtoClient(0,pstr);
                        if (!bUseLegend) {
                            ResetInput();
                        } else {
                            updateMessages();
                        }
                    } else {
                        updateMessages();
                    }
                } else {
                    pstr = im[iIMIndex].GetLegendCandWord(int0);
                    if (pstr) {
                        SendHZtoClient(0,pstr);
                        if (iLegendCandWordCount) {
                            updateMessages();
                        }
                        else {
                            ResetInput ();
                        }
                    }
                }
                fprintf(stderr,"%d:%s\n", int0,pstr);
            }
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "LookupTablePageUp")) {
            fprintf(stderr,"LookupTablePageUp\n");
            im[iIMIndex].GetCandWords (SM_PREV);
            updateMessages();
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "LookupTablePageDown")) {
            fprintf(stderr,"LookupTablePageDown\n");
            im[iIMIndex].GetCandWords (SM_NEXT);
            updateMessages();
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "TriggerProperty")) {
            fprintf(stderr,"TriggerProperty: ");
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message has no arguments!\n"); 
            else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) 
                fprintf(stderr, "Argument is not STRING!\n"); 
            else {
                dbus_message_iter_get_basic(&args, &s0);
                fprintf(stderr,"%s\n", s0);

                triggerProperty(s0);
            }
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "PanelCreated")) {
            fprintf(stderr,"PanelCreated\n");
            registerProperties();
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "Exit")) {
            fprintf(stderr,"Exit\n");
        }
        if (dbus_message_is_signal(msg, "org.kde.impanel", "ReloadConfig")) {
            fprintf(stderr,"ReloadConfig\n");
        }
        // free the message
        dbus_message_unref(msg);
    }

}


void KIMExecDialog(char *prop)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "ExecDialog"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMExecMenu(char *props[],int n)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "ExecMenu"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    if (n == -1) {
        n = 0;
        while (*(props[n])!= 0) {
            n++;
        }
    }

    int i;
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    DBusMessageIter sub;
    dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,"s",&sub);
    for (i = 0; i < n; i++) {
        if (!dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &props[i])) { 
            fprintf(stderr, "Out Of Memory!\n"); 
        }
    }
    dbus_message_iter_close_container(&args,&sub);

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMRegisterProperties(char *props[], int n)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "RegisterProperties"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    if (n == -1) {
        n = 0;
        while (*(props[n])!= 0) {
            n++;
        }
    }

    int i;
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    DBusMessageIter sub;
    dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,"s",&sub);
    for (i = 0; i < n; i++) {
        if (!dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &props[i])) { 
            fprintf(stderr, "Out Of Memory!\n"); 
        }
    }
    dbus_message_iter_close_container(&args,&sub);

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdateProperty(char *prop)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdateProperty"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMRemoveProperty(char *prop)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "RemoveProperty"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMEnable(Bool toEnable)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "Enable"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toEnable)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMShowAux(Bool toShow)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "ShowAux"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toShow)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMShowPreedit(Bool toShow)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "ShowPreedit"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toShow)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMShowLookupTable(Bool toShow)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "ShowLookupTable"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &toShow)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdateLookupTable(char *labels[], int nLabel, char *texts[], int nText /* char *attrs[], int int0, int int1, int int2, Bool b0*/)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdateLookupTable"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    int i;
    DBusMessageIter subLabel;
    DBusMessageIter subText;
    DBusMessageIter subAttrs;
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,"s",&subLabel);
    for (i = 0; i < nLabel; i++) {
        if (!dbus_message_iter_append_basic(&subLabel, DBUS_TYPE_STRING, &labels[i])) { 
            fprintf(stderr, "Out Of Memory!\n"); 
        }
    }
    dbus_message_iter_close_container(&args,&subLabel);

    dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,"s",&subText);
    for (i = 0; i < nText; i++) {
        if (!dbus_message_iter_append_basic(&subText, DBUS_TYPE_STRING, &texts[i])) { 
            fprintf(stderr, "Out Of Memory!\n"); 
        }
    }
    dbus_message_iter_close_container(&args,&subText);

    char *attr = "";
    dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,"s",&subAttrs);
    for (i = 0; i < nLabel; i++) {
        if (!dbus_message_iter_append_basic(&subAttrs, DBUS_TYPE_STRING, &attr)) { 
            fprintf(stderr, "Out Of Memory!\n"); 
        }
    }
    dbus_message_iter_close_container(&args,&subAttrs);

    int int0 = 0;
    Bool b0 = True;
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &int0);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &int0);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &int0);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &b0);

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdatePreeditCaret(int position)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdatePreeditCaret"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &position)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdatePreeditText(char *text)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdatePreeditText"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    char *attr = "";
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &text)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &attr)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdateAux(char *text)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdateAux"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    char *attr = "";

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &text)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &attr)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdateSpotLocation(int x,int y)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdateSpotLocation"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &x)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &y)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void KIMUpdateScreen(int id)
{

    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage* msg;
    DBusMessageIter args;

    // create a signal and check for errors 
    msg = dbus_message_new_signal("/fcitx", // object name of the signal
            "org.kde.fcitx", // interface name of the signal
            "UpdateScreen"); // name of the signal
    if (NULL == msg) 
    { 
        fprintf(stderr, "Message Null\n"); 
    }

    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &id)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }

    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) { 
        fprintf(stderr, "Out Of Memory!\n"); 
    }
    dbus_connection_flush(conn);

    // free the message 
    dbus_message_unref(msg);

}

void updateMessages()
{
    int n = uMessageUp;
    char aux[100] = "";
    int i;
    if (n) {
        // FIXME: buffer overflow
        for (i=0;i<n;i++) {
            strcat(aux,g2u(messageUp[i].strMsg));
        }
        KIMUpdateAux(aux);
        KIMShowAux(True);
    } else {
        KIMShowAux(False);
    }

    n = uMessageDown;
    int nLabels = 0;
    int nTexts = 0;
    char *label[33];
    char *text[33];
    char cmb[100];

    if (n) {
        for (i=0;i<n;i++) {
            fprintf(stderr,"Type: %d Text: %s\n",messageDown[i].type,g2u(messageDown[i].strMsg));
            if (messageDown[i].type == MSG_INDEX) {
                if (nLabels) {
                    text[nTexts++] = strdup(cmb);
                }
                label[nLabels++] = g2u(messageDown[i].strMsg);
                strcpy(cmb,"");
            } else {
                strcat(cmb,g2u(messageDown[i].strMsg));
            }
        }
        text[nTexts++] = strdup(cmb);
        fprintf(stderr,"Labels %d, Texts %d\n",nLabels,nTexts);
        KIMUpdateLookupTable(label,nLabels,text,nTexts);
        KIMShowLookupTable(True);
    } else {
        KIMShowLookupTable(False);
    }
}

void showMessageDown(MESSAGE *msgs, int n)
{
}

void registerProperties()
{
    char *props[10];

    char icon[100] = "";

    logo_prop.key = "/Fcitx/Logo";
    logo_prop.label = "Fcitx";
    logo_prop.icon = PKGDATADIR "/xpm/" "Logo.png";
    logo_prop.tip = "小企鹅输入法";
    props[0] = property2string(&logo_prop);

    state_prop.key = "/Fcitx/State";
    state_prop.label = "英";
    state_prop.icon = "";
    state_prop.tip = "小企鹅输入法";
    props[1] = property2string(&state_prop);

    punc_prop.key = "/Fcitx/Punc";
    punc_prop.label = bChnPunc ? "UP" : "NP";

    if (bChnPunc) {
        punc_prop.icon = PKGDATADIR "/xpm/" "full-punct.png";
    } else {
        punc_prop.icon = PKGDATADIR "/xpm/" "half-punct.png";
    }
    punc_prop.tip = "中英文标点切换";
    props[2] = property2string(&punc_prop);

    corner_prop.key = "/Fcitx/Corner";
    corner_prop.label = bCorner ? "全" : "半";
    if (bCorner) {
        corner_prop.icon = PKGDATADIR "/xpm/" "full-letter.png";
    } else {
        corner_prop.icon = PKGDATADIR "/xpm/" "half-letter.png";
    }
    corner_prop.tip = "全半角切换";
    props[3] = property2string(&corner_prop);

    gbk_prop.key = "/Fcitx/GBK";
    gbk_prop.label = bUseGBK ? "GBK" : "GB";
    gbk_prop.icon = "";
    gbk_prop.tip = bUseGBK ? "标准GB字符集" : "扩展GBK字符集";
    props[4] = property2string(&gbk_prop);

    gbkt_prop.key = "/Fcitx/GBKT";
    gbkt_prop.label = bUseGBKT ? "繁" : "简";
    gbkt_prop.icon = "";
    gbkt_prop.tip = "简繁体转换";
    props[5] = property2string(&gbkt_prop);

    legend_prop.key = "/Fcitx/Legend";
    legend_prop.label = bUseLegend ? "联" : "无联";
    legend_prop.icon = "";
    legend_prop.tip = "启用联想支持";
    props[6] = property2string(&legend_prop);

    KIMRegisterProperties(props,7);
}

void updateProperty(Property *prop)
{
    if (!prop) {
        return;
    }
    // logo_prop need to be handled specially (enable/disable)
    if (prop == &logo_prop) {
        fixProperty(prop);
    }
    if (prop == &state_prop) {
        prop->label = (ConnectIDGetState(connect_id) == IS_CHN) ? "中" : "英";
        //        prop->label = g2u(im[iIMIndex].strName);
    }
    if (prop == &punc_prop) {
        prop->label = bChnPunc ? "UP" : "NP";
        if (bChnPunc) {
            prop->icon = PKGDATADIR "/xpm/" "full-punct.png";
        } else {
            prop->icon = PKGDATADIR "/xpm/" "half-punct.png";
        }
    }
    if (prop == &corner_prop) {
        prop->label = bCorner ? "全" : "半";
        if (bCorner) {
            prop->icon = PKGDATADIR "/xpm/" "full-letter.png";
        } else {
            prop->icon = PKGDATADIR "/xpm/" "half-letter.png";
        }
    }
    if (prop == &gbk_prop) {
        prop->label = bUseGBK ? "GBK" : "GB";
    }
    if (prop == &gbkt_prop) {
        prop->label = bUseGBKT ? "繁" : "简";
    }
    if (prop == &legend_prop) {
        prop->label = bUseLegend ? "联" : "无联";
    }
    KIMUpdateProperty(property2string(prop));
}

void triggerProperty(char *propKey)
{
    char *menu[32];
    char *name;
    int i;
    bzero(menu,32);
    if (strcmp(propKey,logo_prop.key) == 0) {
        for (i = 0; i < iIMCount; i++) {
            name = g2u(im[i].strName);
            //menu[i] = (char *)malloc(100*sizeof(char));
            Property prop;
            prop.key = (char *)malloc(100*sizeof(char));
            prop.icon = "";
            sprintf(prop.key,"/Fcitx/Logo/%d",i);
            prop.label = name;
            prop.tip = name;
            //            strcat(prop.tip,"输入法");
            fixProperty(&prop);
            menu[i] = property2string(&prop);
        }
        KIMExecMenu(menu,iIMCount);
    } else {
        if (strstr(propKey,logo_prop.key) == propKey) {
            i = atoi(strrchr(propKey,'/') + 1);
            if (ConnectIDGetState (connect_id) == IS_CLOSED) {
                SetConnectID (connect_id, IS_CHN);
                EnterChineseMode (False);
            } else if (ConnectIDGetState (connect_id) == IS_ENG) {
                ChangeIMState(connect_id);
            }
            SwitchIM(i);
        }
    }

    if (strcmp(propKey,state_prop.key) == 0) {
        ChangeIMState(connect_id);
    }
    if (strcmp(propKey,punc_prop.key) == 0) {
        ChangePunc();
    }
    if (strcmp(propKey,corner_prop.key) == 0) {
        ChangeCorner();
    }
    if (strcmp(propKey,gbk_prop.key) == 0) {
        ChangeGBK();
    }
    if (strcmp(propKey,gbkt_prop.key) == 0) {
        ChangeGBKT();
    }
    if (strcmp(propKey,legend_prop.key) == 0) {
        ChangeLegend();
    }
}

char* property2string(Property *prop)
{
    char *result = (char*)malloc(500*sizeof(char));
    bzero(result,500*sizeof(char));
    strcat(result,prop->key);
    strcat(result,":");
    strcat(result,prop->label);
    strcat(result,":");
    strcat(result,prop->icon);
    strcat(result,":");
    strcat(result,prop->tip);
    return result;
}

char* g2u(char *instr)
{
    iconv_t cd;
    char *inbuf;
    char *outbuf;
    char *outptr;
    unsigned int insize=strlen(instr);
    unsigned int outputbufsize=100;
    unsigned int avail=outputbufsize;
    unsigned int nconv;
    inbuf=instr;
    outbuf=(char *)malloc(outputbufsize);
    outptr=outbuf;    //使用outptr作为空闲空间指针以避免outbuf被改变
    memset(outbuf,'\0',outputbufsize);
    cd=iconv_open("utf-8","gb18030");    //将字符串编码由gtk转换为utf-8
    if(cd==(size_t)-1)
    {
        return "";
    }
    nconv=iconv(cd,&inbuf,&insize,&outptr,&avail);
    return outbuf;
}

char* u2g(char *instr)
{
    iconv_t cd;
    char *inbuf;
    char *outbuf;
    char *outptr;
    unsigned int insize=strlen(instr);
    unsigned int outputbufsize=100;
    unsigned int avail=outputbufsize;
    unsigned int nconv;
    inbuf=instr;
    outbuf=(char *)malloc(outputbufsize);
    outptr=outbuf;    //使用outptr作为空闲空间指针以避免outbuf被改变
    memset(outbuf,'\0',outputbufsize);
    cd=iconv_open("gb18030","utf-8");    //将字符串编码由utf-8转换为gbk
    if(cd==(size_t)-1)
    {
        return "";
    }
    nconv=iconv(cd,&inbuf,&insize,&outptr,&avail);
    return outbuf;

}

void fixProperty(Property *prop)
{
    if (strcmp(prop->label,"Fcitx") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "Logo.png";
    }
    if (strcmp(prop->label,"智能拼音") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "PinYin.png";
    }
    if (strcmp(prop->label,"智能双拼") == 0) {
        //        prop->label = "双拼";
        prop->icon = "";
    }
    if (strcmp(prop->label,"区位") == 0) {
        //        prop->label = "区位";
        prop->icon = "";
    }
    if (strcmp(prop->label,"五笔字型") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "Wubi.png";
    }
    if (strcmp(prop->label,"五笔拼音") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "Wubi.png";
    }
    if (strcmp(prop->label,"二笔") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "Erbi.png";
    }
    if (strcmp(prop->label,"仓颉") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "CangJie.png";
    }
    if (strcmp(prop->label,"晚风") == 0) {
        prop->icon = "";
    }
    if (strcmp(prop->label,"冰蟾全息") == 0) {
        //        prop->label = "冰";
        prop->icon = "";
    }
    if (strcmp(prop->label,"自然码") == 0) {
        prop->icon = PKGDATADIR "/xpm/" "Ziranma.png";
    }
    if (strcmp(prop->label,"电报码") == 0) {
        prop->icon = "";
        //        prop->label = "电";
    }
}

// vim: sw=4 sts=4 et tw=100
#endif // DBUS_C
