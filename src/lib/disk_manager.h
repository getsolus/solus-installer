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

#include "os.h"

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

/**
 * Attempt to get the Windows version given a path to the root
 * Windows installation location.
 * 
 * Returns a string for the Windows version, or `NULL` if an
 * error occurred trying to get the version. The caller is 
 * responsible for freeing this string.
 */
gchar *disk_manager_get_windows_version(DiskManager *self, const gchar *path, GError **err);

/**
 * Attempt to get the human-friendly name of the installed
 * Windows bootloader at the given path.
 * 
 * Returns a string with the name of the bootloader, or `NULL`
 * if the boot path does not exist. The caller is responsible
 * for freeing this string.
 */
gchar *disk_manager_get_windows_bootloader(DiskManager *self, const gchar *path);

/**
 * Attempt to get the value of a key from an os-release file.
 * 
 * This reads every line in the os-release file until a matching
 * key is found. If no match is found, `NULL` is returned. If there
 * was an error reading the file, `NULL` is returned and `err` is set.
 * 
 * The caller is responsible for checking that the file at the path
 * exists. If it does not exist, `NULL` will be returned and `err`
 * will be set.
 * 
 * Returns a string containing the value for the key, or `NULL` if
 * the key was not found. The caller is responsible for freeing
 * this string.
 */
gchar *disk_manager_get_os_release_val(const gchar *path, const gchar *find_key, GError **err);

/**
 * Extracts a value from a line in an os-release file if the line's key
 * matches.
 * 
 * Returns a string containing the value for the key, or `NULL` if
 * the key was not found. The caller is responsible for freeing
 * this string.
 */
gchar *disk_manager_match_os_release_line(const gchar *line, const gchar *find_key);

/**
 * Attempts to get the reported Linux version for the installation at
 * the given path.
 * 
 * This first looks in the os_release file, moving on to the lsb_release
 * file if no os_release file is found or the name keys aren't set, respecting
 * stateless heirarchy.
 * 
 * Returns a string containing the reported Linux version, or `NULL` if it
 * could not be found. The caller is responsible for freeing this string.
 */
gchar *disk_manager_get_linux_version(const gchar *path);

/**
 * Attempts to get a value from a file with a key.
 * 
 * This function takes an array of paths to look in to try to find the matching
 * key, falling back to each progressive path if a value is not found. Additionally,
 * the function takes a fallback key in case the file being read exists, but the
 * first key is not present or set.
 * 
 * Returns a string containing the value for the key, or `NULL` if it could not
 * be found. The caller is responsible for freeing this string.
 */
gchar *disk_manager_search_for_key(
    const gchar *root,
    const gchar **paths,
    gint paths_len,
    const gchar *key,
    const gchar *fallback_key
);

gchar *disk_manager_get_os_icon(DiskManager *self, InstallerOS *os);

G_END_DECLS
