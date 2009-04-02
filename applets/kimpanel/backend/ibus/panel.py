#! /usr/bin/env python
# vim:set et sts=4 sw=4:
#
# ibus-panel-dbus - Another panel for ibus
#
# Copyright (c) 2009 Wang Hoi <zealot.hoi@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

#IBUS_SERVICE_KIMPANEL = "org.freedesktop.IBus.Panel.KIM"
#IBUS_PATH_KIMPANEL = "/org/freedesktop/IBus/Panel/KIM"

from ibus import *
from ibus.panel import * 
from ibus.bus import Bus
from ibus.inputcontext import InputContext
from ibus import keysyms
#import ibus.interface
import gtk
import dbus

IBUS_ICON_DIR = '/usr/share/ibus/icons/'

from gettext import dgettext
_  = lambda a : dgettext("ibus", a)
N_ = lambda a : a

def prop2string(prop):
    __prop_key = '/IBus/'+prop.get_key()
    __prop_label = prop.get_label().get_text()
    __prop_icon = prop.get_icon()
    __prop_tip = prop.get_tooltip().get_text()

    # workaround
    if len(__prop_icon)==0:
        # the setup icon
        if (prop.get_key()=='setup'):
            __prop_icon = 'configure'

    __prop = __prop_key + ':' + __prop_label + ':' + __prop_icon + ':' +  __prop_tip
    return __prop

class KIMIbusClient(dbus.service.Object):
    def __init__(self, object_path):
        dbus.service.Object.__init__(self, dbus.SessionBus(), object_path)

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='s')
    def ExecDialog(self, prop):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='as')
    def ExecMenu(self, props):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='as')
    def RegisterProperties(self, props):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='s')
    def UpdateProperty(self, prop):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='s')
    def RemoveProperty(self, prop):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='b')
    def Enable(self, b):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='b')
    def ShowAux(self, b):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='b')
    def ShowPreedit(self, b):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='b')
    def ShowLookupTable(self, b):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='asasasbb')
    def UpdateLookupTable(self, labels,items,xs,bool1,bool2):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='i')
    def UpdatePreeditCaret(self, pos):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='ss')
    def UpdatePreeditText(self, test, attr):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='ss')
    def UpdateAux(self, test, attr):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='ii')
    def UpdateSpotLocation(self, x, y):
        pass

    @dbus.service.signal(dbus_interface='org.ibus.panel',
                         signature='i')
    def UpdateScreen(self, id):
        pass

