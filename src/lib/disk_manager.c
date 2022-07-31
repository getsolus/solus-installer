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
#include "drive.h"
#include "os.h"

const gchar *os_release_paths[OS_RELEASE_PATHS_LENGTH] = {"etc/os-release",
                                                          "usr/lib/os-release"};

const gchar *lsb_release_paths[LSB_RELEASE_PATHS_LENGTH] = {
    "etc/lsb-release", "usr/lib/lsb-release",
    "usr/share/defaults/etc/lsb-release"};

const gchar *os_icons[OS_ICONS_LENGTH] = {
    "antergos", "archlinux", "crunchbang", "debian", "deepin",
    "edubuntu", "elementary", "fedora", "frugalware", "gentoo",
    "kubuntu", "linux-mint", "mageia", "mandriva", "manjaro",
    "solus", "opensuse", "slackware", "steamos", "ubuntu-gnome",
    "ubuntu-mate", "ubuntu"};

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
};

G_DEFINE_TYPE(DiskManager, disk_manager, G_TYPE_OBJECT);

static void disk_manager_finalize(GObject *obj);

static void disk_manager_class_init(DiskManagerClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = disk_manager_finalize;
}

/**
 * read_line_full:
 * @file: The #GFile to read from
 * @err: (out): Place to store an error (if any)
 *
 * Reads the contents of a file, returning the full contents as
 * a single string.
 *
 * Returns: (transfer full): The file's contents as a single string
 *     without newlines.
 */
static gchar *read_line_full(GFile *file, GError **err) {
    if (!G_IS_FILE(file)) {
        return NULL;
    }

    gboolean res;
    gsize bytes_read;

    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        return NULL;
    }

    g_autoptr(GFileInfo) file_info =
        g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, 0, NULL, err);
    if (!G_IS_FILE_INFO(file_info)) {
        return NULL;
    }

    goffset file_size = g_file_info_get_size(file_info);
    g_autofree gchar *buffer = g_malloc(file_size);

    res = g_input_stream_read_all(G_INPUT_STREAM(input_stream), buffer, file_size,
                                  &bytes_read, NULL, err);

    if (!res) {
        return NULL;
    }

    if ((goffset) bytes_read < file_size) {
        // The read was successful, but the read count is less than the
        // file size. We know the end was reached (read returned success),
        // so re-create the buffer with only the read characters.
        const g_autofree gchar *tmp = buffer;
        buffer = g_strndup(tmp, bytes_read);
    }

    buffer = g_strchomp(buffer);
    g_autoptr(GString) str = g_string_new(buffer);
    g_string_replace(str, "\r", "", -1);
    g_string_replace(str, "\n", "", -1);
    return g_strdup(str->str);
}

