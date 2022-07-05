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

#include "installer.h"

gboolean installer_init_blockdev(GError **err) {
  BDPluginSpec fs_plugin = {BD_PLUGIN_FS, NULL};
  BDPluginSpec part_plugin = {BD_PLUGIN_PART, NULL};
  BDPluginSpec *plugins[] = {&fs_plugin, &part_plugin, NULL};

  gboolean success = bd_ensure_init(plugins, NULL, err);

  return success;
}

gchar *installer_errno_to_message(GIOErrorEnum errnum) {
  gchar *message = NULL;

  switch (errnum) {
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
    message =
        "The current process has too many files open and can't open any more";
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

  if (!ret) {
    GIOErrorEnum errnum = g_io_error_from_errno(errno);
    gchar *message = installer_errno_to_message(errnum);
    g_set_error_literal(err, G_FILE_ERROR, errnum, message);
    return NULL;
  }

  return ret;
}
