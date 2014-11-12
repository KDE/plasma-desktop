/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2011 Casian Andrei <skeletk13@gmail.com>
    Copyright (C) 2014 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "devicepreference.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QLabel>
#include <QPointer>
#include <QStandardPaths>

#include <phonon/AudioOutput>
#include <phonon/BackendCapabilities>
#include <phonon/MediaObject>
#include <phonon/VideoWidget>
#include <phonon/globalconfig.h>
#include <phonon/phononnamespace.h>

#include <KLocalizedString>
#include <KMessageBox>

#ifndef METATYPE_QLIST_INT_DEFINED
#define METATYPE_QLIST_INT_DEFINED
// Want this exactly once, see phonondefs_p.h kcm/devicepreference.cpp
Q_DECLARE_METATYPE(QList<int>)
#endif

namespace Phonon {

/*
 * Lists of categories for every device type
 */
static const Category audioOutCategories[] = {
    NoCategory,
    NotificationCategory,
    MusicCategory,
    VideoCategory,
    CommunicationCategory,
    GameCategory,
    AccessibilityCategory,
};

static const CaptureCategory audioCapCategories[] = {
    NoCaptureCategory,
    CommunicationCaptureCategory,
    RecordingCaptureCategory,
    ControlCaptureCategory
};

static const CaptureCategory videoCapCategories[] = {
    NoCaptureCategory,
    CommunicationCaptureCategory,
    RecordingCaptureCategory,
};

static const int audioOutCategoriesCount = sizeof(audioOutCategories) / sizeof(Category);
static const int audioCapCategoriesCount = sizeof(audioCapCategories) / sizeof(CaptureCategory);
static const int videoCapCategoriesCount = sizeof(videoCapCategories) / sizeof(CaptureCategory);

void operator++(Category &c)
{
    c = static_cast<Category>(1 + static_cast<int>(c));
    //Q_ASSERT(c <= LastCategory);
}

class CategoryItem : public QStandardItem {
public:
    CategoryItem(Category cat)
        : QStandardItem(),
          m_cat(cat),
          m_odtype(AudioOutputDeviceType)
    {
        if (cat == NoCategory) {
            setText(i18n("Audio Playback"));
        } else {
            setText(categoryToString(cat));
        }
    }

    CategoryItem(CaptureCategory cat, ObjectDescriptionType t = AudioCaptureDeviceType)
        : QStandardItem(),
          m_capcat(cat),
          m_odtype(t)
    {
        if (cat == NoCaptureCategory) {
            switch(t) {
            case AudioCaptureDeviceType:
                setText(i18n("Audio Recording"));
                break;
            case VideoCaptureDeviceType:
                setText(i18n("Video Recording"));
                break;
            default:
                setText(i18n("Invalid"));
            }
        } else {
            setText(categoryToString(cat));
        }
    }

    int type() const { return 1001; }
    Category category() const { return m_cat; }
    CaptureCategory captureCategory() const { return m_capcat; }
    ObjectDescriptionType odtype() const { return m_odtype; }

private:
    Category m_cat;
    CaptureCategory m_capcat;
    ObjectDescriptionType m_odtype;
};

/**
 * Need this to change the colors of the ListView if the Palette changed. With CSS set this won't
 * change automatically
 */
void DevicePreference::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::PaletteChange) {
        deviceList->setStyleSheet(deviceList->styleSheet());
    }
}

