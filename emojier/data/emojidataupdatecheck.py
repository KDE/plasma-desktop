#!/usr/bin/env python3
"""
SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
SPDX-License-Identifier: LGPL-2.0-or-later
"""

from typing import Final
import requests

EMOJI_VERSION: Final = "15.0"
EMOJI_VERSION_CHECK_URL: Final = "https://www.unicode.org/Public/emoji/latest/emoji-sequences.txt"

CLDR_VERSION: Final = "43.0"
CLDR_VERSION_CHECK_URL: Final = "https://api.github.com/repos/unicode-org/cldr/releases/latest"


def check_emoji_version_is_latest() -> bool:
    """
    Checks if the current emoji test file version is the latest
    """
    response = requests.get(EMOJI_VERSION_CHECK_URL, stream=True, timeout=10)
    if response.status_code != 200:
        print(f"Network error {response.status_code}")
        return False

    latest_emoji_version: None | float = None
    for line in response.iter_lines():
        if line.startswith(b"# Version:"):
            line_data: list[str] = line.decode("utf-8").strip().split(": ")
            if len(line_data) != 2:
                print("Invalid version string from emoji data")
                return False

            latest_emoji_version = float(line_data[1])
            break

    if not latest_emoji_version:
        print("No version string in emoji data")
        return False

    print(f"The latest emoji data version is {latest_emoji_version}")

    return latest_emoji_version == float(EMOJI_VERSION)


def check_cldr_version_is_latest() -> bool:
    """
    Checks if the current CLDR data version is the latest
    """
    response = requests.get(CLDR_VERSION_CHECK_URL, timeout=10)
    if response.status_code != 200:
        print(f"Network error {response.status_code}")
        return False

    json_dict: dict = response.json()
    if not "tag_name" in json_dict:
        print("No tag_name in GitHub JSON")
        return False

    latest_release_version: list[str] = json_dict["tag_name"].split("-")
    if len(latest_release_version) < 2 or latest_release_version[0] != "release":
        print("Invalid version string from GitHub JSON")
        return False
    latest_release_version_number_major: int = int(latest_release_version[1])
    latest_release_version_number: float = latest_release_version_number_major + (int(latest_release_version[2]) / 10 if len(latest_release_version) == 3 else 0)

    # Sometimes CLDR releases a minor version without updating the data (e.g. 43.0 and 43.1), so check the real latest version in CLDR_URL
    if latest_release_version_number_major < latest_release_version_number:
        data_exist: bool = False
        while latest_release_version_number >= latest_release_version_number_major:
            response = requests.head(f"https://unicode.org/Public/cldr/{latest_release_version_number_major}/cldr-common-{latest_release_version_number}.zip", timeout=10)
            if response.status_code == 200:
                data_exist = True
                break;
            latest_release_version_number -= 0.1

        if not data_exist:
            print("No CLDR data to download")
            return False

    print(f"The latest CLDR data version is {latest_release_version_number}")

    return latest_release_version_number == float(CLDR_VERSION)


if  __name__ == '__main__':
    emoji_is_latest: bool = check_emoji_version_is_latest()
    cldr_is_latest: bool = check_cldr_version_is_latest()

    exit(0 if emoji_is_latest and cldr_is_latest else 1)
