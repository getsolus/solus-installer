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

#include "lib/disk_manager.h"
#include "lib/install_info.h"
#include "lib/permissions.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define INSTALLER_TYPE_WINDOW (installer_window_get_type())

G_DECLARE_FINAL_TYPE(InstallerWindow, installer_window, INSTALLER, WINDOW, GtkApplicationWindow)

/**
 * Set up our application styling.
 */
void installer_window_setup_style(InstallerWindow *self);

/**
 * Set the name of the icon name and label text to use for the current Solus edition.
 * 
 * This works by checking for each desktop binary, and exiting when one is found.
 */
void installer_window_set_vanity(InstallerWindow *self);

gboolean installer_window_start_threads(InstallerWindow *self);

void installer_window_perform_inits(InstallerWindow *self);

/**
 * Set the sensitivity of the next and previous navigation buttons.
 */
void installer_window_buttons_update_sensitivity(InstallerWindow *self);

/**
 * Add a new page to our window stack.
 */
void installer_window_page_add(InstallerWindow *self, GtkWidget *page);

gpointer installer_window_page_next_callback(InstallerWindow *self);

/**
 * Move to the next page in the stack.
 */
void installer_window_page_next(InstallerWindow *self);

gpointer installer_window_page_prev_callback(InstallerWindow *self);

/**
 * Move to the previous page in the stack.
 */
void installer_window_page_prev(InstallerWindow *self);

/**
 * Skip the next page in the stack.
 */
void installer_window_page_skip(InstallerWindow *self);

void installer_window_page_update_current(InstallerWindow *self);

/**
 * Set if the next button can be clicked.
 */
void installer_window_set_can_next(InstallerWindow *self, gboolean can_next);

/**
 * Set if the previous button can be clicked.
 */
void installer_window_set_can_prev(InstallerWindow *self, gboolean can_prev);

/**
 * Set if the installer is on the final step.
 */
void installer_window_set_is_final(InstallerWindow *self, gboolean is_final);

/**
 * Set if the application can be exited.
 */
void installer_window_set_can_quit(InstallerWindow *self, gboolean can_quit);

G_END_DECLS
