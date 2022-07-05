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

#ifndef INSTALLER_INFO_H
#define INSTALLER_INFO_H

#include <gio/gio.h>
#include <glib.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_INFO (installer_info_get_type())

G_DECLARE_FINAL_TYPE(InstallerInfo, installer_info, INSTALLER, INFO, GObject)

/**
 * Create a new installer info object to hold information about
 * the installation between pages.
 */
InstallerInfo *installer_info_new(GObject *owner);

/* Property Getters */

gboolean installer_info_get_enable_geoip(InstallerInfo *self);

/**
 * Get the hostname for the installation.
 *
 * The caller is expected to free the return value of this function.
 */
gchar *installer_info_get_hostname(InstallerInfo *self);

gboolean installer_info_get_windows_present(InstallerInfo *self);
gboolean installer_info_get_install_bootloader(InstallerInfo *self);
gboolean installer_info_get_invalidated(InstallerInfo *self);

/* Property Setters */

void installer_info_set_enable_geoip(InstallerInfo *self, gboolean value);
void installer_info_set_hostname(InstallerInfo *self, const gchar *value);
void installer_info_set_windows_present(InstallerInfo *self, gboolean value);
void installer_info_set_install_bootloader(InstallerInfo *self, gboolean value);
void installer_info_set_invalidated(InstallerInfo *self, gboolean value);

G_END_DECLS

#endif