DevicePreference::DevicePreference(QWidget *parent)
    : QWidget(parent),
      m_headerModel(0, 1, 0),
      m_media(NULL), m_audioOutput(NULL), m_videoWidget(NULL)
{
    setupUi(this);

    // Setup the buttons
    testPlaybackButton->setIcon(QIcon::fromTheme("media-playback-start"));
    testPlaybackButton->setEnabled(false);
    testPlaybackButton->setToolTip(i18n("Test the selected device"));
    deferButton->setIcon(QIcon::fromTheme("go-down"));
    preferButton->setIcon(QIcon::fromTheme("go-up"));

    // Configure the device list
    deviceList->setDragDropMode(QAbstractItemView::InternalMove);
    deviceList->setStyleSheet(QString("QTreeView {"
                                      "background-color: palette(base);"
                                      "background-image: url(%1);"
                                      "background-position: bottom left;"
                                      "background-attachment: fixed;"
                                      "background-repeat: no-repeat;"
                                      "background-clip: padding;"
                                      "}")
                              .arg(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kcm_phonon/listview-background.png")));
    deviceList->setAlternatingRowColors(false);

    // The root item for the categories
    QStandardItem *parentItem = m_categoryModel.invisibleRootItem();

    // Audio Output Parent
    QStandardItem *aOutputItem = new CategoryItem(NoCategory);
    m_audioOutputModel[NoCategory] = new AudioOutputDeviceModel(this);
    aOutputItem->setEditable(false);
    aOutputItem->setToolTip(i18n("Defines the default ordering of devices which can be overridden by individual categories."));
    parentItem->appendRow(aOutputItem);

    // Audio Capture Parent
    QStandardItem *aCaptureItem = new CategoryItem(NoCaptureCategory, AudioCaptureDeviceType);
    m_audioCaptureModel[NoCaptureCategory] = new AudioCaptureDeviceModel(this);
    aCaptureItem->setEditable(false);
    aCaptureItem->setToolTip(i18n("Defines the default ordering of devices which can be overridden by individual categories."));
    parentItem->appendRow(aCaptureItem);

    // Video Capture Parent
    QStandardItem *vCaptureItem = new CategoryItem(NoCaptureCategory, VideoCaptureDeviceType);
    m_videoCaptureModel[NoCaptureCategory] = new VideoCaptureDeviceModel(this);
    vCaptureItem->setEditable(false);
    vCaptureItem->setToolTip(i18n("Defines the default ordering of devices which can be overridden by individual categories."));
    parentItem->appendRow(vCaptureItem);

    // Audio Output Children
    parentItem = aOutputItem;
    for (int i = 1; i < audioOutCategoriesCount; ++i) { // i == 1 to skip NoCategory
        m_audioOutputModel[audioOutCategories[i]] = new AudioOutputDeviceModel(this);
        QStandardItem *item = new CategoryItem(audioOutCategories[i]);
        item->setEditable(false);
        parentItem->appendRow(item);
    }

    // Audio Capture Children
    parentItem = aCaptureItem;
    for (int i = 1; i < audioCapCategoriesCount; ++i) { // i == 1 to skip NoCategory
        m_audioCaptureModel[audioCapCategories[i]] = new AudioCaptureDeviceModel(this);
        QStandardItem *item = new CategoryItem(audioCapCategories[i], AudioCaptureDeviceType);
        item->setEditable(false);
        parentItem->appendRow(item);
    }

    // Video Capture Children
    parentItem = vCaptureItem;
    for (int i = 1; i < videoCapCategoriesCount; ++i) { // i == 1 to skip NoCategory
        m_videoCaptureModel[videoCapCategories[i]] = new VideoCaptureDeviceModel(this);
        QStandardItem *item = new CategoryItem(videoCapCategories[i], VideoCaptureDeviceType);
        item->setEditable(false);
        parentItem->appendRow(item);
    }

    // Configure the category tree
    categoryTree->setModel(&m_categoryModel);
    if (categoryTree->header()) {
        categoryTree->header()->hide();
    }
    categoryTree->expandAll();

    connect(categoryTree->selectionModel(), SIGNAL(currentChanged(const QModelIndex &,const QModelIndex &)),
            SLOT(updateDeviceList()));

    // Connect all model data change signals to the changed slot
    for (int i = -1; i <= LastCategory; ++i) {
        connect(m_audioOutputModel[i], SIGNAL(rowsInserted(QModelIndex, int, int)), this, SIGNAL(changed()));
        connect(m_audioOutputModel[i], SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SIGNAL(changed()));
        connect(m_audioOutputModel[i], SIGNAL(layoutChanged()), this, SIGNAL(changed()));
        connect(m_audioOutputModel[i], SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SIGNAL(changed()));
        if (m_audioCaptureModel.contains(i)) {
            connect(m_audioCaptureModel[i], SIGNAL(rowsInserted(QModelIndex, int, int)), this, SIGNAL(changed()));
            connect(m_audioCaptureModel[i], SIGNAL(rowsRemoved(QModelIndex , int, int)), this, SIGNAL(changed()));
            connect(m_audioCaptureModel[i], SIGNAL(layoutChanged()), this, SIGNAL(changed()));
            connect(m_audioCaptureModel[i], SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SIGNAL(changed()));
        }
        if (m_videoCaptureModel.contains(i)) {
            connect(m_videoCaptureModel[i], SIGNAL(rowsInserted(QModelIndex, int, int)), this, SIGNAL(changed()));
            connect(m_videoCaptureModel[i], SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SIGNAL(changed()));
            connect(m_videoCaptureModel[i], SIGNAL(layoutChanged()), this, SIGNAL(changed()));
            connect(m_videoCaptureModel[i], SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SIGNAL(changed()));
        }
    }

    connect(showAdvancedDevicesCheckBox, &QCheckBox::stateChanged, this, &DevicePreference::changed);

    // Connect the signals from Phonon that notify changes in the device lists
    connect(BackendCapabilities::notifier(), SIGNAL(availableAudioOutputDevicesChanged()), SLOT(updateAudioOutputDevices()));
    connect(BackendCapabilities::notifier(), SIGNAL(availableAudioCaptureDevicesChanged()), SLOT(updateAudioCaptureDevices()));
    connect(BackendCapabilities::notifier(), SIGNAL(availableVideoCaptureDevicesChanged()), SLOT(updateVideoCaptureDevices()));
    connect(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()), SLOT(updateAudioOutputDevices()));
    connect(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()), SLOT(updateAudioCaptureDevices()));
    connect(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()), SLOT(updateVideoCaptureDevices()));

    if (!categoryTree->currentIndex().isValid()) {
        categoryTree->setCurrentIndex(m_categoryModel.index(0, 0).child(1, 0));
    }
}

