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

#ifndef INSTALLER_PARTITION_H
#define INSTALLER_PARTITION_H

#include <blockdev/fs.h>
#include <blockdev/part.h>
#include <sys/statvfs.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_PARTITION (installer_partition_get_type())

G_DECLARE_FINAL_TYPE(InstallerPartition, installer_partition, INSTALLER,
                     PARTITION, GObject)

/**
 * Create a new partition wrapper struct.
 *
 * Some of the fields of the partition wrapper are populated using functions
 * that could return an error. In such a case, `NULL` will be returned and `err`
 * will be set.
 *
 * Free the returned struct with `g_object_unref()`.
 */
InstallerPartition *installer_partition_new(const gchar *disk,
                                            const gchar *part,
                                            gchar *mount_point, GError **err);

/**
 * Get the disk this partition is on.
 *
 * The caller is responsible for freeing this string.
 */
const gchar *installer_partition_get_disk(InstallerPartition *self);

/**
 * Get the name of this partition.
 *
 * The caller is responsible for freeing this string.
 */
const gchar *installer_partition_get_partition(InstallerPartition *self);

/**
 * Get the path to this partition.
 *
 * The caller is responsible for freeing this string.
 */
const gchar *installer_partition_get_path(InstallerPartition *self);

/**
 * Get whether or not this partition can be resized.
 */
gboolean installer_partition_is_resizable(InstallerPartition *self);

/**
 * Get the amount of free space left on this partition.
 */
guint64 installer_partition_get_freespace(InstallerPartition *self);

/**
 * Get the total space on this partition.
 */
guint64 installer_partition_get_totalspace(InstallerPartition *self);

/**
 * Get the amount of used space on this partition.
 */
guint64 installer_partition_get_usedspace(InstallerPartition *self);

/**
 * Get the size of this partition.
 */
guint64 installer_partition_get_size(InstallerPartition *self);

/**
 * Set the disk this partition is on.
 *
 * This value cannot be `NULL` or empty.
 */
void installer_partition_set_disk(InstallerPartition *self, const gchar *value);

/**
 * Set the name of this partition.
 *
 * This value cannot be `NULL` or empty.
 */
void installer_partition_set_partition(InstallerPartition *self,
                                       const gchar *value);

G_END_DECLS

#endif
