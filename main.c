#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <libnotify/notify.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>

static void on_download_started(WebKitWebContext *context, WebKitDownload *download, gpointer user_data) {
    const gchar *uri = webkit_uri_request_get_uri(webkit_download_get_request(download));
    time_t now = time(NULL);

    const gchar *basename = g_path_get_basename(uri);
    const gchar *dot = strrchr(basename, '.');
    const gchar *extension = (dot && dot[1]) ? dot : "";

    gchar *filename = g_strdup_printf("%s/Downloads/%ld%s", g_get_home_dir(), now, extension);
    gchar *destination = g_strdup_printf("file://%s", filename);

    webkit_download_set_destination(download, destination);
    webkit_download_set_allow_overwrite(download, TRUE);

    gchar *summary = g_strdup("Saved");
    gchar *body = g_strdup_printf("%s", filename);
    NotifyNotification *notification = notify_notification_new(summary, body, "document-save");
    notify_notification_set_timeout(notification, 5000); 
    notify_notification_show(notification, NULL);

    g_free(summary);
    g_free(body);
    g_free(filename);
    g_free(destination);
    g_free((gchar *)basename);
}

static gboolean on_decide_policy(WebKitWebView *webview, WebKitPolicyDecision *decision, WebKitPolicyDecisionType type, gpointer user_data) {
    if (type == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
        WebKitNavigationPolicyDecision *nav_decision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
        WebKitNavigationAction *action = webkit_navigation_policy_decision_get_navigation_action(nav_decision);
        WebKitURIRequest *request = webkit_navigation_action_get_request(action);
        const gchar *uri = webkit_uri_request_get_uri(request);

        guint button = webkit_navigation_action_get_mouse_button(action);
        guint modifiers = webkit_navigation_action_get_modifiers(action);

        if (button == 2 || modifiers != 0) {
            gchar *cmd = g_strdup_printf("xdg-open \"%s\" &", uri);
            system(cmd);
            g_free(cmd);

            webkit_policy_decision_ignore(decision);
            return TRUE;
        }
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    notify_init("Disquad");

    WebKitWebsiteDataManager *data_manager = webkit_website_data_manager_new(
        "base-data-directory", g_strdup_printf("%s/.local/share/disquad", g_get_home_dir()),
        "base-cache-directory", g_strdup_printf("%s/.cache/disquad", g_get_home_dir()),
        NULL
    );

    WebKitWebContext *context = webkit_web_context_new_with_website_data_manager(data_manager);
    g_signal_connect(context, "download-started", G_CALLBACK(on_download_started), NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Disquad");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(context));
    g_signal_connect(webview, "decide-policy", G_CALLBACK(on_decide_policy), NULL);

    webkit_web_view_load_uri(webview, "https://discord.com/app");
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(webview));

    gtk_widget_show_all(window);
    gtk_main();

    notify_uninit();
    return 0;
}
