#!/usr/bin/env python3
#
# Helper script to generate Emoji dictionary file from unicode data.
#
# SPDX-FileCopyrightText: 2022 Weng Xuetian <wengxt@gmail.com>
# SPDX-License-Identifier: LGPL-2.0-or-later
#
import collections
import requests
import io
import zipfile
import os
import glob
import xml.dom.minidom
from PyQt5.QtCore import QFile, QDataStream, QByteArray, QIODevice, qCompress

EMOJI_VERSION = "17.0.0"
EMOJI_TEST_FILE = "emoji-test.txt"
EMOJI_TEST_URL = f"https://www.unicode.org/Public/{EMOJI_VERSION}/emoji/{EMOJI_TEST_FILE}"

CLDR_VERSION = "48.2" # omit trailing .0 from CLDR versions 47.0+
CLDR_FILE = f"cldr-common-{CLDR_VERSION}.zip"
CLDR_URL = f"https://unicode.org/Public/cldr/{CLDR_VERSION}/{CLDR_FILE}"
CLDR_ANNOTATIONS_DIR = "common/annotations"
CLDR_ANNOTATIONS_DERIVED_DIR = "common/annotationsDerived"

SKIN_TONES = {
    b"light skin tone",
    b"medium-light skin tone",
    b"medium skin tone",
    b"medium-dark skin tone",
    b"dark skin tone",
}

GENDER_SYMBOLS = {
    "\u200D♂️",
    "👨",
    "\u200D♀️",
    "👩",
}

class EmojiAnnotation(object):
    def __init__(self):
        self.description = ""
        self.annotations = []

class EmojiParser(object):
    def __init__(self):
        self.variantMapping = dict()
        self.categoryNames = []
        self.emojis = collections.OrderedDict()

        self.descriptionEmojiMap = {}
        self.skinToneMap = collections.defaultdict(list)
        self.genderMap = collections.defaultdict(lambda: [None, None, None])
        self.genderAndSkinToneVariants = set()

    def addToGenderMap(self, emoji):
        isMale = "👨" in emoji or "\u200D♂️" in emoji
        isFemale = "👩" in emoji or "\u200D♀️" in emoji

        if isMale and isFemale:
            genderIndex = 0
        elif isMale:
            genderIndex = 1
        else:
            genderIndex = 2

        neutralEmoji = emoji.replace("\u200D♂️", "")
        neutralEmoji = neutralEmoji.replace("👨", "🧑")
        neutralEmoji = neutralEmoji.replace("\u200D♀️", "")
        neutralEmoji = neutralEmoji.replace("👩", "🧑")

        if neutralEmoji not in self.emojis:
            return

        self.genderMap[neutralEmoji][genderIndex] = emoji
        self.genderAndSkinToneVariants.add(emoji)

    def getSkinToneVariantsDict(self, emoji):
        if emoji is None:
            return {}

        skinToneVariants = {emoji: self.emojis[emoji]}
        for variant in self.skinToneMap[emoji]:
            skinToneVariants[variant] = self.emojis[variant]
        return skinToneVariants

    def parseEmojiTest(self, emojiTestData):
        descriptionMapping = dict()
        currentGroup = 0;
        GROUP_TAG = b"# group: "
        for line in emojiTestData.split(b"\n"):
            line = line.strip()
            if line.startswith(GROUP_TAG):
                currentGroup += 1;
                # "&" has special meaning in Qt, which does not work well in the UI.
                self.categoryNames.append(line[len(GROUP_TAG):].replace(b" & ", b" and "))
                continue

            if line.startswith(b"#"):
                continue;

            # line format: code points; status # emoji name
            segments = line.split(b";")
            if len(segments) != 2:
                continue
            metadata = segments[1].split(b"#")
            if len(metadata) != 2:
                continue;
            desc = metadata[1].strip().split(b" E", 1)
            if len(desc) != 2:
                continue
            description = desc[1]

            codes = segments[0].strip().split(b" ")
            try:
                emoji = "".join(chr(int(code, 16)) for code in codes)
                status = metadata[0].strip()
                if status == b"fully-qualified":
                    self.emojis[emoji] = currentGroup
                    descriptionMapping[description] = emoji

                    # description format: "version emoji name: modifiers"
                    versionNameDelimiter = description.index(b" ")
                    descriptionWithoutVersion = description[versionNameDelimiter+1:]
                    self.descriptionEmojiMap[descriptionWithoutVersion] = emoji
                else:
                    fullyQualified = descriptionMapping.get(description, None);
                    if fullyQualified:
                        self.variantMapping[emoji] = fullyQualified;
            except e:
                pass

        for description, emoji in self.descriptionEmojiMap.items():
            # description format: "emoji name: modifier 1, modifier 2, ..., modifier n"
            nameAndModifiers = description.split(b": ")
            if len(nameAndModifiers) == 2:
                name, modifiers = nameAndModifiers

                if any(skinTone in modifiers for skinTone in SKIN_TONES):
                    modifiers = modifiers.split(b", ")
                    modifiersWithoutSkinTone = b", ".join(mod for mod in modifiers if mod not in SKIN_TONES and mod != b"person")
                    descriptionWithoutSkinTone = name + (b": " + modifiersWithoutSkinTone if modifiersWithoutSkinTone else b"")

                    try:
                        neutralEmoji = self.descriptionEmojiMap[descriptionWithoutSkinTone]
                        self.skinToneMap[neutralEmoji].append(emoji)
                        self.genderAndSkinToneVariants.add(emoji)
                    except:
                        pass
                elif any(gender in emoji for gender in GENDER_SYMBOLS):
                    self.addToGenderMap(emoji)
            elif any(gender in emoji for gender in GENDER_SYMBOLS):
                self.addToGenderMap(emoji)

        self.genderMap["🧑‍🤝‍🧑"] = ["👫", "👬", "👭"]
        self.genderAndSkinToneVariants.update(["👫", "👬", "👭"])

        sortedEmojis = {}
        for emoji, group in self.emojis.items():
            if emoji in self.genderAndSkinToneVariants:
                pass
            elif emoji in self.genderMap:
                for genderEmoji in [emoji] + self.genderMap[emoji]:
                    sortedEmojis.update(self.getSkinToneVariantsDict(genderEmoji))
            elif emoji in self.skinToneMap:
                sortedEmojis.update(self.getSkinToneVariantsDict(emoji))
            else:
                sortedEmojis[emoji] = group

        self.emojis = sortedEmojis

    def parseCldr(self, cldrList):
        annotations = dict()
        for data in cldrList:
            with xml.dom.minidom.parseString(data) as doc:
                annotationNodes = doc.getElementsByTagName("annotation")
                for annotationNode in annotationNodes:
                    if "cp" not in annotationNode.attributes:
                        continue
                    emoji = annotationNode.attributes["cp"].nodeValue
                    if emoji not in self.emojis:
                        emoji = self.variantMapping.get(emoji, None)
                    if not emoji:
                        continue
                    if len(annotationNode.childNodes) != 1 or annotationNode.childNodes[0].nodeType != xml.dom.minidom.Node.TEXT_NODE:
                        continue
                    if emoji not in annotations:
                        annotations[emoji] = EmojiAnnotation()
                    annotation = annotations[emoji]
                    if "type" in annotationNode.attributes and annotationNode.attributes["type"].nodeValue == "tts":
                        annotation.description = annotationNode.childNodes[0].nodeValue
                    else:
                        annotation.annotations = annotationNode.childNodes[0].nodeValue.split(" | ")
        return annotations

    def writeEmojiCategory(self):
        template_header='''/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
// Generated from emoji-test.txt
#include "emojicategory.h"

#include <KLazyLocalizedString>

const QStringList &getCategoryNames()
{
    static const QStringList names = {'''
        template_foot='''    };
    return names;
}
'''
        content = "\n".join([template_header] + [f"        QString::fromUtf8(kli18nc(\"Emoji Category\", \"{categoryName.decode('utf-8')}\").untranslatedText())," for categoryName in self.categoryNames] + [template_foot])
        with open("../emojicategory.cpp", "w") as f:
            f.write(content)

