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

#include "utils.h"

/**
 * Generates a message from an I/O error.
 */
static gchar *installer_errno_to_message(GIOErrorEnum errnum);

static gchar *installer_errno_to_message(GIOErrorEnum errnum) {
    gchar *message = NULL;

    switch (errnum)
    {
    case G_IO_ERROR_EXISTS:
        message = "File already exists";
        break;

    case G_IO_ERROR_FILENAME_TOO_LONG:
        message = "Filename is too many characters";
        break;

    case G_IO_ERROR_INVALID_FILENAME:
        message = "Filename is invalid or contains invalid characters";
        break;

    case G_IO_ERROR_NO_SPACE:
        message = "No space left on drive";
        break;

    case G_IO_ERROR_INVALID_ARGUMENT:
        message = "Invalid argument";
        break;

    case G_IO_ERROR_PERMISSION_DENIED:
        message = "Permission denied";
        break;

    case G_IO_ERROR_TIMED_OUT:
        message = "Operation timed out";
        break;

    case G_IO_ERROR_WOULD_BLOCK:
        message = "Operation would block";
        break;

    case G_IO_ERROR_TOO_MANY_OPEN_FILES:
        message = "The current process has too many files open and can't open any more";
        break;

    case G_IO_ERROR_FAILED:
    default:
        message = "Unknown I/O error";
        break;
    }

    return message;
}

gchar *installer_create_temp_dir(gchar *suffix, GError **err) {
    gchar *template = NULL;
    if (suffix) {
        template = g_strdup_printf("XXXXXX-%s", suffix);
    } else {
        template = g_strdup("XXXXXX-installer");
    }

    gchar *ret = g_mkdtemp(template);

    if (!installer_is_string_valid(ret)) {
        GIOErrorEnum errnum = g_io_error_from_errno(errno);
        gchar *message = installer_errno_to_message(errnum);
        g_set_error_literal(err, G_FILE_ERROR, errnum, message);
        return NULL;
    }

    return ret;
}

gboolean installer_init_blockdev(GError **err) {
    BDPluginSpec fs_plugin = {BD_PLUGIN_FS, NULL};
    BDPluginSpec part_plugin = {BD_PLUGIN_PART, NULL};
    BDPluginSpec *plugins[] = {&fs_plugin, &part_plugin, NULL};

    gboolean success = bd_ensure_init(plugins, NULL, err);

    return success;
}

gboolean installer_is_string_valid(const gchar *str) {
    return (str != NULL) && (g_strcmp0("", str) != 0);
}

gchar *installer_read_line_full(GFile *file, GError **err) {
    if (!G_IS_FILE(file)) {
        return NULL;
    }

    gboolean res;
    gsize bytes_read;

    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, err);
    if (!G_IS_FILE_INPUT_STREAM(input_stream)) {
        return NULL;
    }

    g_autoptr(GFileInfo) file_info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, 0, NULL, err);
    if (!G_IS_FILE_INFO(file_info)) {
        return NULL;
    }

    goffset file_size = g_file_info_get_size(file_info);
    g_autofree gchar *buffer = g_malloc(file_size);

    res = g_input_stream_read_all(G_INPUT_STREAM(input_stream), buffer, file_size, &bytes_read, NULL, err);

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

gboolean installer_str_contains(gchar *key, __attribute((unused)) gchar *value, gchar *item) {
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(item != NULL, FALSE);

    if (g_strstr_len(item, -1, key)) {
        return TRUE;
    }

    return FALSE;
}

gboolean installer_str_starts_with(gchar *key, __attribute((unused)) gchar *value, gchar *item) {
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(item != NULL, FALSE);

    if (installer_string_starts_with(item, key)) {
        return TRUE;
    }

    return FALSE;
}
