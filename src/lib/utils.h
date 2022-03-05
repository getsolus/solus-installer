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

#include <gio/gio.h>
#include <glib/gerror.h>
#include <glib/gtypes.h>
#include <blockdev/blockdev.h>

/**
 * Check if `s1` starts with `s2`.
 * 
 * This expands to a call to `strncmp` to compare the strings.
 */
#define installer_string_starts_with(s1, s2) (strncmp(s1, s2, strlen(s2)) == 0)

/**
 * Creates a new temporary directory.
 * 
 * This directory may be created with an optional suffix. The first
 * portion of the directory name will be automatically generated to
 * ensure uniqueness.
 * 
 * Returns a string containing the name of the directory, or `NULL`.
 * If there was an error, `NULL` will be returned and `err` will be
 * set.
 */
gchar *installer_create_temp_dir(gchar *suffix, GError **err);

/**
 * Attempt to initialize the blockdev library with our required plugins.
 * 
 * Returns `TRUE` if initialization was successful. If there was an error,
 * `FALSE` is returned and `err` is set.
 */
gboolean installer_init_blockdev(GError **err);

/**
 * Simple helper function to check if a string is valid and not empty.
 */
gboolean installer_is_string_valid(const gchar *str);

/**
 * Opens the given file and returns the file contents with newlines stripped.
 * 
 * The returned value is owned by the caller.
 */
gchar *installer_read_line_full(GFile *file, GError **err);

/**
 * Check if a string item contains the key.
 * 
 * For use with `g_hash_table_find`, and can be safely cast to a `GHRFunc`.
 */
gboolean installer_str_contains(gchar *key, __attribute((unused)) gchar *value, gchar *item);

/**
 * Check if a string item starts with the key.
 * 
 * For use with `g_hash_table_find`, and can be safely cast to a `GHRFunc`.
 */
gboolean installer_str_starts_with(gchar *key, __attribute((unused)) gchar *value, gchar *item);
