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

#include "utils.h"
#include <pwd.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_PERMISSIONS_MANAGER (permissions_manager_get_type())

G_DECLARE_FINAL_TYPE(PermissionsManager, permissions_manager, PERMISSIONS, MANAGER, GObject)

/**
 * Create a new PermissionsManager object.
 */
PermissionsManager *permissions_manager_new();

/**
 * Set the home directory.
 */
void permissions_manager_set_details(PermissionsManager *self);

/**
 * Drop our current permissions.
 */
gboolean permissions_manager_down_permissions(PermissionsManager *self);

/**
 * Elevate our current permissions.
 */
gboolean permissions_manager_up_permissions(PermissionsManager *self);

G_END_DECLS
