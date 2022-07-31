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

#ifndef INSTALLER_DRIVE_H
#define INSTALLER_DRIVE_H

#include <blockdev/part.h>
#include <gio/gio.h>
#include <glib.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_DRIVE (installer_drive_get_type())

G_DECLARE_FINAL_TYPE(InstallerDrive, installer_drive, INSTALLER, DRIVE, GObject)

struct _InstallerDrive {
    GObject parent_instance;

    gchar *device;
    BDPartDiskSpec *disk;

    gchar *vendor;
    gchar *model;
    GHashTable *operating_systems;

    GSList *esps;
    BDPartSpec **partitions;
};

/**
 * insaller_drive_new:
 * @device: The name of the device
 * @disk: The name of the disk
 * @vendor: The vendor of the device
 * @model: The model of the device
 * @ops: A mapping of partition to OSType
 * @err: (out): Place to store an error (if any)
 *
 * Creates a new #InstallerDrive.
 *
 * Returns: The new #InstallerDrive
 */
InstallerDrive *installer_drive_new(gchar *device, gchar *disk, gchar *vendor,
                                    gchar *model, GHashTable *ops,
                                    GError **err);

/**
 * installer_drive_get_swap_partitions:
 * @self: The drive to search in
 * @err: (out): Place to store an error (if any)
 *
 * Gets all of the swap partitions on this drive.
 *
 * Returns: (transfer full): A sorted singly-linked list of swap
 *          partitions on the @self or %NULL in case of error (@error is set)
 */
GSList *installer_drive_get_swap_partitions(InstallerDrive *self, GError **err);

/**
 * installer_drive_get_display_string:
 * @self: The drive to format as a string
 *
 * Allocates a formatted string with information about this
 * drive for use in a UI.
 *
 * Returns: (transfer full): A string containing information about this drive
 *          that is suitable for display.
 */
gchar *installer_drive_get_display_string(InstallerDrive *self);

/**
 * installer_drive_get_disk_type:
 * @self: The drive to get the disk type for
 *
 * Gets the partitioning system of this disk, if it is known.
 *
 * Returns: The partitioning system (MSDOS or GPT) of this disk.
 */
const gchar *installer_drive_get_disk_type(InstallerDrive *self);

G_END_DECLS

#endif
