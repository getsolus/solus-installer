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

#ifndef INSTALLER_DISK_MANAGER_H
#define INSTALLER_DISK_MANAGER_H

#include "drive.h"
#include "os.h"

#include <blockdev/fs.h>
#include <blockdev/part.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define OS_RELEASE_PATHS_LENGTH 2

/**
 * Array of possible paths to an os-release file, respecting
 * stateless heirarchy.
 */
extern const gchar *os_release_paths[];

#define LSB_RELEASE_PATHS_LENGTH 3

/**
 * Array of possible paths to an lsb-release file, respecting
 * stateless heirarchy.
 */
extern const gchar *lsb_release_paths[];

#define OS_ICONS_LENGTH 22

extern const gchar *os_icons[];

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
 * disk_manager_get_devices:
 * @self: The #DiskManager
 *
 * Gets the current list of detected devices. The returned list
 * must not be freed.
 *
 * Returns: (transfer none): A #GSList of devices on the system
 */
GSList *disk_manager_get_devices(DiskManager *self);

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

gchar *disk_manager_get_disk_model(gchar *device, GError **err);

gchar *disk_manager_get_disk_vendor(gchar *device, GError **err);

InstallerOS *disk_manager_detect_os(DiskManager *self, BDPartSpec *device,
                                    GError **err);

InstallerDrive *disk_manager_parse_system_disk(DiskManager *self, gchar *device,
                                               gchar *disk, GList *mounts,
                                               GError **err);

G_END_DECLS

#endif
