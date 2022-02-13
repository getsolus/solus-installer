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

#include "disk_manager.h"

const gchar *os_release_paths[OS_RELEASE_PATHS_LENGTH] = {
    "etc/os-release",
    "usr/lib/os-release"
};

const gchar *lsb_release_paths[LSB_RELEASE_PATHS_LENGTH] = {
    "etc/lsb-release",
    "usr/lib/lsb-release",
    "usr/share/defaults/etc/lsb-release"
};

struct _DiskManager {
    GObject parent_instance;

    GRegex *re_whole_disk;
    GRegex *re_mmcblk;
    GRegex *re_nvme;
    GRegex *re_raid;

    GSList *devices;

    GHashTable *win_prefixes;
    GHashTable *win_bootloaders;

    gboolean is_uefi;
    gint uefi_fw_size;
    gint host_size;
    GSList *efi_types;

    GSList *os_icons;
};

G_DEFINE_TYPE(DiskManager, disk_manager, G_TYPE_OBJECT);

static void disk_manager_finalize(GObject *obj);

static void disk_manager_class_init(DiskManagerClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = disk_manager_finalize;
}

static void disk_manager_init(DiskManager *self) {
    g_return_if_fail(DISK_IS_MANAGER(self));

    /* Regexes. Gratefully borrowed from gparted, Proc_Partitions_Info.cc */

    self->re_whole_disk = g_regex_new("^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+([^0-9]+)$", 0, 0, NULL);

    self->re_mmcblk = g_regex_new("^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+(mmcblk[0-9]+)$", 0, 0, NULL);

    self->re_nvme = g_regex_new("^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+(nvme[0-9]+n[0-9]+)$", 0, 0, NULL);

    self->re_raid = g_regex_new("^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+(md[0-9]+)$", 0, 0, NULL);

    /* Windows prefixes */

    self->win_prefixes = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(self->win_prefixes, "10.", "Windows 10");
    g_hash_table_insert(self->win_prefixes, "6.3", "Windows 8.1");
    g_hash_table_insert(self->win_prefixes, "6.2", "Windows 8");
    g_hash_table_insert(self->win_prefixes, "6.1", "Windows 7");
    g_hash_table_insert(self->win_prefixes, "6.0", "Windows Vista");
    g_hash_table_insert(self->win_prefixes, "5.2", "Windows XP");
    g_hash_table_insert(self->win_prefixes, "5.1", "Windows XP");
    g_hash_table_insert(self->win_prefixes, "5.0", "Windows 2000");
    g_hash_table_insert(self->win_prefixes, "4.90", "Windows ME");
    g_hash_table_insert(self->win_prefixes, "4.1", "Windows 98");
    g_hash_table_insert(self->win_prefixes, "4.0.1381", "Windows NT");
    g_hash_table_insert(self->win_prefixes, "4.0.950", "Windows 95");

    /* Windows bootloaders */

    self->win_bootloaders = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(self->win_bootloaders, "V.i.s.t.a", "Windows Vista bootloader");
    g_hash_table_insert(self->win_bootloaders, "W.i.n.d.o.w.s. .7", "Windows 7 bootloader");
    g_hash_table_insert(self->win_bootloaders, "W.i.n.d.o.w.s. .R.e.c.o.v.e.r.y. .E.n.v.i.r.o.n.m.e.n.t", "Windows recovery");
    g_hash_table_insert(self->win_bootloaders, "W.i.n.d.o.w.s. .S.e.r.v.e.r. .2.0.0.8", "Windows Server 2008 bootloader");

    /* Set up UEFI knowledge */

    g_autoptr(GFile) efi_file = g_file_new_for_path("/sys/firmware/efi");
    if (g_file_query_exists(efi_file, NULL)) {
        self->is_uefi = TRUE;
        g_autoptr(GFile) platform_file = g_file_new_for_path("/sys/firmware/efi/fw_platform_size");
        if (g_file_query_exists(platform_file, NULL)) {
            g_autoptr(GError) err = NULL;
            g_autofree gchar *size = installer_read_line_full(platform_file, &err);
            if (err) {
                g_critical("Error reading platform file: %s", err->message);
                return;
            }

            if (g_strcmp0(size, "64") == 0) {
                self->uefi_fw_size = 64;
            } else if (g_strcmp0(size, "32") == 0) {
                self->uefi_fw_size = 32;
            } else {
                g_warning("System reported odd FW size: %s", size);
            }
        }
    } else {
        self->is_uefi = FALSE;
    }

    /* Valid EFI types */

    self->efi_types = g_slist_append(self->efi_types, "fat");
    self->efi_types = g_slist_append(self->efi_types, "fat32");
    self->efi_types = g_slist_append(self->efi_types, "fat16");
    self->efi_types = g_slist_append(self->efi_types, "vfat");
    self->efi_types = g_slist_append(self->efi_types, "fat12");

    /* Host size setup (64/32) */

    if (sizeof(void *) * 8 < 64) {
        self->host_size = 32;
    } else {
        self->host_size = 64;
    }

    /* OS icons for populating OsTypes */

    self->os_icons = g_slist_append(self->os_icons, "antergos");
    self->os_icons = g_slist_append(self->os_icons, "archlinux");
    self->os_icons = g_slist_append(self->os_icons, "crunchbang");
    self->os_icons = g_slist_append(self->os_icons, "debian");
    self->os_icons = g_slist_append(self->os_icons, "deepin");
    self->os_icons = g_slist_append(self->os_icons, "edubuntu");
    self->os_icons = g_slist_append(self->os_icons, "elementary");
    self->os_icons = g_slist_append(self->os_icons, "fedora");
    self->os_icons = g_slist_append(self->os_icons, "frugalware");
    self->os_icons = g_slist_append(self->os_icons, "gentoo");
    self->os_icons = g_slist_append(self->os_icons, "kubuntu");
    self->os_icons = g_slist_append(self->os_icons, "linux-mint");
    self->os_icons = g_slist_append(self->os_icons, "mageia");
    self->os_icons = g_slist_append(self->os_icons, "mandriva");
    self->os_icons = g_slist_append(self->os_icons, "manjaro");
    self->os_icons = g_slist_append(self->os_icons, "solus");
    self->os_icons = g_slist_append(self->os_icons, "opensuse");
    self->os_icons = g_slist_append(self->os_icons, "slackware");
    self->os_icons = g_slist_append(self->os_icons, "steamos");
    self->os_icons = g_slist_append(self->os_icons, "ubuntu-gnome");
    self->os_icons = g_slist_append(self->os_icons, "ubuntu-mate");
    self->os_icons = g_slist_append(self->os_icons, "ubuntu");
}

