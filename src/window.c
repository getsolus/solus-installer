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
#include "window.h"

struct _InstallerWindow {
    GtkApplicationWindow parent_instance;
    
    GtkCssProvider *provider;

    GtkWidget *header_box;
    GtkWidget *box_labels;

    GtkWidget *stack;
    GtkWidget *installer_page;
    GtkWidget *installer_wrap;

    GtkWidget *image_step;
    GtkWidget *label_step;

    GtkWidget *prev_button;
    GtkWidget *next_button;

    InstallerInfo *info;
    PermissionsManager *perms;
    DiskManager *disk_manager;

    GSList *pages;

    gboolean final_step;
    gboolean skip_forward;
    guint page_index;
    
    gchar *vanity_image;
    gchar *vanity_string;
    gboolean is_plasma;
};

G_DEFINE_TYPE(InstallerWindow, installer_window, GTK_TYPE_APPLICATION_WINDOW);

static void installer_window_finalize(GObject *obj);

static void installer_window_class_init(InstallerWindowClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);
    class->finalize = installer_window_finalize;
}

static void installer_window_init(InstallerWindow *self) {
    installer_window_setup_style(self);

    gtk_window_set_position(GTK_WINDOW(self), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(self), "system-software-install");
    gtk_window_set_title(GTK_WINDOW(self), "Install Solus");

    /* Header */

    self->image_step = gtk_image_new_from_icon_name("system-software-install", GTK_ICON_SIZE_DIALOG);
    g_object_set(self->image_step, "margin", 8, NULL);
    // self->label_step = gtk_label_new("");
    // g_object_set(self->label_step, "margin", 8, NULL);

    self->header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkStyleContext *header_style = gtk_widget_get_style_context(self->header_box);
    gtk_style_context_add_class(header_style, "header-box");
    gtk_box_pack_start(GTK_BOX(self->header_box), self->image_step, FALSE, FALSE, 0);
    // TODO: In the python version, the line adding the step label is commented out

    self->box_labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_valign(self->box_labels, GTK_ALIGN_START);
    gtk_widget_set_margin_top(self->box_labels, 20);
    gtk_widget_set_margin_bottom(self->box_labels, 40);
    gtk_box_pack_start(GTK_BOX(self->header_box), self->box_labels, TRUE, TRUE, 0);

    installer_window_set_vanity(self);

    GtkWidget *vanity_image = gtk_image_new_from_icon_name(self->vanity_image, GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_object_set(vanity_image, "margin", 8, NULL);
    gtk_widget_set_margin_top(vanity_image, 0);
    GtkWidget *vanity_label = gtk_label_new(self->vanity_string);
    gtk_widget_set_margin_start(vanity_label, 4);
    gtk_widget_set_margin_end(vanity_label, 8);
    gtk_widget_set_margin_bottom(vanity_label, 8);

    gtk_box_pack_end(GTK_BOX(self->header_box), vanity_label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(self->header_box), vanity_image, FALSE, FALSE, 0);

    /* Main install page */

    self->installer_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    self->stack = gtk_stack_new();
    gtk_box_pack_start(GTK_BOX(self->installer_page), self->stack, TRUE, TRUE, 0);

    self->installer_wrap = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(self->installer_wrap), self->header_box, FALSE, FALSE, 0);
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(self->installer_wrap), separator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self->installer_wrap), self->installer_page, TRUE, TRUE, 0);

    gtk_stack_set_transition_type(GTK_STACK(self->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    /* Nav buttons */

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_widget_set_margin_top(button_box, 10);
    gtk_widget_set_margin_bottom(button_box, 10);
    gtk_widget_set_margin_end(button_box, 10);

    self->prev_button = gtk_button_new_with_label("Previous");
    gtk_widget_set_sensitive(self->prev_button, FALSE);
    g_signal_connect(self->prev_button, "clicked", G_CALLBACK(installer_window_page_prev), self);
    self->next_button = gtk_button_new_with_label("Next");
    g_signal_connect(self->next_button, "clicked", G_CALLBACK(installer_window_page_next), self);

    gtk_widget_set_margin_start(self->prev_button, 4);
    gtk_widget_set_margin_start(self->next_button, 4);

    gtk_box_pack_start(GTK_BOX(button_box), self->prev_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), self->next_button, FALSE, FALSE, 0);

    /* Nav separator */

    GtkWidget *separator_nav = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator_nav, 20);

    /* Pack nav items */

    gtk_box_pack_end(GTK_BOX(self->installer_page), button_box, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(self->installer_page), separator_nav, FALSE, FALSE, 0);

    /* Pack everything into the top-level */

    self->info = installer_info_new(G_OBJECT(self));

    gtk_container_add(GTK_CONTAINER(self), self->installer_wrap);

    // TODO: Add the installer pages

    installer_window_buttons_update_sensitivity(self);

    self->perms = permissions_manager_new();
    self->disk_manager = disk_manager_new();
    disk_manager_scan_parts(self->disk_manager);

    // TODO: Update current page

    // TODO: Start our threads
}