DevicePreference::~DevicePreference()
{
    // Ensure that the video widget is destroyed, if it remains active
    delete m_videoWidget;
}

void DevicePreference::updateDeviceList()
{
    // Temporarily disconnect the device list selection model
    if (deviceList->selectionModel()) {
        disconnect(deviceList->selectionModel(),
                   SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),
                   this, SLOT(updateButtonsEnabled()));
    }

    // Get the current selected category item
    QStandardItem *currentItem = m_categoryModel.itemFromIndex(categoryTree->currentIndex());
    if (currentItem && currentItem->type() == 1001) {
        CategoryItem *catItem = static_cast<CategoryItem *>(currentItem);
        bool cap = catItem->odtype() != AudioOutputDeviceType;
        const Category cat = catItem->category();
        const CaptureCategory capcat = catItem->captureCategory();

        // Update the device list, by setting it's model to the one for the corresponding category
        switch (catItem->odtype()) {
        case AudioOutputDeviceType:
            deviceList->setModel(m_audioOutputModel[cat]);
            break;
        case AudioCaptureDeviceType:
            deviceList->setModel(m_audioCaptureModel[capcat]);
            break;
        case VideoCaptureDeviceType:
            deviceList->setModel(m_videoCaptureModel[capcat]);
            break;
        default: ;
        }

        // Update the header
        if (cap ? capcat == NoCaptureCategory : cat == NoCategory) {
            switch (catItem->odtype()) {
            case AudioOutputDeviceType:
                m_headerModel.setHeaderData(0, Qt::Horizontal, i18n("Default Audio Playback Device Preference"), Qt::DisplayRole);
                break;
            case AudioCaptureDeviceType:
                m_headerModel.setHeaderData(0, Qt::Horizontal, i18n("Default Audio Recording Device Preference"), Qt::DisplayRole);
                break;
            case VideoCaptureDeviceType:
                m_headerModel.setHeaderData(0, Qt::Horizontal, i18n("Default Video Recording Device Preference"), Qt::DisplayRole);
                break;
            default: ;
            }
        } else {
            switch (catItem->odtype()) {
            case AudioOutputDeviceType:
                m_headerModel.setHeaderData(0, Qt::Horizontal, i18n("Audio Playback Device Preference for the '%1' Category",
                                                                    categoryToString(cat)), Qt::DisplayRole);
                break;
            case AudioCaptureDeviceType:
                m_headerModel.setHeaderData(0, Qt::Horizontal, i18n("Audio Recording Device Preference for the '%1' Category",
                                                                    categoryToString(capcat)), Qt::DisplayRole);
                break;
            case VideoCaptureDeviceType:
                m_headerModel.setHeaderData(0, Qt::Horizontal, i18n("Video Recording Device Preference for the '%1' Category ",
                                                                    categoryToString(capcat)), Qt::DisplayRole);
                break;
            default: ;
            }
        }
    } else {
        // No valid category selected
        m_headerModel.setHeaderData(0, Qt::Horizontal, QString(), Qt::DisplayRole);
        deviceList->setModel(0);
    }

    // Update the header, the buttons enabled state
    deviceList->header()->setModel(&m_headerModel);
    updateButtonsEnabled();

    // Reconnect the device list selection model
    if (deviceList->selectionModel()) {
        connect(deviceList->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),
                this, SLOT(updateButtonsEnabled()));
    }

    deviceList->resizeColumnToContents(0);
}