class KIMPanel(PanelBase):
    def __init__(self):
        self.__bus = Bus()
        self.__bus.connect("disconnected", gtk.main_quit)
        super(KIMPanel, self).__init__(self.__bus)
        self.__bus.request_name(IBUS_SERVICE_PANEL, 0)
        self.__session_bus = dbus.SessionBus()
        #self.__kimproxy = self.__session_bus.get_object('org.kde.impanel',
        #        '/org/kde/impanel')
        #self.__kimifce = dbus.Interface(self.__kimproxy,
        #        'org.kde.impanel')
        self.__session_bus.add_signal_receiver(self.kim_trigger_property,
                signal_name='TriggerProperty',
                dbus_interface='org.kde.impanel')
        self.__session_bus.add_signal_receiver(self.kim_panel_created,
                signal_name='PanelCreated',
                dbus_interface='org.kde.impanel')
        self.__session_bus.add_signal_receiver(self.kim_reload_config,
                signal_name='ReloadConfig',
                dbus_interface='org.kde.impanel')
        self.__session_bus.add_signal_receiver(gtk.main_quit,
                signal_name='Exit',
                dbus_interface='org.kde.impanel')
        self.__session_bus.add_signal_receiver(self.page_up,
                signal_name='LookupTablePageUp',
                dbus_interface='org.kde.impanel')
        self.__session_bus.add_signal_receiver(self.page_down,
                signal_name='LookupTablePageDown',
                dbus_interface='org.kde.impanel')
        self.__session_bus.add_signal_receiver(self.kim_select_candidate,
                signal_name='SelectCandidate',
                dbus_interface='org.kde.impanel')

        self.__kimclient = KIMIbusClient('/org/ibus/panel')

        self.__focus_ic = None

        self.__logo_prop = Property(key='Logo', label='IBus', icon=IBUS_ICON_DIR + '/ibus.svg', tooltip='IBus input method')
        self.__about_prop = Property(key='About', label=_('IBus intelligent input bus'), icon='help-about')
        self.__about_prop.set_tooltip(_("IBus is an intelligent input bus for Linux/Unix.\n\nHuang Peng <shawn.p.huang@gmail.com>"))
        self.__prop_map = {}

        self.__im_menu = []

    def focus_in(self,ic):
        self.__focus_ic = InputContext(self.__bus, ic)
        enabled = self.__focus_ic.is_enabled()

        if not enabled:
            self.__logo_prop.icon = IBUS_ICON_DIR + '/ibus.svg'
        else:
            engine = self.__focus_ic.get_engine()
            if engine:
                self.__logo_prop.icon = engine.icon
            else:
                self.__logo_prop.icon = IBUS_ICON_DIR + '/ibus.svg'

        self.__kimclient.UpdateProperty(prop2string(self.__logo_prop))

    def state_changed(self):
        print 'state_changed'
        if not self.__focus_ic:
            return

        enabled = self.__focus_ic.is_enabled()

        if enabled == False:
            self.__reset()
            self.__logo_prop.set_icon(IBUS_ICON_DIR + 'ibus.svg')
        else:
            engine = self.__focus_ic.get_engine()
            if engine:
                self.__logo_prop.set_icon(engine.icon)
            else:
                self.__logo_prop.set_icon(IBUS_ICON_DIR + 'ibus.svg')
        self.__kimclient.UpdateProperty(prop2string(self.__logo_prop))

    def focus_out(self,ic):
        #self.__focus_ic = None
        self.__logo_prop.icon = IBUS_ICON_DIR + '/ibus.svg'
        self.__kimclient.UpdateProperty(prop2string(self.__logo_prop))

    def set_cursor_location(self, x, y, w, h):
        #print 'set_cursor_location',x,y,w,h
        self.__kimclient.UpdateSpotLocation(x+w,y+h)

    def update_preedit_text(self, text, cursor_pos, visible):
        print 'update_preedit_text',cursor_pos,visible
        self.__kimclient.UpdatePreeditText(text.get_text(),'')
        self.__kimclient.UpdatePreeditCaret(cursor_pos)
        if visible:
            self.show_preedit_text()
        else:
            self.hide_preedit_text()

    def show_preedit_text(self):
        print 'show_preedit_text'
        self.__kimclient.ShowPreedit(1)

    def hide_preedit_text(self):
        print 'hide_preedit_text'
        self.__kimclient.ShowPreedit(0)

    def update_auxiliary_text(self, text, visible):
        #print 'update_auxiliary_text',visible
        self.__kimclient.UpdateAux(text.get_text(),'')
        if visible:
            self.show_auxiliary_text()
        else:
            self.hide_auxiliary_text()

    def show_auxiliary_text(self):
        print 'show_auxiliary_text'
        self.__kimclient.ShowAux(1)

    def hide_auxiliary_text(self):
        print 'hide_auxiliary_text'
        self.__kimclient.ShowAux(0)

    def update_lookup_table(self, lookup_table, visible):
        if lookup_table == None:
            lookup_table = LookupTable()

        self.__lookup_table = lookup_table

        self.__labels = []
        self.__candis = []
        self.__attrs = []
        i = 0
        for text_obj in lookup_table.get_candidates_in_current_page():
            i=i+1
            if i==10:
                i=0
            self.__labels.append(str(i))
            self.__candis.append(text_obj.get_text())
            self.__attrs.append('')
        
        self.__kimclient.UpdateLookupTable(self.__labels,
                self.__candis,self.__attrs,dbus.Boolean(1),dbus.Boolean(lookup_table.get_current_page_size() <= lookup_table.get_page_size()))

        if visible:
            self.show_lookup_table()
        else:
            self.hide_lookup_table()


    def show_lookup_table(self):
        print 'show_lookup_table'
        self.__kimclient.ShowLookupTable(1)

    def hide_lookup_table(self):
        print 'hide_lookup_table'
        self.__kimclient.ShowLookupTable(0)

    def cursor_up_lookup_table(self):
        print 'cursor_up_lookup_table'

    def cursor_down_lookup_table(self):
        print 'cursor_down_lookup_table'

    def show_candidate_window(self):
        print 'show_candidate_window'

    def hide_candidate_window(self):
        print 'hide_candidate_window'

    def show_language_bar(self):
        print 'show_language_bar'

    def hide_language_bar(self):
        print 'hide_language_bar'

    def register_properties(self, props):
        print 'register_properties'
        __props = []
        __props.append(prop2string(self.__logo_prop))
        for prop in props.get_properties():
            __props.append(prop2string(prop))
            __prop_key = '/IBus/'+prop.get_key()
            #self.__prop
        __props.append(prop2string(self.__about_prop))

        self.__kimclient.RegisterProperties(__props)

    def update_property(self, prop):
        print 'update_property'
        self.__kimclient.UpdateProperty(prop2string(prop))

    def get_status_icon(self):
        print 'get_status_icon'