static void disk_manager_finalize(GObject *obj) {
    DiskManager *self = DISK_MANAGER(obj);

    g_regex_unref(self->re_whole_disk);
    g_regex_unref(self->re_mmcblk);
    g_regex_unref(self->re_nvme);
    g_regex_unref(self->re_raid);
    g_slist_free_full(g_steal_pointer(&self->devices), (GDestroyNotify) g_free);
    g_hash_table_destroy(self->win_prefixes);
    g_hash_table_destroy(self->win_bootloaders);
    g_slist_free(g_steal_pointer(&self->efi_types));
    g_slist_free(g_steal_pointer(&self->os_icons));

    G_OBJECT_CLASS(disk_manager_parent_class)->finalize(obj);
}

DiskManager *disk_manager_new() {
    return g_object_new(INSTALLER_TYPE_DISK_MANAGER, NULL);
}

void disk_manager_scan_parts(DiskManager *self) {
    g_return_if_fail(DISK_IS_MANAGER(self));

    // Open and read the system partitions file
    g_autoptr(GFile) partition_file = g_file_new_for_path("/proc/partitions");
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream = g_file_read(partition_file, NULL, &err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        g_warning("Error reading partition file: %s", err->message);
        return;
    }

    g_autoptr(GDataInputStream) data_stream = g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    // Read each line of the partitions file
    g_autofree gchar *line = NULL;
    while ((line = g_data_input_stream_read_line(data_stream, NULL, NULL, &err)) != NULL) {
        if (!line) {
            g_warning("Error reading partition line: %s", err->message);
            break;
        }

        GRegex *groups[4] = {
            self->re_whole_disk,
            self->re_mmcblk,
            self->re_nvme,
            self->re_raid
        };

        // Check each regex for a match
        gint i;
        for (i = 0; i < 4; i++) {
            g_autoptr(GMatchInfo) match_info = NULL;
            if (!g_regex_match(groups[i], line, 0, &match_info)) {
                continue;
            }

            // Found a match, append the device
            g_autofree gchar *device = g_match_info_fetch(match_info, 1);
            disk_manager_append_device(self, device);
        }

        g_free(line);
    }
}