void DevicePreference::updateAudioCaptureDevices()
{
    const QList<AudioCaptureDevice> list = availableAudioCaptureDevices();
    QHash<int, AudioCaptureDevice> hash;
    foreach (const AudioCaptureDevice &dev, list) {
        hash.insert(dev.index(), dev);
    }

    for (int catIndex = 0; catIndex < audioCapCategoriesCount; ++ catIndex) {
        const int i = audioCapCategories[catIndex];
        AudioCaptureDeviceModel *model = m_audioCaptureModel.value(i);
        Q_ASSERT(model);

        QHash<int, AudioCaptureDevice> hashCopy(hash);
        QList<AudioCaptureDevice> orderedList;

        if (model->rowCount() > 0) {
            QList<int> order = model->tupleIndexOrder();
            foreach (int idx, order) {
                if (hashCopy.contains(idx)) {
                    orderedList << hashCopy.take(idx);
                }
            }

            if (hashCopy.size() > 1) {
                // keep the order of the original list
                foreach (const AudioCaptureDevice &dev, list) {
                    if (hashCopy.contains(dev.index())) {
                        orderedList << hashCopy.take(dev.index());
                    }
                }
            } else if (hashCopy.size() == 1) {
                orderedList += hashCopy.values();
            }

            model->setModelData(orderedList);
        } else {
            model->setModelData(list);
        }
    }

    deviceList->resizeColumnToContents(0);
}

void DevicePreference::updateVideoCaptureDevices()
{
    const QList<VideoCaptureDevice> list = availableVideoCaptureDevices();
    QHash<int, VideoCaptureDevice> hash;
    foreach (const VideoCaptureDevice &dev, list) {
        hash.insert(dev.index(), dev);
    }

    for (int catIndex = 0; catIndex < videoCapCategoriesCount; ++ catIndex) {
        const int i = videoCapCategories[catIndex];
        VideoCaptureDeviceModel *model = m_videoCaptureModel.value(i);
        Q_ASSERT(model);

        QHash<int, VideoCaptureDevice> hashCopy(hash);
        QList<VideoCaptureDevice> orderedList;

        if (model->rowCount() > 0) {
            QList<int> order = model->tupleIndexOrder();
            foreach (int idx, order) {
                if (hashCopy.contains(idx)) {
                    orderedList << hashCopy.take(idx);
                }
            }

            if (hashCopy.size() > 1) {
                // keep the order of the original list
                foreach (const VideoCaptureDevice &dev, list) {
                    if (hashCopy.contains(dev.index())) {
                        orderedList << hashCopy.take(dev.index());
                    }
                }
            } else if (hashCopy.size() == 1) {
                orderedList += hashCopy.values();
            }

            model->setModelData(orderedList);
        } else {
            model->setModelData(list);
        }
    }

    deviceList->resizeColumnToContents(0);
}

void DevicePreference::updateAudioOutputDevices()
{
    const QList<AudioOutputDevice> list = availableAudioOutputDevices();
    QHash<int, AudioOutputDevice> hash;
    foreach (const AudioOutputDevice &dev, list) {
        hash.insert(dev.index(), dev);
    }

    for (int catIndex = 0; catIndex < audioOutCategoriesCount; ++ catIndex) {
        const int i = audioOutCategories[catIndex];
        AudioOutputDeviceModel *model = m_audioOutputModel.value(i);
        Q_ASSERT(model);

        QHash<int, AudioOutputDevice> hashCopy(hash);
        QList<AudioOutputDevice> orderedList;

        if (model->rowCount() > 0) {
            QList<int> order = model->tupleIndexOrder();
            foreach (int idx, order) {
                if (hashCopy.contains(idx)) {
                    orderedList << hashCopy.take(idx);
                }
            }

            if (hashCopy.size() > 1) {
                // keep the order of the original list
                foreach (const AudioOutputDevice &dev, list) {
                    if (hashCopy.contains(dev.index())) {
                        orderedList << hashCopy.take(dev.index());
                    }
                }
            } else if (hashCopy.size() == 1) {
                orderedList += hashCopy.values();
            }

            model->setModelData(orderedList);
        } else {
            model->setModelData(list);
        }
    }

    deviceList->resizeColumnToContents(0);
}

