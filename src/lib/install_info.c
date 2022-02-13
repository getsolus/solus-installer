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

#include "install_info.h"

enum {
    PROP_EXP_0,
    PROP_OWNER,
    PROP_ENABLE_GEOIP,
    PROP_HOSTNAME,
    PROP_WINDOWS_PRESENT,
    PROP_INSTALL_BOOTLOADER,
    PROP_INVALIDATED,
    N_EXP_PROPERTIES
};

static GParamSpec *props[N_EXP_PROPERTIES] = {NULL};

struct _InstallerInfo {
    GObject parent_instance;
    GObject *owner;

    /* Locale */

    /* Keyboard */

    /* Timezone */

    /* Disk Strategy */

    /* GeoIP */
    gboolean enable_geoip;

    /* Hostname */
    gchar *hostname;

    /* Windows */
    gboolean windows_present;

    /* Users */

    /* Disk Prober */

    /* Bootloader */
    gboolean install_bootloader;

    /* Other */

    gboolean invalidated;
};

G_DEFINE_TYPE(InstallerInfo, installer_info, G_TYPE_OBJECT);

static void installer_info_finalize(GObject *obj);
static void installer_info_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void installer_info_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void installer_info_class_init(InstallerInfoClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = installer_info_finalize;
    class->get_property = installer_info_get_property;
    class->set_property = installer_info_set_property;

    props[PROP_OWNER] = g_param_spec_gtype(
        "owner",
        "Owner",
        "The installer window that this info belongs to",
        G_TYPE_NONE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    props[PROP_ENABLE_GEOIP] = g_param_spec_boolean(
        "enable-geoip",
        "Enable GeoIP",
        "Whether or not GeoIP should be used for locale",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    props[PROP_HOSTNAME] = g_param_spec_string(
        "hostname",
        "Hostname",
        "The hostname to set for the installation",
        "",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );
    
    props[PROP_WINDOWS_PRESENT] = g_param_spec_boolean(
        "windows-present",
        "Windows present",
        "Whether or not Windows is present on this system",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    props[PROP_INSTALL_BOOTLOADER] = g_param_spec_boolean(
        "install-bootloader",
        "Install bootloader",
        "Whether or not we should install a bootloader",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    props[PROP_INVALIDATED] = g_param_spec_boolean(
        "invalidated",
        "Invalidated",
        "Whether or not the installation info is invalid",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    g_object_class_install_properties(class, N_EXP_PROPERTIES, props);
}

static void installer_info_init(InstallerInfo *self) {
    (void) self;
}

static void installer_info_finalize(GObject *obj) {
    InstallerInfo *self = INSTALLER_INFO(obj);

    self->owner = NULL;
    g_free(self->hostname);

    G_OBJECT_CLASS(installer_info_parent_class)->finalize(obj);
}

static void installer_info_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    InstallerInfo *self = INSTALLER_INFO(obj);

    switch (prop_id)
    {
    case PROP_OWNER:
        g_value_set_gtype(val, (GType) self->owner);
        break;

    case PROP_ENABLE_GEOIP:
        g_value_set_boolean(val, self->enable_geoip);
        break;

    case PROP_HOSTNAME:
        g_value_set_string(val, (const gchar *) self->hostname);
        break;

    case PROP_WINDOWS_PRESENT:
        g_value_set_boolean(val, self->windows_present);
        break;

    case PROP_INSTALL_BOOTLOADER:
        g_value_set_boolean(val, self->install_bootloader);
        break;

    case PROP_INVALIDATED:
        g_value_set_boolean(val, self->invalidated);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
        break;
    }
}

static void installer_info_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    InstallerInfo *self = INSTALLER_INFO(obj);

    switch (prop_id)
    {
    case PROP_OWNER:
        if (!(self->owner)) {
            self->owner = (GObject *) g_value_get_gtype(val);
        }
        break;

    case PROP_ENABLE_GEOIP:
        installer_info_set_enable_geoip(self, g_value_get_boolean(val));
        break;

    case PROP_HOSTNAME:
        installer_info_set_hostname(self, g_value_get_string(val));
        break;

    case PROP_WINDOWS_PRESENT:
        installer_info_set_windows_present(self, g_value_get_boolean(val));
        break;

    case PROP_INSTALL_BOOTLOADER:
        installer_info_set_install_bootloader(self, g_value_get_boolean(val));
        break;

    case PROP_INVALIDATED:
        installer_info_set_invalidated(self, g_value_get_boolean(val));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
        break;
    }
}

InstallerInfo *installer_info_new(GObject *owner) {
    return g_object_new(INSTALLER_TYPE_INFO, "owner", owner, NULL);
}

/* Property Getters */

gboolean installer_info_get_enable_geoip(InstallerInfo *self) {
    return self->enable_geoip;
}

gchar *installer_info_get_hostname(InstallerInfo *self) {
    return g_strdup(self->hostname);
}

gboolean installer_info_get_windows_present(InstallerInfo *self) {
    return self->windows_present;
}

gboolean installer_info_get_install_bootloader(InstallerInfo *self) {
    return self->install_bootloader;
}

gboolean installer_info_get_invalidated(InstallerInfo *self) {
    return self->invalidated;
}

/* Property Setters */

void installer_info_set_enable_geoip(InstallerInfo *self, gboolean value) {
    g_return_if_fail(INSTALLER_IS_INFO(self));

    self->enable_geoip = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_ENABLE_GEOIP]);
}

void installer_info_set_hostname(InstallerInfo *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_INFO(self));

    if (!installer_is_string_valid((char *) value)) {
        return;
    }

    if (installer_is_string_valid((char *) self->hostname)) {
        g_free(self->hostname);
    }

    self->hostname = g_strdup(value);

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_HOSTNAME]);
}

void installer_info_set_windows_present(InstallerInfo *self, gboolean value) {
    g_return_if_fail(INSTALLER_IS_INFO(self));

    self->windows_present = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_WINDOWS_PRESENT]);
}

void installer_info_set_install_bootloader(InstallerInfo *self, gboolean value) {
    g_return_if_fail(INSTALLER_IS_INFO(self));

    self->install_bootloader = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_INSTALL_BOOTLOADER]);
}

void installer_info_set_invalidated(InstallerInfo *self, gboolean value) {
    g_return_if_fail(INSTALLER_IS_INFO(self));

    self->invalidated = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_INVALIDATED]);
}
