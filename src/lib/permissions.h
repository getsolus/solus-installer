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

#ifndef INSTALLER_PERMISSIONS_H
#define INSTALLER_PERMISSIONS_H

#include <gio/gio.h>
#include <glib.h>
#include <pwd.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_PERMISSIONS (installer_permissions_get_type())

G_DECLARE_FINAL_TYPE(InstallerPermissions, installer_permissions, INSTALLER,
                     PERMISSIONS, GObject)

/**
 * Create a new PermissionsManager object.
 */
InstallerPermissions *installer_permissions_new();

/**
 * Drop our current permissions.
 */
gboolean installer_permissions_drop(InstallerPermissions *self);

/**
 * Elevate our current permissions.
 */
gboolean installer_permissions_elevate(InstallerPermissions *self);

G_END_DECLS

#endif