QList<AudioOutputDevice> DevicePreference::availableAudioOutputDevices() const
{
    return BackendCapabilities::availableAudioOutputDevices();
}

QList<AudioCaptureDevice> DevicePreference::availableAudioCaptureDevices() const
{
#ifndef PHONON_NO_AUDIOCAPTURE
    return BackendCapabilities::availableAudioCaptureDevices();
#else
    return QList<AudioCaptureDevice>();
#endif
}

QList<VideoCaptureDevice> DevicePreference::availableVideoCaptureDevices() const
{
#ifndef PHONON_NO_VIDEOCAPTURE
    return BackendCapabilities::availableVideoCaptureDevices();
#else
    return QList<VideoCaptureDevice>();
#endif
}

void DevicePreference::load()
{
    showAdvancedDevicesCheckBox->setChecked(!GlobalConfig().hideAdvancedDevices());
    loadCategoryDevices();
}

void DevicePreference::loadCategoryDevices()
{
    // "Load" the settings from the backend.
    for (int i = 0; i < audioOutCategoriesCount; ++ i) {
        const Category cat = audioOutCategories[i];
        QList<AudioOutputDevice> list;
        const QList<int> deviceIndexes = GlobalConfig().audioOutputDeviceListFor(cat);
        foreach (int i, deviceIndexes) {
            list.append(AudioOutputDevice::fromIndex(i));
        }

        m_audioOutputModel[cat]->setModelData(list);
    }

#ifndef PHONON_NO_AUDIOCAPTURE
    for (int i = 0; i < audioCapCategoriesCount; ++ i) {
        const CaptureCategory cat = audioCapCategories[i];
        QList<AudioCaptureDevice> list;
        const QList<int> deviceIndexes = GlobalConfig().audioCaptureDeviceListFor(cat);
        foreach (int i, deviceIndexes) {
            list.append(AudioCaptureDevice::fromIndex(i));
        }

        m_audioCaptureModel[cat]->setModelData(list);
    }
#endif

#ifndef PHONON_NO_VIDEOCAPTURE
    for (int i = 0; i < videoCapCategoriesCount; ++ i) {
        const CaptureCategory cat = videoCapCategories[i];
        QList<VideoCaptureDevice> list;
        const QList<int> deviceIndexes = GlobalConfig().videoCaptureDeviceListFor(cat);
        foreach (int i, deviceIndexes) {
            list.append(VideoCaptureDevice::fromIndex(i));
        }

        m_videoCaptureModel[cat]->setModelData(list);
    }
#endif

    deviceList->resizeColumnToContents(0);
}

void DevicePreference::save()
{
    for (int i = 0; i < audioOutCategoriesCount; ++i) {
        const Category cat = audioOutCategories[i];
        Q_ASSERT(m_audioOutputModel.value(cat));
        const QList<int> order = m_audioOutputModel.value(cat)->tupleIndexOrder();
        GlobalConfig().setAudioOutputDeviceListFor(cat, order);
    }

#ifndef PHONON_NO_AUDIOCAPTURE
    for (int i = 0; i < audioCapCategoriesCount; ++i) {
        const CaptureCategory cat = audioCapCategories[i];
        Q_ASSERT(m_audioCaptureModel.value(cat));
        const QList<int> order = m_audioCaptureModel.value(cat)->tupleIndexOrder();
        GlobalConfig().setAudioCaptureDeviceListFor(cat, order);
    }
#endif

#ifndef PHONON_NO_VIDEOCAPTURE
    for (int i = 0; i < videoCapCategoriesCount; ++i) {
        const CaptureCategory cat = videoCapCategories[i];
        Q_ASSERT(m_videoCaptureModel.value(cat));
        const QList<int> order = m_videoCaptureModel.value(cat)->tupleIndexOrder();
        GlobalConfig().setVideoCaptureDeviceListFor(cat, order);
    }
#endif
}