void disk_manager_append_device(DiskManager *self, gchar *device) {
    g_return_if_fail(DISK_IS_MANAGER(self));
    g_return_if_fail(device != NULL);

    g_autofree gchar *path = g_build_path(G_DIR_SEPARATOR_S, "/dev/", device, NULL);

    g_autoptr(GFile) file = g_file_new_for_path(path);
    if (!g_file_query_exists(file, NULL)) {
        g_warning("Trying to add non-existant device: %s", path);
        return;
    }

    if (g_slist_find(self->devices, device) == NULL) {
        self->devices = g_slist_append(self->devices, g_canonicalize_filename(path, "/"));
    }
}

gboolean disk_manager_is_device_ssd(const gchar *path) {
    g_return_val_if_fail(path != NULL, FALSE);

    g_autofree gchar *nodename = g_path_get_basename(path);
    g_autofree gchar *fpath = g_strdup_printf("/sys/block/%s/queue/rotational", nodename);

    g_autoptr(GFile) file = g_file_new_for_path(fpath);
    if (!g_file_query_exists(file, NULL)) {
        return FALSE;
    }

    // Don't try using a SSD with eMMC
    if (strncmp(nodename, "mmcblk", 6)) {
        return FALSE;
    }

    // Open and read the device type file
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, &err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        g_warning("Error reading device type file: %s", err->message);
        return FALSE;
    }

    g_autoptr(GDataInputStream) data_stream = g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    g_autofree gchar *line = g_data_input_stream_read_line(data_stream, NULL, NULL, &err);
    if (g_strcmp0(line, "0") == 0) {
        return TRUE;
    }

    return FALSE;
}

gboolean disk_manager_is_install_supported(const gchar *path) {
    g_return_val_if_fail(path != NULL, FALSE);

    g_autofree gchar *nodename = g_path_get_basename(path);
    if (strncmp(nodename, "md", 2)) {
        return FALSE;
    }
    return TRUE;
}

GHashTable *disk_manager_get_mount_points() {
    GHashTable *ret = g_hash_table_new(g_str_hash, g_str_equal);

    // Open and read the mounts file
    g_autoptr(GFile) mounts_file = g_file_new_for_path("/proc/self/mounts");
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream = g_file_read(mounts_file, NULL, &err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        g_warning("Error reading mounts file: %s", err->message);
        return ret;
    }

    g_autoptr(GDataInputStream) data_stream = g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    // Read each line of the mounts file
    g_autofree gchar *line = NULL;
    while ((line = g_data_input_stream_read_line(data_stream, NULL, NULL, &err)) != NULL) {
        if (strcmp(line, "") == 0) {
            g_free(line);
            continue;
        }

        // Split the line into parts
        g_autofree gchar **parts = g_strsplit(line, " ", -1);

        // Calculate the length of the parts vector
        gsize len = (sizeof parts / sizeof (gchar) - 1);
        if (len < 4) {
            g_free(line);
            continue;
        }

        gchar *dev = parts[0];
        gchar *mount_point = parts[1];

        // We only want block devices
        if (strncmp(dev, "/", 1) == 0) {
            g_hash_table_insert(ret, dev, mount_point);
        }

        g_free(line);
    }

    return ret;
}

gboolean disk_manager_mount_device(
    gchar *device,
    const gchar *mpoint,
    gchar *fsystem,
    gchar *options,
    GError **err
) {
    g_return_val_if_fail(device != NULL, FALSE);
    g_return_val_if_fail(mpoint != NULL, FALSE);
    g_return_val_if_fail(fsystem != NULL, FALSE);

    g_autoptr(GSubprocess) proc = NULL;
    if (options) {
        proc = g_subprocess_new(
            G_SUBPROCESS_FLAGS_NONE,
            err,
            "mount",
            "-t", g_strdup(fsystem),
            g_strdup(device),
            g_strdup(mpoint),
            "-o", g_strdup(options),
            NULL
        );
    } else {
        proc = g_subprocess_new(
            G_SUBPROCESS_FLAGS_NONE,
            err,
            "mount",
            "-t", g_strdup(fsystem),
            g_strdup(device),
            g_strdup(mpoint),
            NULL
        );
    }

    if (!G_IS_SUBPROCESS(proc)) {
        return FALSE;
    }

    return g_subprocess_wait_check(proc, NULL, err);
}

