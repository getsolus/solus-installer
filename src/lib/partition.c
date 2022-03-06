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

#include "partition.h"

enum {
    PROP_EXP_0,
    PROP_DISK,
    PROP_PARTITION,
    N_EXP_PROPERTIES
};

static GParamSpec *props[N_EXP_PROPERTIES] = {NULL};

struct _InstallerPartition {
    GObject parent_instance;

    const gchar *disk;
    const gchar *partition;

    const gchar *path;

    gboolean resizeable;
    guint64 freespace;
    guint64 totalspace;
    guint64 usedspace;
    guint64 size;
};

G_DEFINE_TYPE(InstallerPartition, installer_partition, G_TYPE_OBJECT);

static void installer_partition_finalize(GObject *obj);
static void installer_partition_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void installer_partition_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void installer_partition_class_init(InstallerPartitionClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = installer_partition_finalize;
    class->get_property = installer_partition_get_property;
    class->set_property = installer_partition_set_property;

    props[PROP_DISK] = g_param_spec_string(
        "disk",
        "Disk",
        "Path of the disk the partition is on",
        "",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    props[PROP_PARTITION] = g_param_spec_string(
        "partition",
        "Partition",
        "Name of the partition",
        "",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    g_object_class_install_properties(class, N_EXP_PROPERTIES, props);
}

static void installer_partition_init(InstallerPartition *self) {
    (void) self;
}

static void installer_partition_finalize(GObject *obj) {
    InstallerPartition *self = INSTALLER_PARTITION(obj);

    g_free((gchar *) self->disk);
    g_free((gchar *) self->partition);
    g_free((gchar *) self->path);

    G_OBJECT_CLASS(installer_partition_parent_class)->finalize(obj);
}

static void installer_partition_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    InstallerPartition *self = INSTALLER_PARTITION(obj);

    switch (prop_id)
    {
    case PROP_DISK:
        g_value_set_string(val, installer_partition_get_disk(self));
        break;

    case PROP_PARTITION:
        g_value_set_string(val, installer_partition_get_partition(self));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
        break;
    }
}

static void installer_partition_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    InstallerPartition *self = INSTALLER_PARTITION(obj);

    switch (prop_id)
    {
    case PROP_DISK:
        installer_partition_set_disk(self, g_value_dup_string(val));
        break;

    case PROP_PARTITION:
        installer_partition_set_partition(self, g_value_dup_string(val));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
        break;
    }
}

InstallerPartition *installer_partition_new(
    const gchar *disk,
    const gchar *part,
    gchar *mount_point,
    GError **err
) {
    GObject *obj = g_object_new(
        INSTALLER_TYPE_PARTITION,
        "disk", disk,
        "partition", part,
        NULL
    );

    if (!INSTALLER_IS_PARTITION(obj)) {
        return NULL;
    }

    InstallerPartition *self = INSTALLER_PARTITION(obj);

    // Get the partition spec from libblockdev
    BDPartSpec *part_spec = bd_part_get_part_spec(self->disk, self->partition, err);
    if (!part_spec) {
        g_object_unref(self);
        return NULL;
    }

    // Set our partition size and path
    self->path = g_strdup(part_spec->path);
    self->size = part_spec->size;

    // Figure out if we're resizable
    g_autofree gchar *type = bd_fs_get_fstype(self->path, err);
    if (!installer_is_string_valid(type)) {
        bd_part_spec_free(part_spec);
        g_object_unref(self);
        return NULL;
    }

    self->resizeable = bd_fs_can_resize(type, NULL, NULL, err);
    if (*err) {
        bd_part_spec_free(part_spec);
        g_object_unref(self);
        return NULL;
    }

    // TODO: In the Python version, we just called out to the
    // resize tools directly to get the min_size of the resized
    // partition. libblockdev doesn't seem to have a way to get
    // that, and I really don't want to start a process for it
    // in here. So, evaluate if it's really needed.

    bd_part_spec_free(part_spec);

    // Stat the mount point to get the free/total/used space
    struct statvfs *buf = malloc(sizeof (struct statvfs));
    gint ret = statvfs(mount_point, buf);
    if (ret != 0) {
        g_set_error_literal(err, G_IO_ERROR, errno, "Error stating file system");
        g_object_unref(self);
        return NULL;
    }

    self->freespace = buf->f_bavail * buf->f_frsize;
    self->totalspace = buf->f_blocks * buf->f_frsize;
    self->usedspace = (buf->f_blocks - buf->f_bavail) * buf->f_frsize;

    free(buf);

    return self;
}

const gchar *installer_partition_get_disk(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), NULL);

    return g_strdup(self->disk);
}

const gchar *installer_partition_get_partition(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), NULL);

    return g_strdup(self->partition);
}

const gchar *installer_partition_get_path(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), NULL);

    return g_strdup(self->path);
}

gboolean installer_partition_is_resizable(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), FALSE);

    return self->resizeable;
}

guint64 installer_partition_get_freespace(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), 0);

    return self->freespace;
}

guint64 installer_partition_get_totalspace(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), 0);

    return self->totalspace;
}

guint64 installer_partition_get_usedspace(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), 0);

    return self->usedspace;
}

guint64 installer_partition_get_size(InstallerPartition *self) {
    g_return_val_if_fail(INSTALLER_IS_PARTITION(self), 0);

    return self->size;
}

void installer_partition_set_disk(InstallerPartition *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_PARTITION(self));

    if (!installer_is_string_valid(value)) {
        return;
    }

    if (installer_is_string_valid(self->disk)) {
        g_free((gchar *) self->disk);
    }

    self->disk = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_DISK]);
}

void installer_partition_set_partition(InstallerPartition *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_PARTITION(self));

    if (!installer_is_string_valid(value)) {
        return;
    }

    if (installer_is_string_valid(self->partition)) {
        g_free((gchar *) self->partition);
    }

    self->partition = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_PARTITION]);
}
