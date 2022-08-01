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

#include "user.h"
#include <string.h>

// clang-format off
G_DEFINE_QUARK(installer-user-error-quark, installer_user_error)
// clang-format on

/**
 * installer_user_new:
 *
 * Creates a new #InstallerUser.
 *
 * Returns: (transfer full): The new #InstallerUser.
 *          Free with installer_user_free()
 */
InstallerUser *installer_user_new(void) {
    InstallerUser *self = NULL;

    self = (InstallerUser *) malloc(sizeof(InstallerUser));

    if (!self) {
        return NULL;
    }

    self->user_name = NULL;
    self->full_name = NULL;
    self->password = NULL;
    self->confirm_password = NULL;
    self->passwords_match = FALSE;

    return self;
}

/**
 * installer_user_free:
 * @self: (nullable): The user struct to free
 *
 * Frees resources used by a user struct.
 */
void installer_user_free(InstallerUser *self) {
    if (!self) {
        return;
    }

    g_free(self->user_name);
    g_free(self->full_name);
    g_free(self->password);
    g_free(self->confirm_password);

    g_free(self);
}

/**
 * installer_user_set_user_name:
 * @self: (not nullable): The #InstallerUser to set the username for
 * @user_name: (not nullable): The new username to set on @self
 *
 * Set a new username for this #InstallerUser.
 */
void installer_user_set_user_name(InstallerUser *self, const gchar *user_name) {
    g_return_if_fail(self != NULL);
    g_return_if_fail(user_name != NULL);

    if (self->user_name) {
        g_free((gchar *) self->user_name);
    }

    self->user_name = g_strdup(user_name);
}

/**
 * installer_user_set_full_name:
 * @self: (not nullable): The #InstallerUser to set the username for
 * @full_name: (not nullable): The new full name to set on @self
 *
 * Set a new full name for this #InstallerUser.
 */
void installer_user_set_full_name(InstallerUser *self, const gchar *full_name) {
    g_return_if_fail(self != NULL);
    g_return_if_fail(full_name != NULL);

    if (self->full_name) {
        g_free((gchar *) self->full_name);
    }

    self->full_name = g_strdup(full_name);
}

/**
 * check_passwords_match:
 * @a: (nullable): A string containing a password
 * @b: (nullable): A string containing a password
 *
 * Compares two password strings to see if they match.
 *
 * Returns: %TRUE if the passwords match, %FALSE otherwise
 */
static gboolean check_passwords_match(gchar *a, gchar *b) {
    if (g_strcmp0(a, b) == 0) {
        return TRUE;
    }

    return FALSE;
}

/**
 * installer_user_set_password:
 * @self: (not nullable): The #InstallerUser to set the username for
 * @password: (not nullable): The new password to set on @self
 *
 * Set a new password for this #InstallerUser.
 */
void installer_user_set_password(InstallerUser *self, const gchar *password) {
    g_return_if_fail(self != NULL);
    g_return_if_fail(password != NULL);

    if (self->password) {
        g_free((gchar *) self->password);
    }

    self->password = g_strdup(password);
    self->passwords_match = check_passwords_match(self->password, self->confirm_password);
}

/**
 * installer_user_set_password:
 * @self: (not nullable): The #InstallerUser to set the username for
 * @password: (not nullable): The new password to set on @self
 *
 * Set a new password for this #InstallerUser.
 */
void installer_user_set_confirm_password(InstallerUser *self, const gchar *password) {
    g_return_if_fail(self != NULL);
    g_return_if_fail(password != NULL);

    if (self->confirm_password) {
        g_free((gchar *) self->confirm_password);
    }

    self->confirm_password = g_strdup(password);
    self->passwords_match = check_passwords_match(self->password, self->confirm_password);
}

/**
 * installer_user_validate:
 * @self: The #InstallerUser to validate
 * @err: (out) (optional): Return location for validation error
 *
 * Validates a user struct. Validation includes making sure the username is
 * not empty, the passwords are not empty, and that the passwords match.
 *
 * Returns: %TRUE if valid, %FALSE otherwise, and @err is set
 */
gboolean installer_user_validate(InstallerUser *self, GError **err) {
    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

    if (self->user_name == NULL || strcmp(self->user_name, "") == 0) {
        g_set_error(err, INSTALLER_USER_ERROR, INSTALLER_USER_ERROR_INVALID_USERNAME, "Username is empty");
        return FALSE;
    }

    if (self->password == NULL || strcmp(self->password, "") == 0) {
        g_set_error(err, INSTALLER_USER_ERROR, INSTALLER_USER_ERROR_EMPTY_PASSWORD, "Password is empty");
        return FALSE;
    }

    if (self->confirm_password == NULL || strcmp(self->confirm_password, "") == 0) {
        g_set_error(err, INSTALLER_USER_ERROR, INSTALLER_USER_ERROR_EMPTY_CONFIRM_PASSWORD, "Confirmation password is empty");
        return FALSE;
    }

    if (!self->passwords_match) {
        g_set_error(err, INSTALLER_USER_ERROR, INSTALLER_USER_ERROR_MISMATCHED_PASSWORDS, "Entered passwords do not match");
        return FALSE;
    }

    return TRUE;
}
