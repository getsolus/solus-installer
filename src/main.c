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

#include "lib/installer.h"
#include "window.h"

GtkWindow *main_window;

static void on_activate(GtkApplication *app) {
    g_assert(GTK_IS_APPLICATION(app));

    g_autoptr(GError) err = NULL;
    if (!installer_init_blockdev(&err)) {
        if (err) {
            g_critical("Error initializing blockdev library: %s", err->message);
        } else {
            g_critical("Error initializing blockdev library: unknown error");
        }

        return;
    }

    main_window = gtk_application_get_active_window(app);
    if (main_window == NULL) {
        main_window = g_object_new(INSTALLER_TYPE_WINDOW,
                                   "application", app,
                                   "default-width", 768,
                                   "default-height", 500,
                                   NULL);
    }

    gtk_widget_show_all(GTK_WIDGET(main_window));
}

static void on_shutdown(GtkApplication *app) {
    (void) app;
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "us.getsol.Installer",
        G_APPLICATION_FLAGS_NONE
    );

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown), NULL);

    gtk_init(&argc, &argv);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    g_object_unref(app);
    return status;
}
