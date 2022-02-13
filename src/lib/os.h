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

G_BEGIN_DECLS

#define INSTALLER_TYPE_OS (installer_os_get_type())

G_DECLARE_FINAL_TYPE(InstallerOS, installer_os, INSTALLER, OS, GObject)

/**
 * Create a new object for information about an operating system.
 */
InstallerOS *installer_os_new(gchar *otype, gchar *name, const gchar *device_path);

/**
 * Get the type of this OS.
 * 
 * The caller is responsible for freeing this string.
 */
gchar *installer_os_get_otype(InstallerOS *self);

/**
 * Get the name of this OS.
 * 
 * The caller is responsible for freeing this string.
 */
gchar *installer_os_get_name(InstallerOS *self);

/**
 * Get the path to the device this OS is installed on.
 * 
 * The caller is responsible for freeing this string.
 */
const gchar *installer_os_get_device_path(InstallerOS *self);

/**
 * Get the icon name for this OS.
 * 
 * The caller is responsible for freeing this string.
 */
gchar *installer_os_get_icon_name(InstallerOS *self);

/**
 * Set the type of this OS.
 */
void installer_os_set_otype(InstallerOS *self, const gchar *value);

/**
 * Set the name of this OS.
 */
void installer_os_set_name(InstallerOS *self, const gchar *value);

/**
 * Set the path to the device this OS is installed on.
 */
void installer_os_set_device_path(InstallerOS *self, const gchar *value);

/**
 * Set the icon name for this OS.
 */
void installer_os_set_icon_name(InstallerOS *self, const gchar *value);

G_END_DECLS
