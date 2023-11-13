/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.import org.kde.kirigami 2.20 as Kirigami

const iconMargin = Math.round(Kirigami.Units.smallSpacing / 4);
const labelMargin = Kirigami.Units.smallSpacing;

function horizontalMargins() {
    const spacingAdjustment = (tasks.plasmoid.pluginName === "org.kde.plasma.icontasks") ? (Kirigami.Settings.tabletMode ? 3 : tasks.plasmoid.configuration.iconSpacing) : 1
    return (taskFrame.margins.left + taskFrame.margins.right) * (tasks.vertical ? 1 : spacingAdjustment);
}

function verticalMargins() {
    const spacingAdjustment = (tasks.plasmoid.pluginName === "org.kde.plasma.icontasks") ? (Kirigami.Settings.tabletMode ? 3 : tasks.plasmoid.configuration.iconSpacing) : 1
    return (taskFrame.margins.top + taskFrame.margins.bottom) * (tasks.vertical ? spacingAdjustment : 1);
}

function adjustMargin(height, margin) {
    var available = height - verticalMargins();

    if (available < Kirigami.Units.iconSizes.small) {
        return Math.floor((margin * (Kirigami.Units.iconSizes.small / available)) / 3);
    }

    return margin;
}

function launcherLayoutTasks() {
    return Math.round(tasksModel.logicalLauncherCount / Math.floor(preferredMinWidth() / launcherWidth()));
}

function launcherLayoutWidthDiff() {
    return (launcherLayoutTasks() * taskWidth()) - (tasksModel.logicalLauncherCount * launcherWidth());
}

function logicalTaskCount() {
    var count = (tasksModel.count - tasksModel.logicalLauncherCount) + launcherLayoutTasks();

    return Math.max(tasksModel.count ? 1 : 0, count);
}

function maxStripes() {
    var length = tasks.vertical ? tasks.width : tasks.height;
    var minimum = tasks.vertical ? preferredMinWidth() : preferredMinHeight();

    return Math.min(tasks.plasmoid.configuration.maxStripes, Math.max(1, Math.floor(length / minimum)));
}

function tasksPerStripe() {
    if (tasks.plasmoid.configuration.forceStripes) {
        return Math.ceil(logicalTaskCount() / maxStripes());
    } else {
        var length = tasks.vertical ? taskList.height : taskList.width;
        var minimum = tasks.vertical ? preferredMinHeight() : preferredMinWidth();

        return Math.floor(length / minimum);
    }
}

function calculateStripes() {
    var stripes = tasks.plasmoid.configuration.forceStripes ? tasks.plasmoid.configuration.maxStripes : Math.min(tasks.plasmoid.configuration.maxStripes, Math.ceil(logicalTaskCount() / tasksPerStripe()));

    return Math.min(stripes, maxStripes());
}

function full() {
    return (maxStripes() == calculateStripes());
}

function optimumCapacity(width, height) {
    var length = tasks.vertical ? height : width;
    var maximum = tasks.vertical ? preferredMaxHeight() : preferredMaxWidth();

    if (!tasks.vertical) {
        //  Fit more tasks in this case, that is possible to cut text, before combining tasks.
        return Math.ceil(length / maximum) * maxStripes() + 1;
    }

    return Math.floor(length / maximum) * maxStripes();
}

function layoutWidth() {
    if (tasks.plasmoid.configuration.forceStripes && !tasks.vertical) {
        return Math.min(tasks.width, Math.max(preferredMaxWidth(), tasksPerStripe() * preferredMaxWidth()));
    } else {
        return tasks.width;
    }
}

function layoutHeight() {
    if (tasks.plasmoid.configuration.forceStripes && tasks.vertical) {
        return Math.min(tasks.height, Math.max(preferredMaxHeight(), tasksPerStripe() * preferredMaxHeight()));
    } else {
        return tasks.height;
    }
}

function preferredMinWidth() {
    var width = launcherWidth();

    if (!tasks.vertical && !tasks.iconsOnly) {
      width +=
          (Kirigami.Units.smallSpacing * 2) +
          (Kirigami.Units.gridUnit * 8);
    }

    return width;
}

