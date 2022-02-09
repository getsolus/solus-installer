//
// Copyright © 2022 Solus Project <copyright@getsol.us>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include "utils.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_DISK_MANAGER (disk_manager_get_type())

G_DECLARE_FINAL_TYPE(DiskManager, disk_manager, DISK, MANAGER, GObject)

/**
 * Create a new DiskManager object.
 */
DiskManager *disk_manager_new();

/**
 * Scan all partitions on the device and populate the manager's
 * device list.
 */
void disk_manager_scan_parts(DiskManager *self);

/**
 * Append a new device to our list of devices.
 * 
 * The device will only be added if the node exists and has not
 * already been added to the list.
 */
void disk_manager_append_device(DiskManager *self, gchar *device);

/**
 * Check if the given path is on a SSD.
 */
gboolean disk_manager_is_device_ssd(const gchar *path);

/**
 * Check if the rootfs install is supported on this device.
 * 
 * Currently we only support rootfs installs on certain types.
 */
gboolean disk_manager_is_install_supported(const gchar *path);

/**
 * Get a mapping of devices to mount points.
 */
GHashTable *disk_manager_get_mount_points();

/**
 * Mount a device at the specified mount point.
 * 
 * `options` are options to be passed to the mount command
 * as a comma-separated list, or `NULL` if there are none.
 * 
 * This returns `TRUE` if the mount command ran successfully.
 * If it did not, `FALSE` will be returned and `err` will be set.
 */
gboolean disk_manager_mount_device(
    gchar *device,
    const gchar *mpoint,
    gchar *fsystem,
    gchar *options,
    GError **err
);

/**
 * Unmount a device from the specified mount point.
 * 
 * This returns `TRUE` if the umount command ran successfully.
 * If it did not, `FALSE` will be returned and `err` will be set.
 */
gboolean disk_manager_umount_device(const gchar *mpoint, GError **err);

G_END_DECLS