gboolean disk_manager_umount_device(const gchar *mpoint, GError **err) {
    g_return_val_if_fail(mpoint != NULL, FALSE);

    g_autoptr(GSubprocess) proc = g_subprocess_new(
        G_SUBPROCESS_FLAGS_NONE,
        err,
        "umount", g_strdup(mpoint),
        NULL
    );

    if (!G_IS_SUBPROCESS(proc)) {
        return FALSE;
    }

    gint try_count = 0;

    // Try to unmount the device. This will try a maximum of three times,
    // waiting half a second between attempts.
    while (try_count < 3) {
        try_count++;

        if (g_subprocess_wait_check(proc, NULL, NULL)) {
            return TRUE;
        }

        // Wait 0.5s to try again
        g_usleep(500000);
    }

    // Trying to mount so far has failed, so try again
    // with the lazy option
    proc = g_subprocess_new(
        G_SUBPROCESS_FLAGS_NONE,
        err,
        "umount",
        "-l", g_strdup(mpoint),
        NULL
    );

    return g_subprocess_wait_check(proc, NULL, err);
}

gchar *disk_manager_get_windows_version(DiskManager *self, const gchar *path, GError **err) {
    if (!DISK_IS_MANAGER(self)) {
        return NULL;
    }

    g_return_val_if_fail(path != NULL, NULL);

    g_autofree gchar *fpath = g_build_path(G_DIR_SEPARATOR_S, path, "Windows", "servicing", "Version", NULL);
    g_autoptr(GFile) version_file = g_file_new_for_path(fpath);

    // Check if the version file exists. If it doesn't, look for System32
    // to make sure the path really is a Windows path.
    if (!g_file_query_exists(version_file, NULL)) {
        fpath = g_build_path(path, "Windows", "System32", NULL);
        g_autoptr(GFile) system32 = g_file_new_for_path(fpath);

        // Windows is installed, but we can't find out what version it is
        if (g_file_query_exists(system32, NULL)) {
            return "Windows (Unknown)";
        }

        return NULL;
    }

    // Open the Windows version directory
    g_autoptr(GDir) version_dir = g_dir_open(fpath, 0, err);
    if (!version_dir) {
        return NULL;
    }

    const gchar *child = NULL;

    // Iterate over the items in the directory to try to find a match
    // in our Windows prefixes HashTable. If one is found, return the
    // value in the table.
    while ((child = g_dir_read_name(version_dir)) != NULL) {
        gchar *item = g_hash_table_find(self->win_prefixes, (GHRFunc) installer_str_starts_with, (gchar *) child);
        if (item) {
            return g_strdup(item);
        }
    }

    g_dir_close(version_dir);
    return NULL;
}

gchar *disk_manager_get_windows_bootloader(DiskManager *self, const gchar *path) {
    if (!DISK_IS_MANAGER(self)) {
        return NULL;
    }

    g_return_val_if_fail(path != NULL, NULL);

    g_autofree gchar *fpath = g_build_path(G_DIR_SEPARATOR_S, path, "Boot", "BCD", NULL);
    g_autoptr(GFile) file = g_file_new_for_path(fpath);

    if (!g_file_query_exists(file, NULL)) {
        return NULL;
    }

    gchar *item = g_hash_table_find(self->win_bootloaders, (GHRFunc) installer_str_contains, fpath);
    if (item) {
        return g_strdup(item);
    }

    return "Windows bootloader";
}