function preferredMaxWidth() {
    if (tasks.iconsOnly) {
        if (tasks.vertical) {
            return tasks.width + verticalMargins();
        } else {
            return tasks.height + horizontalMargins();
        }
    }

    if (tasks.plasmoid.configuration.groupingStrategy != 0 && !tasks.plasmoid.configuration.groupPopups && !tasks.iconsOnly) {
        return preferredMinWidth();
    }

    return Math.floor(preferredMinWidth() * 1.6);
}

function preferredMinHeight() {
    // TODO FIXME UPSTREAM: Port to proper font metrics for descenders once we have access to them.
    return Kirigami.Units.iconSizes.sizeForLabels + 4;
}

function preferredMaxHeight() {
    if (tasks.vertical) {
      return verticalMargins() +
             Math.min(
                 // Do not allow the preferred icon size to exceed the width of
                 // the vertical task manager.
                 tasks.width,
                 tasks.iconsOnly ? tasks.width :
                    Math.max(
                        Kirigami.Units.iconSizes.sizeForLabels,
                        Kirigami.Units.iconSizes.medium
                    )
             );
    } else {
      return verticalMargins() +
             Math.min(
                 Kirigami.Units.iconSizes.small * 3,
                 Kirigami.Units.iconSizes.sizeForLabels *
                     3);
    }
}

// Returns the number of 'm' characters whose joint width must be available in the task button label
// so that the button text is rendered at all.
function minimumMColumns() {
    return tasks.vertical ? 4 : 5;
}

function taskWidth() {
    if (tasks.vertical) {
        return Math.floor(taskList.width / calculateStripes());
    } else {
        if (full() && Math.max(1, logicalTaskCount()) > tasksPerStripe()) {
            return Math.floor(taskList.width / Math.ceil(logicalTaskCount() / maxStripes()));
        } else {
            return Math.min(preferredMaxWidth(), Math.floor(taskList.width / Math.min(logicalTaskCount(), tasksPerStripe())));
        }
    }
}

function taskHeight() {
    if (tasks.vertical) {
        if (full() && Math.max(1, logicalTaskCount()) > tasksPerStripe()) {
            return Math.floor(taskList.height / Math.ceil(logicalTaskCount() / maxStripes()));
        } else {
            return Math.min(preferredMaxHeight(), Math.floor(taskList.height / Math.min(logicalTaskCount(), tasksPerStripe())));
        }
    } else {
        return Math.floor(taskList.height / calculateStripes());
    }
}

function launcherWidth() {
    var baseWidth = tasks.vertical ? preferredMinHeight() : Math.min(tasks.height, Kirigami.Units.iconSizes.small * 3);

    return (baseWidth + horizontalMargins())
        - (adjustMargin(baseWidth, taskFrame.margins.top) + adjustMargin(baseWidth, taskFrame.margins.bottom));
}

function maximumContextMenuTextWidth() {
  return (Kirigami.Units.iconSizes.sizeForLabels * 28);
}

function layout(container) {
    var item;
    var stripes = calculateStripes();
    var taskCount = tasksModel.count - tasksModel.logicalLauncherCount;
    var width = taskWidth();
    var adjustedWidth = width;
    var height = taskHeight();

    if (!tasks.vertical && stripes == 1 && taskCount)
    {
        var shrink = ((tasksModel.count - tasksModel.logicalLauncherCount) * preferredMaxWidth())
            + (tasksModel.logicalLauncherCount * launcherWidth()) > taskList.width;
        width = Math.min(shrink ? width + Math.floor(launcherLayoutWidthDiff() / taskCount) : width,
            preferredMaxWidth());
    }

    for (var i = 0; i < container.count; ++i) {
        item = container.itemAt(i);

        if (!item) {
            continue;
        }

        adjustedWidth = width;

        if (!tasks.vertical && !tasks.iconsOnly && (tasks.plasmoid.configuration.separateLaunchers || stripes == 1)) {
            if (item.m.IsLauncher
                || (!tasks.plasmoid.configuration.separateLaunchers && item.m.IsStartup && item.m.HasLauncher)) {
                adjustedWidth = launcherWidth();
            } else if (stripes > 1 && i == tasksModel.logicalLauncherCount) {
                adjustedWidth += launcherLayoutWidthDiff();
            }
        }

        item.width = adjustedWidth;
        item.height = height;
        item.visible = true;
    }
}
