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

#include <gtk/gtk.h>

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