gchar *disk_manager_get_os_release_val(const gchar *path, const gchar *find_key, GError **err) {
    g_return_val_if_fail(path != NULL, NULL);
    g_return_val_if_fail(find_key != NULL, NULL);

    g_autoptr(GFile) file = g_file_new_for_path(path);
    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        return NULL;
    }

    g_autoptr(GDataInputStream) data_stream = g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    // Read each line of the os-release file
    g_autofree gchar *line = NULL;
    g_autofree gchar *val = NULL;
    while ((line = g_data_input_stream_read_line(data_stream, NULL, NULL, err)) != NULL) {
        if (strcmp(line, "") == 0) {
            g_free(line);
            continue;
        }

        // Make sure the line contains an equals character
        if (!strchr(line, '=')) {
            g_free(line);
            continue;
        }

        // Get the value from the line if the key matches
        val = disk_manager_match_os_release_line(line, find_key);
        if (val) {
            break;
        }

        g_free(line);
    }

    return g_strdup(val);
}

gchar *disk_manager_match_os_release_line(const gchar *line, const gchar *find_key) {
    g_return_val_if_fail(line != NULL, NULL);
    g_return_val_if_fail(find_key != NULL, NULL);

    // Split the line into parts
    g_autofree GStrv parts = g_strsplit(line, "=", 2);
    g_autofree gchar *key = parts[0];
    g_autofree gchar *val = parts[1];
    gsize len = strlen(val);

    if (len == 0) {
        return NULL;
    }

    // Extract the value from inside quotation marks if
    // they are present.
    if (val[0] == '"') {
        memmove(val, val + 1, len);
        len = strlen(val);
    }
    if (val[len - 1] == '"') {
        g_autofree gchar *tmp = g_strdup(val);
        g_free(val);
        val = g_strdup_printf("%.*s", (gint) len - 1, tmp);
    }

    // Convert the keys to lowercase and return the 
    // value if the keys match
    g_autofree gchar *key_lower = g_utf8_casefold(key, -1);
    g_autofree gchar *find_lower = g_utf8_casefold(find_key, -1);
    if (strcmp(key_lower, find_lower) == 0) {
        return g_strdup(val);
    }

    // This line doesn't match the given key
    return NULL;
}

gchar *disk_manager_get_linux_version(const gchar *path) {
    g_return_val_if_fail(path != NULL, NULL);

    // Iterate os-release files and then fallback to lsb-release files,
    // respecting stateless heirarchy
    gchar *name = disk_manager_search_for_key(path, os_release_paths, OS_RELEASE_PATHS_LENGTH, "PRETTY_NAME", "NAME");

    // Check that we have a name. If we don't, start looking at the
    // lsb_release files.
    if (!installer_is_string_valid(name)) {
        name = disk_manager_search_for_key(path, lsb_release_paths, LSB_RELEASE_PATHS_LENGTH, "DISTRIB_DESCRIPTION", "DISTRIB_ID");
    }

    return name;
}

gchar *disk_manager_search_for_key(
    const gchar *root,
    const gchar **paths,
    gint paths_len,
    const gchar *key,
    const gchar *fallback_key
) {
    // Sanity checks
    g_return_val_if_fail(root != NULL, NULL);
    g_return_val_if_fail(paths != NULL, NULL);
    g_return_val_if_fail(paths_len > 0, NULL);
    g_return_val_if_fail(key != NULL, NULL);

    gchar *name = NULL;
    gint i;

    // Iterate over our paths
    for (i = 0; i < paths_len; i++) {
        g_autofree gchar *fpath = g_build_path(G_DIR_SEPARATOR_S, root, paths[i], NULL);
        g_autoptr(GFile) file = g_file_new_for_path(fpath);
        if (!g_file_query_exists(file, NULL)) {
            continue;
        }

        // First, try to get the value of the first key we were given
        g_autoptr(GError) err = NULL;
        name = disk_manager_get_os_release_val(fpath, key, &err);
        if (err) {
            g_warning("Error reading release file at path '%s': %s", fpath, err->message);
            continue;
        }

        // If the first key was not found or set, try using the fallback key if
        // we were given one.
        if (!installer_is_string_valid(name) && installer_is_string_valid(fallback_key)) {
            name = disk_manager_get_os_release_val(fpath, fallback_key, &err);
            if (err) {
                g_warning("Error reading release file at path '%s': %s", fpath, err->message);
                continue;
            }
        }

        // Still no name, try the next iteration
        if (!installer_is_string_valid(name)) {
            continue;
        }
    }

    return name;
}