static void installer_window_finalize(GObject *obj) {
    InstallerWindow *self = INSTALLER_WINDOW(obj);

    g_object_unref(self->provider);
    g_object_unref(self->disk_manager);
    g_object_unref(self->perms);
    g_object_unref(self->info);
    g_slist_free(self->pages);

    G_OBJECT_CLASS(installer_window_parent_class)->finalize(obj);
}

void installer_window_setup_style(InstallerWindow *self) {
    if (!INSTALLER_IS_WINDOW(self)) {
        return;
    }

    if (!GTK_IS_CSS_PROVIDER(self->provider)) {
        self->provider = gtk_css_provider_new();
    }

    // Load our custom CSS
    gtk_css_provider_load_from_resource(self->provider, "/us/getsol/installer/style.css");

    // Set our dark theme preference
    g_autoptr(GtkSettings) settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);

    // Set our styles
    GtkStyleContext *style_context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style_context, "installer-window");

    // Get the current theme
    g_autofree gchar *theme = NULL;
    g_object_get(settings, "gtk-theme-name", &theme, NULL);
    g_autofree gchar *theme_lower = g_utf8_casefold(theme, -1);

    // Check if this is an Arc theme, and apply arc styles
    if (strncmp("arc", theme_lower, 3) == 0) {
        gtk_style_context_add_class(style_context, "arc-theme");
    }
}

void installer_window_set_vanity(InstallerWindow *self) {
    if (!INSTALLER_IS_WINDOW(self)) {
        return;
    }

    self->vanity_image = "start-here-solus";
    self->vanity_string = "Solus";
    self->is_plasma = FALSE;

    g_autoptr(GFile) file = g_file_new_for_path("/usr/bin/budgie-panel");
    if (g_file_query_exists(file, NULL)) {
        self->vanity_image = "budgie-desktop-symbolic";
        self->vanity_string = "Solus Budgie";
        return;
    }

    file = g_file_new_for_path("/usr/bin/gnome-shell");
    if (g_file_query_exists(file, NULL)) {
        self->vanity_image = "desktop-environment-gnome";
        self->vanity_string = "Solus GNOME";
        return;
    }

    file = g_file_new_for_path("/usr/bin/mate-panel");
    if (g_file_query_exists(file, NULL)) {
        self->vanity_image = "mate";
        self->vanity_string = "Solus MATE";
        return;
    }

    file = g_file_new_for_path("/usr/bin/plasmashell");
    if (g_file_query_exists(file, NULL)) {
        self->vanity_image = "plasma";
        self->vanity_string = "Solus Plasma";
        self->is_plasma = TRUE;
        return;
    }
}

void installer_window_buttons_update_sensitivity(InstallerWindow *self) {
    g_return_if_fail(INSTALLER_IS_WINDOW(self));

    gboolean prev_sensitive;
    gboolean next_sensitive;

    if (self->page_index == 0) {
        prev_sensitive = FALSE;
    } else {
        prev_sensitive = TRUE;
    }

    if (self->page_index < g_slist_length(self->pages)) {
        next_sensitive = TRUE;
    } else {
        next_sensitive = FALSE;
    }

    gtk_widget_set_sensitive(self->prev_button, prev_sensitive);
    gtk_widget_set_sensitive(self->next_button, next_sensitive);
}

gpointer installer_window_page_next_callback(InstallerWindow *self) {
    installer_window_page_next(self);
    return NULL;
}

void installer_window_page_next(InstallerWindow *self) {
    g_return_if_fail(INSTALLER_IS_WINDOW(self));

    // Check if we're on the final step, showing a warning message if so
    // to confirm that the user wishes to install Solus with the chosen
    // parameters.
    if (self->final_step) {
        const gchar *message = "Installation will make changes to your disks, and could " \
                               "result in data loss.\nDo you wish to install?";
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(self),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_CANCEL,
            message
        );

        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result != GTK_RESPONSE_OK) {
            return;
        }
    }

    self->skip_forward = TRUE;
    guint index = self->page_index + 1;

    if (index >= g_slist_length(self->pages)) {
        return;
    }

    // TODO: Get the page at the new index

    // TODO: Check if the new page is hidden, incrementing the index again if so

    self->page_index = index;

    // TODO: Update the current page
}

gpointer installer_window_page_prev_callback(InstallerWindow *self) {
    installer_window_page_prev(self);
    return NULL;
}

void installer_window_page_prev(InstallerWindow *self) {
    g_return_if_fail(INSTALLER_IS_WINDOW(self));

    self->skip_forward = FALSE;

    if (self->page_index == 0) {
        return;
    }

    guint index = self->page_index - 1;

    // TODO: Get the page at the new index

    // TODO: Check if the page is hidden, decrementing the index again if so

    self->page_index = index;

    // TODO: Update the current page
}