# Naive sanity check for running it under correct directory
if not os.path.exists("../emojicategory.cpp"):
    print("Please run this script under its own directory")
    exit(1)

print("Removing old *.dict files")
for olddict in glob.glob("*.dict"):
    os.remove(olddict)

parser = EmojiParser()

print(f"Downloading {EMOJI_TEST_URL}")
response = requests.get(EMOJI_TEST_URL)
print(f"Parsing {EMOJI_TEST_FILE}")
parser.parseEmojiTest(response.content)
parser.writeEmojiCategory()

print(f"Downloading {CLDR_URL}")
response = requests.get(CLDR_URL)

with zipfile.ZipFile(io.BytesIO(response.content)) as thezip:
    annotationsFiles = set()
    annotationsDerivedFiles = set()
    for zipinfo in thezip.infolist():
        dirname = os.path.dirname(zipinfo.filename)
        basename = os.path.basename(zipinfo.filename)
        if not basename.endswith(".xml"):
            continue
        if dirname == CLDR_ANNOTATIONS_DIR:
            annotationsFiles.add(basename)
        elif dirname == CLDR_ANNOTATIONS_DERIVED_DIR:
            annotationsDerivedFiles.add(basename)

    files = annotationsFiles.intersection(annotationsDerivedFiles)
    for langfile in files:
        dictfilename = langfile[:-4] + ".dict"
        print(f"Generating {dictfilename}")
        annotations = dict()
        with thezip.open(os.path.join(CLDR_ANNOTATIONS_DIR, langfile)) as annotationsFile, thezip.open(os.path.join(CLDR_ANNOTATIONS_DERIVED_DIR, langfile)) as annotationsDerivedFile:
            annotations = parser.parseCldr([annotationsFile.read(), annotationsDerivedFile.read()])

        filtered_emojis = [(emoji, category) for (emoji, category) in parser.emojis.items() if emoji in annotations and annotations[emoji].description]
        # There's indeed some annotations file with 0 entries.
        if not filtered_emojis:
            print(f"Skipping {dictfilename}")
            continue

        dictfile = QFile(dictfilename)
        if not dictfile.open(QIODevice.WriteOnly):
            continue

        buf = QByteArray()
        stream = QDataStream(buf, QIODevice.WriteOnly)
        stream.setVersion(QDataStream.Qt_5_15)
        stream.setByteOrder(QDataStream.LittleEndian)
        stream.writeUInt32(len(filtered_emojis))
        for emoji, category in filtered_emojis:
            stream << QByteArray(emoji.encode("utf-8"))
            annotation = annotations[emoji]
            stream << QByteArray(annotation.description.encode("utf-8"))
            stream.writeInt32(category)
            # Write QList<QByteArray>
            stream.writeUInt32(len(annotation.annotations))
            for item in annotation.annotations:
                stream << QByteArray(item.encode("utf-8"))
        compressed = qCompress(buf)
        dictfile.write(compressed)
        dictfile.close()

print("Update Finished, please also update CategoryAction.qml with new category name.")
