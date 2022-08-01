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

#ifndef INSTALLER_USER_H
#define INSTALLER_USER_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * INSTALLER_USER_ERROR:
 *
 * Error domain for user error validation. Errors in this domain
 * will be from the #InstallerUserError enumeration. See #GError
 * for information on error domains.
 */
#define INSTALLER_USER_ERROR (installer_user_error_quark())

/**
 * InstallerUserError:
 * @INSTALLER_USER_ERROR_INVALID_USERNAME: Username is %NULL or empty
 * @INSTALLER_USER_ERROR_EMPTY_PASSWORD: Password is %NULL or empty
 *
 * Error codes returned by validating users.
 */
typedef enum {
    INSTALLER_USER_ERROR_INVALID_USERNAME,
    INSTALLER_USER_ERROR_EMPTY_PASSWORD,
    INSTALLER_USER_ERROR_EMPTY_CONFIRM_PASSWORD,
    INSTALLER_USER_ERROR_MISMATCHED_PASSWORDS
} InstallerUserError;

G_END_DECLS

/**
 * InstallerUser:
 * @user_name: This user's login name
 * @full_name: This user's full name
 * @password: This user's login password
 * @passwords_match: Whether or not the password setting fields match
 *
 * This struct represents a user to add to the system during
 * installation.
 */
typedef struct _InstallerUser {
    gchar *user_name;
    gchar *full_name;
    gchar *password;
    gchar *confirm_password;
    gboolean passwords_match;
} InstallerUser;

InstallerUser *installer_user_new(void);

void installer_user_free(InstallerUser *self);

void installer_user_set_user_name(InstallerUser *self, const gchar *user_name);

void installer_user_set_full_name(InstallerUser *self, const gchar *full_name);

void installer_user_set_password(InstallerUser *self, const gchar *password);

void installer_user_set_confirm_password(InstallerUser *self, const gchar *password);

gboolean installer_user_validate(InstallerUser *self, GError **err);

#endif