# begin of signal handler
    
    def kim_panel_created(self):
        print 'KIM: panel created'

    def kim_reload_config(self):
        print 'KIM: reload config'

    def kim_trigger_property(self,prop):
        print 'KIM: trigger property'
        if prop.startswith('/IBus/'):
            __prop_key = prop[6:]
            if __prop_key == 'Logo':
                self.__im_menu = self.__create_im_menu()
                self.__kimclient.ExecMenu(map(prop2string,self.__im_menu))
            elif __prop_key == 'About':
                self.__kimclient.ExecDialog(prop2string(self.__about_prop))
            elif __prop_key.startswith('Engine/'):
                self.__reset()
                __prop_key = __prop_key[7:]
                if __prop_key == 'None':
                    self.__focus_ic.disable()
                else:
                    engines = self.__bus.list_active_engines()
                    for engine in engines:
                        print engine.name
                        if engine.name == __prop_key:
                            print 'matched engine'
                            self.__focus_ic.set_engine(engine)
            else:
                self.property_activate(__prop_key,PROP_STATE_CHECKED)

    def kim_select_candidate(self,index):
        print 'select_candidate:Implement me!'
        # dirty hack
        #if self.__focus_ic:
        #    #engine = self.__focus_ic.get_engine()
        #    #if engine:
        #    #    print 'select_candidate',index
        #    self.__focus_ic.process_key_event(keysyms._1,0)
        pass

    def __reset(self):
        self.hide_auxiliary_text()
        self.hide_preedit_text()
        self.hide_lookup_table()

    def __create_im_menu(self):
        engines = self.__bus.list_active_engines()

        tmp = {}
        for engine in engines:
            lang = get_language_name(engine.language)
            if lang not in tmp:
                tmp[lang] = []
            tmp[lang].append(engine)

        langs = tmp.keys()
        other = tmp.get(_("Other"), [])
        if _("Other") in tmp:
            langs.remove(_("Other"))
            langs.append(_("Other"))

        im_menu = []

        for lang in langs:
            if len(tmp[lang]) == 1:
                engine = tmp[lang][0]
                item = Property(key='Engine/'+engine.name)
                item.set_label("%s - %s" % (lang, engine.longname))
                if engine.icon:
                    item.set_icon(engine.icon)
                else:
                    item.set_icon("engine-default")
                print prop2string(item)
                im_menu.append(item)
            else:
                pass

        item = Property(key='Engine/None',label=_('Disable'),icon=IBUS_ICON_DIR+'/ibus.svg')
        im_menu.append(item)

        return im_menu


def launch_panel():
    panel = KIMPanel()
    gtk.main()

if __name__ == "__main__":
    launch_panel()

