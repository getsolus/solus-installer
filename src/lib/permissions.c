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

#define _GNU_SOURCE

#include "permissions.h"

struct _PermissionsManager {
    GObject parent_instance;

    guint down_uid;
    guint down_gid;
    const gchar *home_dir;
};

G_DEFINE_TYPE(PermissionsManager, permissions_manager, G_TYPE_OBJECT);

static void permissions_manager_finalize(GObject *obj);

static void permissions_manager_class_init(PermissionsManagerClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = permissions_manager_finalize;
}

static void permissions_manager_init(PermissionsManager *self) {
    const gchar *pkexec_uid = g_getenv("PKEXEC_UID");
    guint64 uid = 0;
    if (installer_is_string_valid(pkexec_uid)) {
        g_autoptr(GError) err = NULL;
        // Convert string to guint64
        g_ascii_string_to_unsigned(pkexec_uid, 10, 0, 60000, &uid, &err);

        if (err) {
            g_warning("Defaulting on fallback UID: %s", err->message);
        }

        self->down_uid = uid;
        self->down_gid = uid;
        permissions_manager_set_details(self);
        return;
    }

    const gchar *sudo_uid = g_getenv("SUDO_UID");
    if (installer_is_string_valid(sudo_uid)) {
        g_autoptr(GError) err = NULL;
        // Convert string to guint64
        g_ascii_string_to_unsigned(sudo_uid, 10, 0, 60000, &uid, &err);

        if (err) {
            g_warning("Defaulting on fallback UID: %s", err->message);
        }

        self->down_uid = uid;
        self->down_gid = uid;
        permissions_manager_set_details(self);
    }
}

static void permissions_manager_finalize(GObject *obj) {
    PermissionsManager *self = PERMISSIONS_MANAGER(obj);

    g_free((gchar *) self->home_dir);

    G_OBJECT_CLASS(permissions_manager_parent_class)->finalize(obj);
}

PermissionsManager *permissions_manager_new() {
    return g_object_new(INSTALLER_TYPE_PERMISSIONS_MANAGER, NULL);
}

void permissions_manager_set_details(PermissionsManager *self) {
    g_return_if_fail(PERMISSIONS_IS_MANAGER(self));

    struct passwd *pw = getpwuid(self->down_uid);

    if (!pw) {
        self->home_dir = "/home/live";
        return;
    }

    self->home_dir = pw->pw_dir;
}

gboolean permissions_manager_down_permissions(PermissionsManager *self) {
    g_return_val_if_fail(PERMISSIONS_IS_MANAGER(self), FALSE);

    int result = setresgid(self->down_gid, self->down_gid, 0);
    if (result != 0) {
        g_warning("Failed to restore GID");
        return FALSE;
    }

    result = setresuid(self->down_uid, self->down_uid, 0);
    if (result != 0) {
        g_warning("Failed to restore GID");
        return FALSE;
    }

    gboolean success = g_setenv("HOME", self->home_dir, TRUE);
    if (!success) {
        g_warning("Failed to reset HOME environment variable");
    }
    return success;
}

gboolean permissions_manager_up_permissions(PermissionsManager *self) {
    g_return_val_if_fail(PERMISSIONS_IS_MANAGER(self), FALSE);

    int result = setresgid(0, 0, 0);
    if (result != 0) {
        g_warning("Failed to elevate GID");
        return FALSE;
    }

    result = setresuid(0, 0, 0);
    if (result != 0) {
        g_warning("Failed to elevate UID");
        return FALSE;
    }

    gboolean success = g_setenv("HOME", "/root", TRUE);
    if (!success) {
        g_warning("Failed to set HOME environment variable");
    }
    return success;
}