void DevicePreference::defaults()
{
    {
        const QList<AudioOutputDevice> list = availableAudioOutputDevices();
        for (int i = 0; i < audioOutCategoriesCount; ++i) {
            m_audioOutputModel[audioOutCategories[i]]->setModelData(list);
        }
    }
    {
        const QList<AudioCaptureDevice> list = availableAudioCaptureDevices();
        for (int i = 0; i < audioCapCategoriesCount; ++i) {
            m_audioCaptureModel[audioCapCategories[i]]->setModelData(list);
        }
    }
    {
        const QList<VideoCaptureDevice> list = availableVideoCaptureDevices();
        for (int i = 0; i < videoCapCategoriesCount; ++i) {
            m_videoCaptureModel[videoCapCategories[i]]->setModelData(list);
        }
    }

    /*
     * Save this list (that contains even hidden devices) to GlobaConfig, and then
     * load them back. All devices that should be hidden will be hidden
     */
    save();
    loadCategoryDevices();

    deviceList->resizeColumnToContents(0);
}

void DevicePreference::pulseAudioEnabled()
{
    showAdvancedDevicesContainer->removeItem(showAdvancedDevicesSpacer);
    delete showAdvancedDevicesSpacer;
    showAdvancedDevicesCheckBox->setVisible(false);
}

void DevicePreference::on_preferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    {
        AudioOutputDeviceModel *deviceModel = dynamic_cast<AudioOutputDeviceModel *>(model);
        if (deviceModel) {
            deviceModel->moveUp(deviceList->currentIndex());
            updateButtonsEnabled();
            emit changed();
        }
    }
    {
        AudioCaptureDeviceModel *deviceModel = dynamic_cast<AudioCaptureDeviceModel *>(model);
        if (deviceModel) {
            deviceModel->moveUp(deviceList->currentIndex());
            updateButtonsEnabled();
            emit changed();
        }
    }
    {
        VideoCaptureDeviceModel *deviceModel = dynamic_cast<VideoCaptureDeviceModel *>(model);
        if (deviceModel) {
            deviceModel->moveUp(deviceList->currentIndex());
            updateButtonsEnabled();
            emit changed();
        }
    }
}

void DevicePreference::on_deferButton_clicked()
{
    QAbstractItemModel *model = deviceList->model();
    {
        AudioOutputDeviceModel *deviceModel = dynamic_cast<AudioOutputDeviceModel *>(model);
        if (deviceModel) {
            deviceModel->moveDown(deviceList->currentIndex());
            updateButtonsEnabled();
            emit changed();
        }
    }
    {
        AudioCaptureDeviceModel *deviceModel = dynamic_cast<AudioCaptureDeviceModel *>(model);
        if (deviceModel) {
            deviceModel->moveDown(deviceList->currentIndex());
            updateButtonsEnabled();
            emit changed();
        }
    }
    {
        VideoCaptureDeviceModel *deviceModel = dynamic_cast<VideoCaptureDeviceModel *>(model);
        if (deviceModel) {
            deviceModel->moveDown(deviceList->currentIndex());
            updateButtonsEnabled();
            emit changed();
        }
    }
}


DevicePreference::DeviceType DevicePreference::shownModelType() const
{
    const QStandardItem *item = m_categoryModel.itemFromIndex(categoryTree->currentIndex());
    if (!item)
        return dtInvalidDevice;
    Q_ASSERT(item->type() == 1001);

    const CategoryItem *catItem = static_cast<const CategoryItem *>(item);
    if (!catItem)
        return dtInvalidDevice;

    switch (catItem->odtype()) {
    case AudioOutputDeviceType:
        return dtAudioOutput;
    case AudioCaptureDeviceType:
        return dtAudioCapture;
    case VideoCaptureDeviceType:
        return dtVideoCapture;
    default:
        return dtInvalidDevice;
    }
}

