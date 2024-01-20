#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import time

import gi

gi.require_version('UMockdev', '1.0')
from gi.repository import UMockdev

if __name__ == '__main__':
    assert "umockdev" in os.getenv("LD_PRELOAD", "")

    lines: list[str] = subprocess.check_output(['dbus-daemon', '--fork', '--print-address=1', '--print-pid=1', '--session'], universal_newlines=True).strip().splitlines()
    assert len(lines) == 2, "Expected exactly 2 lines of output from dbus-daemon"
    dbus_daemon_pid = lines[1]
    assert int(dbus_daemon_pid) > 0, "Failed to start dbus-daemon"
    os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = lines[0]
    os.environ["DBUS_SESSION_BUS_ADDRESS"] = lines[0]

    testbed = UMockdev.Testbed.new()
    assert testbed.add_from_file(os.path.join(os.path.dirname(os.path.abspath(__file__)), "tablet.umockdev"))
    mock_env = os.environ.copy()
    mock_env["UMOCKDEV_DIR"] = testbed.get_root_dir()
    mock_env["LD_PRELOAD"] = ""
    test_process = subprocess.Popen(["umockdev-run", "selenium-webdriver-at-spi-run", "env", f"DBUS_SYSTEM_BUS_ADDRESS={os.environ['DBUS_SYSTEM_BUS_ADDRESS']}", f"DBUS_SESSION_BUS_ADDRESS={os.environ['DBUS_SESSION_BUS_ADDRESS']}", os.path.join(os.path.dirname(os.path.abspath(__file__)), "kcm_tablet_test.py")], env=mock_env, stdout=sys.stdout, stderr=sys.stderr)

    result: int = 1
    try:
        result = test_process.wait(timeout=60)
    except subprocess.TimeoutExpired as e:
        print("Timeout", e)
    finally:
        test_process.terminate()
        subprocess.check_call(["kill", "-15", dbus_daemon_pid])

    sys.exit(result)
