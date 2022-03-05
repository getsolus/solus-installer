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

#include "strings.h"
#include <glib.h>
#include <blockdev/blockdev.h>

/**
 * Attempt to initialize the blockdev library with our required plugins.
 * 
 * Returns `TRUE` if initialization was successful. If there was an error,
 * `FALSE` is returned and `err` is set.
 */
gboolean installer_init_blockdev(GError **err);

/**
 * Generates a message from an I/O error.
 */
gchar *installer_errno_to_message(GIOErrorEnum errnum);

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