void DevicePreference::on_applyPreferencesButton_clicked()
{
    const QModelIndex idx = categoryTree->currentIndex();
    const QStandardItem *item = m_categoryModel.itemFromIndex(idx);
    if (!item)
        return;
    Q_ASSERT(item->type() == 1001);

    const CategoryItem *catItem = static_cast<const CategoryItem *>(item);

    QList<AudioOutputDevice> aoPreferredList;
    QList<AudioCaptureDevice> acPreferredList;
    QList<VideoCaptureDevice> vcPreferredList;
    const Category *categoryList = NULL;
    const CaptureCategory *capCategoryList = NULL;
    int categoryListCount;
    int catIndex;
    bool cap = false;

    switch (catItem->odtype()) {
    case AudioOutputDeviceType:
        aoPreferredList = m_audioOutputModel.value(catItem->category())->modelData();
        categoryList = audioOutCategories;
        categoryListCount = audioOutCategoriesCount;
        cap = false;
        break;

    case AudioCaptureDeviceType:
        acPreferredList = m_audioCaptureModel.value(catItem->captureCategory())->modelData();
        capCategoryList = audioCapCategories;
        categoryListCount = audioCapCategoriesCount;
        cap = true;
        break;

    case VideoCaptureDeviceType:
        vcPreferredList = m_videoCaptureModel.value(catItem->captureCategory())->modelData();
        capCategoryList = videoCapCategories;
        categoryListCount = videoCapCategoriesCount;
        cap = true;
        break;

    default:
        return;
    }

    QPointer<QDialog> dialog = new QDialog(this);

    QLabel *label = new QLabel(dialog);
    label->setText(i18n("Apply the currently shown device preference list to the following other "
                        "audio playback categories:"));
    label->setWordWrap(true);

    QListWidget *list = new QListWidget(dialog);

    for (catIndex = 0; catIndex < categoryListCount; catIndex ++) {
        Category cat = cap ? NoCategory : categoryList[catIndex];
        CaptureCategory capcat = cap ? capCategoryList[catIndex] : NoCaptureCategory;

        QListWidgetItem *item = NULL;
        if (cap) {
            if (capcat == NoCaptureCategory) {
                item = new QListWidgetItem(i18n("Default/Unspecified Category"), list, capcat);
            } else {
                item = new QListWidgetItem(categoryToString(capcat), list, capcat);
            }
        } else {
            if (cat == NoCategory) {
                item = new QListWidgetItem(i18n("Default/Unspecified Category"), list, cat);
            } else {
                item = new QListWidgetItem(categoryToString(cat), list, cat);
            }
        }

        item->setCheckState(Qt::Checked);
        if (cat == catItem->category()) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                       | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog.data(), &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(label);
    layout->addWidget(list);
    layout->addWidget(buttonBox);

    switch (dialog->exec()) {
    case QDialog::Accepted:
        for (catIndex = 0; catIndex < categoryListCount; catIndex ++) {
            Category cat = cap ? NoCategory : categoryList[catIndex];
            CaptureCategory capcat = cap ? capCategoryList[catIndex] : NoCaptureCategory;

            if (cap ? capcat != catItem->captureCategory() : cat != catItem->category()) {
                QListWidgetItem *item = list->item(catIndex);
                Q_ASSERT(item->type() == cap ? (int) capcat : (int) cat);
                if (item->checkState() == Qt::Checked) {
                    switch (catItem->odtype()) {
                    case AudioOutputDeviceType:
                        m_audioOutputModel.value(cat)->setModelData(aoPreferredList);
                        break;

                    case AudioCaptureDeviceType:
                        m_audioCaptureModel.value(capcat)->setModelData(acPreferredList);
                        break;

                    case VideoCaptureDeviceType:
                        m_videoCaptureModel.value(capcat)->setModelData(vcPreferredList);
                        break;

                    default: ;
                    }
                }
            }
        }

        emit changed();
        break;

    case QDialog::Rejected:
        // nothing to do
        break;
    }

    delete dialog;
}

void DevicePreference::on_showAdvancedDevicesCheckBox_toggled()
{
    // In order to get the right list from the backend, we need to update the settings now
    // before calling availableAudio{Output,Capture}Devices()
    GlobalConfig().setHideAdvancedDevices(!showAdvancedDevicesCheckBox->isChecked());
    loadCategoryDevices();
}

void DevicePreference::on_testPlaybackButton_toggled(bool down)
{
    if (down) {
        QModelIndex idx = deviceList->currentIndex();
        if (!idx.isValid()) {
            return;
        }

        // Shouldn't happen, but better to be on the safe side
        if (m_testingType != dtInvalidDevice) {
            delete m_media;
            m_media = NULL;
            delete m_audioOutput;
            m_audioOutput = NULL;
            delete m_videoWidget;
            m_videoWidget = NULL;
        }

        // Setup the Phonon objects according to the testing type
        m_testingType = shownModelType();
        switch (m_testingType) {
        case dtAudioOutput: {
            // Create an audio output with the selected device
            m_media = new MediaObject(this);
            const AudioOutputDeviceModel *model = static_cast<const AudioOutputDeviceModel *>(idx.model());
            const AudioOutputDevice &device = model->modelData(idx);
            m_audioOutput = new AudioOutput(this);
            if (!m_audioOutput->setOutputDevice(device)) {
                KMessageBox::error(this, i18n("Failed to set the selected audio output device"));
                break;
            }

            // Just to be very sure that nothing messes our test sound up
            m_audioOutput->setVolume(1.0);
            m_audioOutput->setMuted(false);

            createPath(m_media, m_audioOutput);
            static QUrl testUrl = QUrl::fromLocalFile(QStandardPaths::locate(
                                                          QStandardPaths::GenericDataLocation,
                                                          QStringLiteral("sounds/KDE-Sys-Log-In.ogg")));
            m_media->setCurrentSource(testUrl);
            connect(m_media, &MediaObject::finished, testPlaybackButton, &QToolButton::toggle);

            break;
        }

#ifndef PHONON_NO_AUDIOCAPTURE
        case dtAudioCapture: {
            // Create a media object and an audio output
            m_media = new MediaObject(this);
            m_audioOutput = new AudioOutput(NoCategory, this);

            // Just to be very sure that nothing messes our test sound up
            m_audioOutput->setVolume(1.0);
            m_audioOutput->setMuted(false);

            // Try to create a path
            if (!createPath(m_media, m_audioOutput).isValid()) {
                KMessageBox::error(this, i18n("Your backend may not support audio recording"));
                break;
            }

            // Determine the selected device
            const AudioCaptureDeviceModel *model = static_cast<const AudioCaptureDeviceModel *>(idx.model());
            const AudioCaptureDevice &device = model->modelData(idx);
            m_media->setCurrentSource(device);

            break;
        }
#endif

#ifndef PHONON_NO_VIDEOCAPTURE
        case dtVideoCapture: {
            // Create a media object and a video output
            m_media = new MediaObject(this);
            m_videoWidget = new VideoWidget(NULL);

            // Try to create a path
            if (!createPath(m_media, m_videoWidget).isValid()) {
                KMessageBox::error(this, i18n("Your backend may not support video recording"));
                break;
            }

            // Determine the selected device
            const VideoCaptureDeviceModel *model = static_cast<const VideoCaptureDeviceModel *>(idx.model());
            const VideoCaptureDevice &device = model->modelData(idx);
            m_media->setCurrentSource(device);

            // Set up the testing video widget
            m_videoWidget->setWindowTitle(i18n("Testing %1", device.name()));
            m_videoWidget->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint);
            if (device.property("icon").canConvert(QVariant::String))
                m_videoWidget->setWindowIcon(QIcon::fromTheme(device.property("icon").toString()));
            m_videoWidget->move(QCursor::pos() - QPoint(250, 295));
            m_videoWidget->resize(320, 240);
            m_videoWidget->show();

            break;
        }
#endif

        default:
            return;
        }

        m_media->play();
    } else {
        // Uninitialize the Phonon objects according to the testing type
        switch (m_testingType) {
        case dtAudioOutput:
            disconnect(m_media, &MediaObject::finished, testPlaybackButton, &QToolButton::toggle);
            delete m_media;
            delete m_audioOutput;
            break;

        case dtAudioCapture:
            delete m_media;
            delete m_audioOutput;
            break;

        case dtVideoCapture:
            delete m_media;
            delete m_videoWidget;
            break;

        default:
            return;
        }

        m_media = NULL;
        m_videoWidget = NULL;
        m_audioOutput = NULL;
        m_testingType = dtInvalidDevice;
    }
}

void DevicePreference::updateButtonsEnabled()
{
    if (deviceList->model()) {
        QModelIndex idx = deviceList->currentIndex();
        preferButton->setEnabled(idx.isValid() && idx.row() > 0);
        deferButton->setEnabled(idx.isValid() && idx.row() < deviceList->model()->rowCount() - 1);
        testPlaybackButton->setEnabled(idx.isValid() && (idx.flags() & Qt::ItemIsEnabled));
    } else {
        preferButton->setEnabled(false);
        deferButton->setEnabled(false);
        testPlaybackButton->setEnabled(false);
    }
}

} // Phonon namespace

#include "moc_devicepreference.cpp"
