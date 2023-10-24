/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#define QT_FORCE_ASSERTS

#include <atomic>
#include <iostream>
#include <thread>

#include <Python.h>

#include "qwayland-fake-input.h"
#include <QGuiApplication>
#include <QWaylandClientExtensionTemplate>

#include <wayland-client-protocol.h>

using namespace Qt::StringLiterals;

namespace
{
class FakeInputInterface : public QWaylandClientExtensionTemplate<FakeInputInterface>, public QtWayland::org_kde_kwin_fake_input
{
public:
    FakeInputInterface()
        : QWaylandClientExtensionTemplate<FakeInputInterface>(5 /*ORG_KDE_KWIN_FAKE_INPUT_DESTROY_SINCE_VERSION*/)
    {
        initialize();
    }

    ~FakeInputInterface()
    {
        destroy();
    }

    Q_DISABLE_COPY_MOVE(FakeInputInterface)
};

QGuiApplication *s_application = nullptr;
wl_display *s_display = nullptr;
FakeInputInterface *s_fakeInputInterface = nullptr;

[[nodiscard]] inline quint32 linuxKeyCode(int keycode)
{
    // The offset between KEY_* numbering, and keycodes in the XKB evdev dataset.
    // Stolen from xdg-desktop-portal-kde.
    constexpr uint EVDEV_OFFSET = 8;

    return keycode - EVDEV_OFFSET;
}

void init_fake_input()
{
    QMetaObject::invokeMethod(
        s_application,
        [] {
            s_fakeInputInterface = new FakeInputInterface();
            s_fakeInputInterface->setParent(s_application);
            if (!s_fakeInputInterface->isInitialized()) {
                std::cerr << "Failed to initialize fake_input" << std::endl;
                Q_UNREACHABLE(); // Crash directly to fail a test early
            }
            if (!s_fakeInputInterface->isActive()) {
                std::cerr << "fake_input is inactive" << std::endl;
                Q_UNREACHABLE(); // Crash directly to fail a test early
            }
            s_fakeInputInterface->authenticate("inputsynth"_L1, "used in a test"_L1);
            s_display = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()->display();
            wl_display_roundtrip(s_display);
        },
        Qt::BlockingQueuedConnection);
}

PyObject *init_module(PyObject *, PyObject *)
{
    if (s_application) {
        Py_RETURN_NONE;
    }
    std::atomic<bool> locked{true}; // Used to block function return in the main thread
    std::thread t([&locked] {
        int argc = 1;
        const char *argv[] = {"inputsynth"};
        qputenv("QT_QPA_PLATFORM", "wayland");
        s_application = new QGuiApplication(argc, const_cast<char **>(argv));
        locked = false;
        locked.notify_one();
        std::clog << "QGuiApplication thread has started" << s_application << std::endl;
        s_application->exec();
    });
    t.detach();
    locked.wait(true);
    init_fake_input();
    Py_RETURN_NONE;
}

void unload_module(void *)
{
    if (!s_application) {
        return;
    }
    QMetaObject::invokeMethod(
        s_application,
        [] {
            s_application->quit();
            s_application = nullptr;
        },
        Qt::BlockingQueuedConnection);
    std::clog << "QGuiApplication quit" << std::endl;
}

PyObject *key_press(PyObject *, PyObject *arg)
{
    const long keycode = PyLong_AsLong(arg);
    QMetaObject::invokeMethod(
        s_application,
        [keycode] {
            s_fakeInputInterface->keyboard_key(linuxKeyCode(keycode), WL_KEYBOARD_KEY_STATE_PRESSED);
            wl_display_roundtrip(s_display);
        },
        Qt::BlockingQueuedConnection);
    std::clog << "pressing " << keycode << std::endl;
    Py_RETURN_NONE;
}

PyObject *key_release(PyObject *, PyObject *arg)
{
    const long keycode = PyLong_AsLong(arg);
    QMetaObject::invokeMethod(
        s_application,
        [keycode] {
            s_fakeInputInterface->keyboard_key(linuxKeyCode(keycode), WL_KEYBOARD_KEY_STATE_RELEASED);
            wl_display_roundtrip(s_display);
        },
        Qt::BlockingQueuedConnection);
    std::clog << "releasing" << keycode << std::endl;
    Py_RETURN_NONE;
}

// Exported methods are collected in a table
// https://docs.python.org/3/c-api/structures.html
PyMethodDef method_table[] = {
    {"init_module", (PyCFunction)init_module, METH_NOARGS, "Initialize QGuiApplication"},
    {"key_press", (PyCFunction)key_press, METH_O, "Press down and hold a key"},
    {"key_release", (PyCFunction)key_release, METH_O, "Release a key"},
    {nullptr, nullptr, 0, nullptr} // Sentinel value ending the table
};

// A struct contains the definition of a module
PyModuleDef inputsynth_module = {
    PyModuleDef_HEAD_INIT,
    "inputsynth_plasma_desktop", // Module name
    "Python bindings of the fake input protocol",
    -1, // Optional size of the module state memory
    method_table,
    nullptr, // Optional slot definitions
    nullptr, // Optional traversal function
    nullptr, // Optional clear function
    unload_module // Optional module deallocation function
};
}

// The module init function
PyMODINIT_FUNC PyInit_inputsynth_plasma_desktop(void)
{
    PyObject *m = PyModule_Create(&inputsynth_module);
    if (!m) {
        Q_UNREACHABLE();
    }
    return m;
}
