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

#include "drive.h"

G_DEFINE_TYPE(InstallerDrive, installer_drive, G_TYPE_OBJECT);

static void installer_drive_finalize(GObject *obj);

static void installer_drive_class_init(InstallerDriveClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = installer_drive_finalize;
}

static void installer_drive_init(__attribute((unused)) InstallerDrive *self) {}

static void installer_drive_finalize(GObject *obj) {
    InstallerDrive *self = INSTALLER_DRIVE(obj);

    bd_part_disk_spec_free(self->disk);

    g_free(self->device);
    g_free(self->vendor);
    g_free(self->model);

    g_hash_table_destroy(self->operating_systems);

    g_slist_free_full(g_steal_pointer(&self->esps), (GDestroyNotify) g_free);
    g_free(self->partitions);

    G_OBJECT_CLASS(installer_drive_parent_class)->finalize(obj);
}

static gint sort_swap_partitions(BDPartSpec *a, BDPartSpec *b) {
    gint ret = 0;

    if (a->size > b->size) {
        ret = -1;
    } else if (a->size < b->size) {
        ret = 1;
    }

    return ret;
}

InstallerDrive *installer_drive_new(gchar *device, gchar *disk, gchar *vendor,
                                    gchar *model, GHashTable *ops,
                                    GError **err) {
    InstallerDrive *self = g_object_new(INSTALLER_TYPE_DRIVE, NULL);

    g_return_val_if_fail(INSTALLER_IS_DRIVE(self), NULL);

    BDPartDiskSpec *disk_spec = bd_part_get_disk_spec(disk, err);
    if (!disk_spec) {
        g_object_unref(self);
        return NULL;
    }

    self->disk = disk_spec;

    self->device = device;
    self->vendor = vendor;
    self->model = model;
    self->operating_systems = ops;

    return self;
}

GSList *installer_drive_get_swap_partitions(InstallerDrive *self,
                                            GError **err) {
    GSList *parts = NULL;
    BDPartSpec **bd_parts = NULL;
    BDPartSpec *part_spec = NULL;
    int i = 0;

    g_return_val_if_fail(self->disk != NULL, parts);

    bd_parts = bd_part_get_disk_parts(self->device, err);

    g_return_val_if_fail(bd_parts != NULL, parts);

    while ((part_spec = bd_parts[i]) != NULL) {
        if (part_spec->flags & BD_PART_FLAG_SWAP) {
            parts = g_slist_insert_sorted(parts, bd_part_spec_copy(part_spec),
                                          (GCompareFunc) sort_swap_partitions);
        }

        i++;
    }

    g_free(bd_parts);
    return parts;
}

gchar *installer_drive_get_display_string(InstallerDrive *self) {
    g_autofree gchar *size_str = g_format_size(self->disk->size);
    return g_strdup_printf("%s %s (%s)", self->model, size_str, self->disk->path);
}

const gchar *installer_drive_get_disk_type(InstallerDrive *self) {
    return bd_part_get_part_table_type_str(self->disk->table_type, NULL);
}
