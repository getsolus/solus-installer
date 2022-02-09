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
