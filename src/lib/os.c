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

#include "os.h"

enum {
    PROP_EXP_0,
    PROP_OTYPE,
    PROP_NAME,
    PROP_DEVICE_PATH,
    PROP_ICON_NAME,
    N_EXP_PROPERTIES
};

static GParamSpec *props[N_EXP_PROPERTIES] = {NULL};

struct _InstallerOS {
    GObject parent_instance;

    const gchar *otype;
    const gchar *name;
    const gchar *device_path;
    const gchar *icon_name;
};

G_DEFINE_TYPE(InstallerOS, installer_os, G_TYPE_OBJECT);

static void installer_os_finalize(GObject *obj);
static void installer_os_get_property(GObject *obj, guint prop_id, GValue *val,
                                      GParamSpec *spec);
static void installer_os_set_property(GObject *obj, guint prop_id,
                                      const GValue *val, GParamSpec *spec);

static void installer_os_class_init(InstallerOSClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = installer_os_finalize;
    class->get_property = installer_os_get_property;
    class->set_property = installer_os_set_property;

    props[PROP_OTYPE] = g_param_spec_string(
        "otype", "OS Type", "The type of OS, e.g. Windows or Linux", "",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_NAME] = g_param_spec_string(
        "name", "Name", "The full name of the OS", "",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_DEVICE_PATH] = g_param_spec_string(
        "device-path", "Device Path",
        "The path of the device the OS is installed on", "",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_ICON_NAME] = g_param_spec_string(
        "icon-name", "Icon Name", "The icon name for the OS",
        "system-software-install",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_EXP_PROPERTIES, props);
}

static void installer_os_init(InstallerOS *self) {
    (void) self;
}

static void installer_os_finalize(GObject *obj) {
    InstallerOS *self = INSTALLER_OS(obj);

    g_free((gchar *) self->otype);
    g_free((gchar *) self->name);
    g_free((gchar *) self->device_path);
    g_free((gchar *) self->icon_name);

    G_OBJECT_CLASS(installer_os_parent_class)->finalize(obj);
}

static void installer_os_get_property(GObject *obj, guint prop_id, GValue *val,
                                      GParamSpec *spec) {
    InstallerOS *self = INSTALLER_OS(obj);

    switch (prop_id) {
        case PROP_OTYPE:
            g_value_set_string(val, g_strdup(self->otype));
            break;

        case PROP_NAME:
            g_value_set_string(val, g_strdup(self->name));
            break;

        case PROP_DEVICE_PATH:
            g_value_set_string(val, g_strdup(self->device_path));
            break;

        case PROP_ICON_NAME:
            g_value_set_string(val, g_strdup(self->icon_name));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void installer_os_set_property(GObject *obj, guint prop_id,
                                      const GValue *val, GParamSpec *spec) {
    InstallerOS *self = INSTALLER_OS(obj);

    switch (prop_id) {
        case PROP_OTYPE:
            installer_os_set_otype(self, g_value_dup_string(val));
            break;

        case PROP_NAME:
            installer_os_set_name(self, g_value_dup_string(val));
            break;

        case PROP_DEVICE_PATH:
            installer_os_set_device_path(self, g_value_dup_string(val));
            break;

        case PROP_ICON_NAME:
            installer_os_set_icon_name(self, g_value_dup_string(val));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

InstallerOS *installer_os_new(gchar *otype, gchar *name,
                              const gchar *device_path) {
    return g_object_new(INSTALLER_TYPE_OS, "otype", otype, "name", name,
                        "device-path", device_path, NULL);
}

gchar *installer_os_get_otype(InstallerOS *self) {
    return g_strdup(self->otype);
}

gchar *installer_os_get_name(InstallerOS *self) {
    return g_strdup(self->name);
}

const gchar *installer_os_get_device_path(InstallerOS *self) {
    return g_strdup(self->device_path);
}

gchar *installer_os_get_icon_name(InstallerOS *self) {
    return g_strdup(self->icon_name);
}

void installer_os_set_otype(InstallerOS *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_OS(self));

    if (!value) {
        return;
    }

    if (self->otype) {
        g_free((gchar *) self->otype);
    }

    self->otype = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_OTYPE]);
}

void installer_os_set_name(InstallerOS *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_OS(self));

    if (!value) {
        return;
    }

    if (self->name) {
        g_free((gchar *) self->name);
    }

    self->name = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_NAME]);
}

void installer_os_set_device_path(InstallerOS *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_OS(self));

    if (!value) {
        return;
    }

    if (self->device_path) {
        g_free((gchar *) self->device_path);
    }

    self->device_path = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_DEVICE_PATH]);
}

void installer_os_set_icon_name(InstallerOS *self, const gchar *value) {
    g_return_if_fail(INSTALLER_IS_OS(self));

    if (!value) {
        return;
    }

    if (self->icon_name) {
        g_free((gchar *) self->icon_name);
    }

    self->icon_name = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_ICON_NAME]);
}