static void disk_manager_init(DiskManager *self) {
    g_return_if_fail(DISK_IS_MANAGER(self));

    /* Regexes. Gratefully borrowed from gparted, Proc_Partitions_Info.cc */

    self->re_whole_disk = g_regex_new(
        "^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+([^0-9]+)$", 0, 0, NULL);

    self->re_mmcblk = g_regex_new(
        "^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+(mmcblk[0-9]+)$", 0, 0, NULL);

    self->re_nvme = g_regex_new(
        "^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+(nvme[0-9]+n[0-9]+)$", 0, 0,
        NULL);

    self->re_raid = g_regex_new(
        "^[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+[0-9]+[\t ]+(md[0-9]+)$", 0, 0, NULL);

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
    g_hash_table_insert(self->win_bootloaders, "V.i.s.t.a",
                        "Windows Vista bootloader");
    g_hash_table_insert(self->win_bootloaders, "W.i.n.d.o.w.s. .7",
                        "Windows 7 bootloader");
    g_hash_table_insert(self->win_bootloaders,
                        "W.i.n.d.o.w.s. .R.e.c.o.v.e.r.y. .E.n.v.i.r.o.n.m.e.n.t",
                        "Windows recovery");
    g_hash_table_insert(self->win_bootloaders,
                        "W.i.n.d.o.w.s. .S.e.r.v.e.r. .2.0.0.8",
                        "Windows Server 2008 bootloader");

    /* Set up UEFI knowledge */

    g_autoptr(GFile) efi_file = g_file_new_for_path("/sys/firmware/efi");
    if (g_file_query_exists(efi_file, NULL)) {
        self->is_uefi = TRUE;
        g_autoptr(GFile) platform_file =
            g_file_new_for_path("/sys/firmware/efi/fw_platform_size");
        if (g_file_query_exists(platform_file, NULL)) {
            g_autoptr(GError) err = NULL;
            g_autofree gchar *size = read_line_full(platform_file, &err);
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
    g_autoptr(GFileInputStream) input_stream =
        g_file_read(partition_file, NULL, &err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        g_warning("Error reading partition file: %s", err->message);
        return;
    }

    g_autoptr(GDataInputStream) data_stream =
        g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    // Read each line of the partitions file
    g_autofree gchar *line = NULL;
    while ((line = g_data_input_stream_read_line(data_stream, NULL, NULL,
                                                 &err)) != NULL) {
        if (!line) {
            g_warning("Error reading partition line: %s", err->message);
            break;
        }

        GRegex *groups[4] = {self->re_whole_disk, self->re_mmcblk, self->re_nvme,
                             self->re_raid};

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

    g_autofree gchar *path =
        g_build_path(G_DIR_SEPARATOR_S, "/dev/", device, NULL);

    g_autoptr(GFile) file = g_file_new_for_path(path);
    if (!g_file_query_exists(file, NULL)) {
        g_warning("Trying to add non-existant device: %s", path);
        return;
    }

    if (g_slist_find(self->devices, device) == NULL) {
        self->devices =
            g_slist_append(self->devices, g_canonicalize_filename(path, "/"));
    }
}

GSList *disk_manager_get_devices(DiskManager *self) {
    return self->devices;
}

gboolean disk_manager_is_device_ssd(const gchar *path) {
    g_return_val_if_fail(path != NULL, FALSE);

    g_autofree gchar *nodename = g_path_get_basename(path);
    g_autofree gchar *fpath =
        g_strdup_printf("/sys/block/%s/queue/rotational", nodename);

    g_autoptr(GFile) file = g_file_new_for_path(fpath);
    if (!g_file_query_exists(file, NULL)) {
        return FALSE;
    }

    // Don't try using a SSD with eMMC
    if (g_str_has_prefix(nodename, "mmcblk")) {
        return FALSE;
    }

    // Open and read the device type file
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, &err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        g_warning("Error reading device type file: %s", err->message);
        return FALSE;
    }

    g_autoptr(GDataInputStream) data_stream =
        g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    g_autofree gchar *line =
        g_data_input_stream_read_line(data_stream, NULL, NULL, &err);
    if (g_strcmp0(line, "0") == 0) {
        return TRUE;
    }

    return FALSE;
}

gboolean disk_manager_is_install_supported(const gchar *path) {
    g_return_val_if_fail(path != NULL, FALSE);

    g_autofree gchar *nodename = g_path_get_basename(path);
    if (g_str_has_prefix(nodename, "md")) {
        return FALSE;
    }
    return TRUE;
}

GHashTable *disk_manager_get_mount_points() {
    GHashTable *ret = g_hash_table_new(g_str_hash, g_str_equal);

    // Open and read the mounts file
    g_autoptr(GFile) mounts_file = g_file_new_for_path("/proc/self/mounts");
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream =
        g_file_read(mounts_file, NULL, &err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        g_warning("Error reading mounts file: %s", err->message);
        return ret;
    }

    g_autoptr(GDataInputStream) data_stream =
        g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    // Read each line of the mounts file
    g_autofree gchar *line = NULL;
    while ((line = g_data_input_stream_read_line(data_stream, NULL, NULL,
                                                 &err)) != NULL) {
        if (strcmp(line, "") == 0) {
            g_free(line);
            continue;
        }

        // Split the line into parts
        g_autofree gchar **parts = g_strsplit(line, " ", -1);

        // Calculate the length of the parts vector
        gsize len = (sizeof parts / sizeof(gchar) - 1);
        if (len < 4) {
            g_free(line);
            continue;
        }

        gchar *dev = parts[0];
        gchar *mount_point = parts[1];

        // We only want block devices
        if (g_str_has_prefix(dev, "/")) {
            g_hash_table_insert(ret, dev, mount_point);
        }

        g_free(line);
    }

    return ret;
}

static gboolean has_prefix(gchar *key, __attribute((unused)) gchar *value,
                           gchar *item) {
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(item != NULL, FALSE);

    return g_str_has_prefix(key, item);
}

/**
 * get_windows_version:
 * @path: The path to possible Windows partition
 * @self: The current #DiskManager
 *
 * Attempts to get the version of Windows installed on a partition.
 *
 * Returns: (transfer full): A string containing the Windows version,
 *          or %NULL
 */
static gchar *get_windows_version(const gchar *path, DiskManager *self) {
    g_return_val_if_fail(DISK_IS_MANAGER(self), NULL);
    g_return_val_if_fail(path != NULL, NULL);

    g_autofree gchar *fpath = g_build_path(G_DIR_SEPARATOR_S, path, "Windows",
                                           "servicing", "Version", NULL);
    g_autoptr(GFile) version_file = g_file_new_for_path(fpath);

    // Check if the version file exists. If it doesn't, look for System32
    // to make sure the path really is a Windows path.
    if (!g_file_query_exists(version_file, NULL)) {
        g_free(fpath);
        fpath = g_build_path(path, "Windows", "System32", NULL);
        g_autoptr(GFile) system32 = g_file_new_for_path(fpath);

        // Windows is installed, but we can't find out what version it is
        if (g_file_query_exists(system32, NULL)) {
            return "Windows (Unknown)";
        }

        return NULL;
    }

    // Open the Windows version directory
    // TODO: Error handling
    g_autoptr(GDir) version_dir = g_dir_open(fpath, 0, NULL);
    if (!version_dir) {
        return NULL;
    }

    const gchar *child = NULL;

    // Iterate over the items in the directory to try to find a match
    // in our Windows prefixes HashTable. If one is found, return the
    // value in the table.
    while ((child = g_dir_read_name(version_dir)) != NULL) {
        gchar *item = g_hash_table_find(self->win_prefixes, (GHRFunc) has_prefix,
                                        (gchar *) child);
        if (item) {
            return g_strdup(item);
        }
    }

    g_dir_close(version_dir);
    return NULL;
}

static gboolean contains_path(gchar *key, __attribute((unused)) gchar *value,
                              gchar *item) {
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(item != NULL, FALSE);

    if (g_strstr_len(item, -1, key)) {
        return TRUE;
    }

    return FALSE;
}

/**
 * get_windows_bootloader:
 * @path: The path to a partition
 * @self: The current #DiskManager
 *
 * Attempts to get the Windows bootloader version if one is
 * installed on the given partition.
 *
 * Returns: (transfer full): A string containing the Windows
 *          bootloader version, or %NULL
 */
static gchar *get_windows_bootloader(const gchar *path, DiskManager *self) {
    if (!DISK_IS_MANAGER(self)) {
        return NULL;
    }

    g_return_val_if_fail(path != NULL, NULL);

    g_autofree gchar *fpath =
        g_build_path(G_DIR_SEPARATOR_S, path, "Boot", "BCD", NULL);
    g_autoptr(GFile) file = g_file_new_for_path(fpath);

    if (!g_file_query_exists(file, NULL)) {
        return NULL;
    }

    gchar *item =
        g_hash_table_find(self->win_bootloaders, (GHRFunc) contains_path, fpath);
    if (item) {
        return g_strdup(item);
    }

    return "Windows bootloader";
}

/**
 * match_os_release_line:
 * @line: The line to try to match
 * @find_key: The key to look for
 *
 * Checks if the %line starts with the %find_key, returning
 * the value of the line if it does. If the line doesn't match,
 * %NULL is returned.
 *
 * Returns: (transfer full): The value of the line as a string, or
 *          %NULL
 */
static gchar *match_os_release_line(const gchar *line, const gchar *find_key) {
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

/**
 * get_os_release_val:
 * @path: The path to the release file, e.g. /etc/os-release or
 *        /usr/lib/os-release
 * @find_key: The key in the release file to get the value of
 * @err: (out): Place to store an error, if present
 *
 * Opens a Linux os-release file and searches for the line in
 * the file that has the given %find_key.
 *
 * If found, the value of the key will be returned. If there was
 * an error, %NULL will be returned and %err will be set.
 *
 * Returns: (transfer full): The value in the file corresponding
 *           to %find_key, or %NULL
 */
static gchar *get_os_release_val(const gchar *path, const gchar *find_key,
                                 GError **err) {
    g_return_val_if_fail(path != NULL, NULL);
    g_return_val_if_fail(find_key != NULL, NULL);

    g_autoptr(GFile) file = g_file_new_for_path(path);
    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        return NULL;
    }

    g_autoptr(GDataInputStream) data_stream =
        g_data_input_stream_new(G_INPUT_STREAM(input_stream));

    // Read each line of the os-release file
    g_autofree gchar *line = NULL;
    g_autofree gchar *val = NULL;
    while ((line = g_data_input_stream_read_line(data_stream, NULL, NULL, err)) !=
           NULL) {
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
        val = match_os_release_line(line, find_key);
        if (val) {
            break;
        }

        g_free(line);
    }

    return g_strdup(val);
}

/**
 * search_for_key:
 * @root: The root of the path to the file to search
 * @paths: An array of paths to use
 * @paths_len: The length of the %paths array
 * @key: The key to search for
 * @fallback_key: A secondary %key to use if the first one isn't found
 *
 * Searches files at the given paths for the given keys, returning
 * the value if one is found. Once a key is found, searching stops.
 *
 * Returns: (transfer full): The value of the %key, or %NULL
 */
static gchar *search_for_key(const gchar *root, const gchar **paths,
                             gint paths_len, const gchar *key,
                             const gchar *fallback_key) {
    // Sanity checks
    g_return_val_if_fail(root != NULL, NULL);
    g_return_val_if_fail(paths != NULL, NULL);
    g_return_val_if_fail(paths_len > 0, NULL);
    g_return_val_if_fail(key != NULL, NULL);

    gchar *name = NULL;
    gint i;

    // Iterate over our paths
    for (i = 0; i < paths_len; i++) {
        g_autofree gchar *fpath =
            g_build_path(G_DIR_SEPARATOR_S, root, paths[i], NULL);
        g_autoptr(GFile) file = g_file_new_for_path(fpath);
        if (!g_file_query_exists(file, NULL)) {
            continue;
        }

        // First, try to get the value of the first key we were given
        g_autoptr(GError) err = NULL;
        name = get_os_release_val(fpath, key, &err);
        if (err) {
            g_warning("Error reading release file at path '%s': %s", fpath,
                      err->message);
            continue;
        }

        // If the first key was not found or set, try using the fallback key if
        // we were given one.
        if (!name && fallback_key) {
            name = get_os_release_val(fpath, fallback_key, &err);
            if (err) {
                g_warning("Error reading release file at path '%s': %s", fpath,
                          err->message);
                continue;
            }
        }

        // Still no name, try the next iteration
        if (!name) {
            continue;
        }
    }

    return name;
}

/**
 * get_linux_version:
 * @path: The path to a partition to check
 * @self: The current #DiskManager
 *
 * Looks for a Linux installation on the given partition by searching
 * the os-release and lsb-release paths for Linux distro identifiers.
 *
 * Returns: (transfer full): The name or identifier of the installed
 *          Linux distrobution if one is installed, or %NULL
 */
static gchar *get_linux_version(const gchar *path,
                                __attribute((unused)) DiskManager *self) {
    g_return_val_if_fail(path != NULL, NULL);

    // Iterate os-release files and then fallback to lsb-release files,
    // respecting stateless heirarchy
    gchar *name = search_for_key(path, os_release_paths, OS_RELEASE_PATHS_LENGTH,
                                 "PRETTY_NAME", "NAME");

    // Check that we have a name. If we don't, start looking at the
    // lsb_release files.
    if (!name) {
        name = search_for_key(path, lsb_release_paths, LSB_RELEASE_PATHS_LENGTH,
                              "DISTRIB_DESCRIPTION", "DISTRIB_ID");
    }

    return name;
}

/**
 * get_os_icon:
 * @os: The #InstallerOS to get an icon for
 *
 * Attempts to get an icon to use for an Operating System.
 *
 * Returns: (transfer full): The icon name to use for this %os,
 *          or %NULL
 */
static gchar *get_os_icon(InstallerOS *os) {
    g_return_val_if_fail(INSTALLER_IS_OS(os), "system-software-install");

    g_autofree gchar *otype = installer_os_get_otype(os);

    // Check the OS type to see if it's Windows or Linux
    if (strcmp(otype, "windows") == 0 || strcmp(otype, "windows-boot") == 0) {
        return "distributor-logo-windows";
    } else if (strcmp(otype, "linux") != 0) {
        return "system-software-install";
    }

    // Convert the OS name to lowercase and remove leading/trailing spaces
    g_autofree gchar *raw = installer_os_get_name(os);
    raw = g_strstrip(raw);
    g_autofree gchar *raw_lower = g_utf8_casefold(raw, -1);

    // Turn spaces into hyphens
    g_autoptr(GString) mangled = g_string_new(raw_lower);
    g_string_replace(mangled, " ", "-", -1);

    // Look for a matching icon
    gint i;
    for (i = 0; i < OS_ICONS_LENGTH; i++) {
        if (g_str_has_prefix(mangled->str, os_icons[i])) {
            return g_strdup_printf("distributor-logo-%s", os_icons[i]);
        }
    }

    return "system-software-install";
}

gchar *disk_manager_get_disk_model(gchar *device, GError **err) {
    g_return_val_if_fail(device != NULL, NULL);

    g_autofree gchar *node = NULL;
    g_autofree gchar *fpath = NULL;
    g_autoptr(GFile) model_file = NULL;
    gchar *model = NULL;

    node = g_path_get_basename(device);
    fpath = g_strdup_printf("/sys/block/%s/device/model", node);
    model_file = g_file_new_for_path(fpath);
    model = read_line_full(model_file, err);

    return model;
}

gchar *disk_manager_get_disk_vendor(gchar *device, GError **err) {
    g_return_val_if_fail(device != NULL, NULL);

    g_autofree gchar *node = NULL;
    g_autofree gchar *fpath = NULL;
    g_autoptr(GFile) vendor_file = NULL;
    gchar *vendor = NULL;

    node = g_path_get_basename(device);
    fpath = g_strdup_printf("/sys/block/%s/device/vendor", node);
    vendor_file = g_file_new_for_path(fpath);
    vendor = read_line_full(vendor_file, err);

    return vendor;
}

typedef struct _BlacklistData {
    GSList *blacklist;

    gchar *current_path;
    gboolean should_skip;
} BlacklistData;

static void check_blacklist(gchar *path, BlacklistData *data) {
    if (strcmp(path, data->current_path) == 0) {
        data->should_skip = TRUE;
    }
}

static void maybe_blacklist(GMount *mount, BlacklistData *blacklist_data) {
    g_autoptr(GFile) mount_root = g_mount_get_root(mount);
    g_autofree gchar *mount_point = g_file_get_path(mount_root);

    if (strcmp(mount_point, "/") == 0 ||
        g_str_has_prefix(mount_point, "/run/initramfs")) {
        blacklist_data->blacklist =
            g_slist_append(blacklist_data->blacklist, mount);
    }
}

typedef gpointer (*OSVersionFunc)(gpointer path, gpointer user_data);

InstallerOS *disk_manager_detect_os(DiskManager *self, BDPartSpec *device,
                                    GError **err) {
    g_debug("attempting to detect OS on '%s'", device->path);

    gboolean mounted = FALSE;
    g_autofree gchar *mount_point = NULL;
    g_autoptr(GFile) mount_dir = NULL;
    g_autoptr(GHashTable) possibles = NULL;
    GHashTableIter iter;
    g_autofree gchar *os_name = NULL;
    g_autofree gchar *os_icon_name = NULL;
    InstallerOS *ret = NULL;

    // Ignore swap and Microsoft reserved partitions
    if (device->flags & (BD_PART_FLAG_SWAP | BD_PART_FLAG_MSFT_RESERVED)) {
        g_debug("detected swap or Microsoft reserved; skipping device");
        return NULL;
    }

    // Get or create a mount point for this OS
    // TODO: Error handling
    mount_point = bd_fs_get_mountpoint(device->path, NULL);
    if (!mount_point) {
        g_debug("attempting to create a temp dir for mounting");
        mount_point = g_dir_make_tmp("us.getsol.Installer-XXXXXX", err);
        if (!mount_point) {
            return NULL;
        }

        g_debug("attempting to mount device at %s", mount_point);
        if (!bd_fs_mount(device->path, mount_point, "auto", "ro", NULL, err)) {
            mount_dir = g_file_new_for_path(mount_point);
            g_file_delete(mount_dir, NULL, err);
            return NULL;
        }

        mounted = TRUE;
    }

    // TODO: There has got to be a nicer way to do this...
    possibles = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(possibles, "windows", get_windows_version);
    g_hash_table_insert(possibles, "windows-boot", get_windows_bootloader);
    g_hash_table_insert(possibles, "linux", get_linux_version);

    g_hash_table_iter_init(&iter, possibles);
    gchar *key = NULL;
    OSVersionFunc *func = NULL;

    // Iterate over our possible OS types
    while (g_hash_table_iter_next(&iter, (gpointer *) &key, (gpointer *) &func)) {
        g_debug("looking for %s", key);

        // Try to get the OS version for this type
        os_name = ((OSVersionFunc) func)(device->path, self);
        if (!os_name) {
            // None found, continue to the next possibility
            continue;
        }

        // Create our OS info struct to return
        ret = installer_os_new(key, os_name, device->path);
        os_icon_name = get_os_icon(ret);
        installer_os_set_icon_name(ret, os_icon_name);
        break;
    }

    // Make sure we're not mounted
    if (mounted) {
        g_debug("unmounting device '%s' at '%s'", device->path, mount_point);
        if (bd_fs_unmount(mount_point, TRUE, FALSE, NULL, err)) {
            g_debug("cleaning up mount point '%s'", mount_point);
            mount_dir = g_file_new_for_path(mount_point);
            g_file_delete(mount_dir, NULL, err);
        }
    }

    return ret;
}

static gboolean is_efi_system_partition(DiskManager *self,
                                        BDPartDiskSpec *disk_spec,
                                        BDPartSpec *part_spec, GError **err) {
    g_autofree gchar *basename = NULL;
    g_autofree gchar *fstype = NULL;
    g_autofree gchar *found_fstype = NULL;

    basename = g_path_get_basename(part_spec->path);

    if (disk_spec->table_type != BD_PART_TABLE_GPT) {
        return FALSE;
    }

    fstype = bd_fs_get_fstype(part_spec->path, err);

    if (!fstype) {
        return FALSE;
    }

    found_fstype = (gchar *) g_slist_find(self->efi_types, fstype);

    if (!found_fstype) {
        return FALSE;
    }

    return part_spec->flags & BD_PART_FLAG_BOOT;
}

InstallerDrive *disk_manager_parse_system_disk(DiskManager *self, gchar *device,
                                               gchar *disk, GList *mounts,
                                               GError **err) {
    GHashTable *operating_systems = NULL;
    GSList *esps = NULL;
    BDPartDiskSpec *disk_spec = NULL;
    BDPartSpec **partitions = NULL;
    BlacklistData blacklist_data = {
        .blacklist = NULL, .current_path = device, .should_skip = FALSE};

    g_autofree gchar *vendor = NULL;
    g_autofree gchar *model = NULL;

    InstallerDrive *ret = NULL;

    // Check if some mount points should be blacklisted
    g_list_foreach(mounts, (GFunc) maybe_blacklist, &blacklist_data);

    // Check if the current device path is blacklisted, e.g. /dev/sda
    g_slist_foreach(blacklist_data.blacklist, (GFunc) check_blacklist,
                    &blacklist_data);

    if (blacklist_data.should_skip) {
        g_debug("blacklist indicates we should skip");
        return NULL;
    }

    operating_systems = g_hash_table_new(g_str_hash, g_str_equal);

    if (disk) {
        disk_spec = bd_part_get_disk_spec(disk, err);
        if (!disk_spec) {
            return NULL;
        }

        g_debug("getting partitions on disk '%s'", disk_spec->path);

        partitions = bd_part_get_disk_parts(disk, err);
        g_return_val_if_fail(partitions != NULL, NULL);

        int i = 0;
        BDPartSpec *partition = NULL;

        g_debug("iterating over partitions on disk");
        while ((partition = partitions[i++]) != NULL) {
            InstallerOS *os = NULL;
            g_autoptr(GError) detect_err = NULL;

            blacklist_data.current_path = partition->path;
            g_slist_foreach(blacklist_data.blacklist, (GFunc) check_blacklist,
                            &blacklist_data);

            if (blacklist_data.should_skip) {
                g_debug("partition '%s' blacklisted; skipping", partition->path);
                continue;
            }

            if (partition->type & BD_PART_TYPE_FREESPACE) {
                g_debug("partition is free space; skipping");
                continue;
            }

            os = disk_manager_detect_os(self, partition, &detect_err);
            if (!os) {
                if (detect_err) {
                    g_critical("error detecting operating system for partition '%s': %s",
                               partition->path, detect_err->message);
                }
                g_debug("no operating system detected at '%s'", partition->path);
                continue;
            }

            g_hash_table_insert(operating_systems, partition->path, os);

            if (is_efi_system_partition(self, disk_spec, partition, err)) {
                g_debug("detected this is a system partition");
                esps = g_slist_append(esps, partition);
            }
        }

        g_free(disk_spec);
        g_free(partitions);
    }

    vendor = disk_manager_get_disk_vendor(device, err);
    g_return_val_if_fail(vendor != NULL, NULL);

    model = disk_manager_get_disk_model(device, err);
    g_return_val_if_fail(model != NULL, NULL);

    ret = installer_drive_new(g_strdup(device), g_strdup(disk), g_strdup(vendor),
                              g_strdup(model), operating_systems, err);

    g_return_val_if_fail(ret != NULL, NULL);

    ret->esps = esps;
    ret->partitions = partitions;

    return ret;
}
