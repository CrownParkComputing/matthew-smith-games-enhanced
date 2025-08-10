#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <unistd.h>
#include <fstream>

namespace fs = std::filesystem;

class GameLauncherGUI {
private:
    GtkApplication* app;
    GtkWindow* window;
    GtkBox* mainBox;
    GtkHeaderBar* headerBar;
    bool isFullscreen;
    
    // Use const static strings to reduce memory allocation
    static const std::string PROJECT_DIR_SUFFIX;
    static const std::string MANIC_PATH_SUFFIX;
    static const std::string JETSET_PATH_SUFFIX;
    static const std::string IMAGE_DIR_SUFFIX;
    static const std::string MANIC_IMAGE_NAME;
    static const std::string JETSET_IMAGE_NAME;
    
    std::string homeDir;
    
    // Memory monitoring
    size_t getMemoryUsage() const {
        std::ifstream statm("/proc/self/statm");
        size_t size, resident, share, text, lib, data, dt;
        statm >> size >> resident >> share >> text >> lib >> data >> dt;
        return resident * getpagesize() / 1024; // KB
    }
    
    void logMemoryUsage(const std::string& context) const {
        std::cout << "[MEMORY] " << context << ": " << getMemoryUsage() << " KB" << std::endl;
    }
    
    std::string getGamePath(const std::string& gameType) const {
        if (gameType == "manic") {
            return homeDir + PROJECT_DIR_SUFFIX + MANIC_PATH_SUFFIX;
        } else if (gameType == "jetset") {
            return homeDir + PROJECT_DIR_SUFFIX + JETSET_PATH_SUFFIX;
        }
        return "";
    }
    
    std::string getImagePath(const std::string& gameType) const {
        if (gameType == "manic") {
            return homeDir + IMAGE_DIR_SUFFIX + MANIC_IMAGE_NAME;
        } else if (gameType == "jetset") {
            return homeDir + IMAGE_DIR_SUFFIX + JETSET_IMAGE_NAME;
        }
        return "";
    }
    
    bool checkGameExists(const std::string& path) const {
        return fs::exists(path) && fs::is_regular_file(path);
    }
    
    bool checkImageExists(const std::string& path) const {
        return fs::exists(path) && fs::is_regular_file(path);
    }
    
    // Struct to pass data to async callback
    struct GameLaunchData {
        GameLauncherGUI* launcher;
        std::string gameName;
        GtkWidget* statusDialog;
        
        GameLaunchData(GameLauncherGUI* l, const std::string& name, GtkWidget* dialog) 
            : launcher(l), gameName(name), statusDialog(dialog) {}
    };
    
    static void onGameLaunchFinished(GObject* source, GAsyncResult* result, gpointer userData) {
        auto* data = static_cast<GameLaunchData*>(userData);
        data->launcher->logMemoryUsage("Async Game Launch Finished");
        
        GError* error = nullptr;
        gboolean success = g_subprocess_wait_finish(G_SUBPROCESS(source), result, &error);
        
        // Close the launch status dialog
        if (data->statusDialog) {
            gtk_window_destroy(GTK_WINDOW(data->statusDialog));
        }
        
        // Show completion dialog
        const char* message;
        GtkMessageType msgType;
        
        if (success && !error) {
            int exitStatus = g_subprocess_get_exit_status(G_SUBPROCESS(source));
            if (exitStatus == 0) {
                message = "Game finished successfully!";
                msgType = GTK_MESSAGE_INFO;
            } else {
                message = "Game exited with error code";
                msgType = GTK_MESSAGE_WARNING;
            }
        } else {
            message = "Failed to launch game";
            msgType = GTK_MESSAGE_ERROR;
            if (error) {
                std::cerr << "Launch error: " << error->message << std::endl;
                g_error_free(error);
            }
        }
        
        auto resultDialog = gtk_message_dialog_new(
            data->launcher->window,
            GTK_DIALOG_MODAL,
            msgType,
            GTK_BUTTONS_OK,
            "%s: %s",
            data->gameName.c_str(),
            message
        );
        
        gtk_window_present(GTK_WINDOW(resultDialog));
        g_signal_connect(resultDialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
        
        data->launcher->logMemoryUsage("Game Launch Complete");
        delete data;
    }
    
    static void onGameLaunch(GtkButton* button, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->logMemoryUsage("Async Game Launch Start");
        
        const char* gameType = (const char*)g_object_get_data(G_OBJECT(button), "game_type");
        if (!gameType) {
            std::cerr << "Error: No game type data found" << std::endl;
            return;
        }
        
        std::string gamePath = launcher->getGamePath(gameType);
        std::string gameName = (std::string(gameType) == "manic") ? "Manic Miner" : "Jet Set Willy";
        
        if (!launcher->checkGameExists(gamePath)) {
            launcher->showErrorDialog(gameName, gamePath);
            return;
        }
        
        launcher->launchGameAsync(gamePath, gameName);
    }
    
    void showErrorDialog(const std::string& gameName, const std::string& gamePath) {
        auto dialog = gtk_message_dialog_new(
            window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Game Not Found!\n\n%s executable not found at:\n%s\n\nPlease build the game first using 'make' in the game directory.",
            gameName.c_str(),
            gamePath.c_str()
        );
        
        gtk_window_present(GTK_WINDOW(dialog));
        g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    }
    
    void launchGameAsync(const std::string& gamePath, const std::string& gameName) {
        // Minimize the main window to avoid launch conflicts
        logMemoryUsage("Before window minimize");
        gtk_window_minimize(window);
        
        // Process pending events to ensure minimize takes effect
        while (g_main_context_pending(NULL)) {
            g_main_context_iteration(NULL, FALSE);
        }
        
        // Small delay to ensure window minimize completes
        g_usleep(200000); // 200ms
        
        // Create launch status dialog (non-modal)
        auto statusDialog = gtk_message_dialog_new(
            nullptr, // No parent to avoid blocking
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_NONE,
            "Launching %s...\nGame is running in background.",
            gameName.c_str()
        );
        
        gtk_window_present(GTK_WINDOW(statusDialog));
        
        logMemoryUsage("Before async game launch");
        
        // Create subprocess launcher
        GError* error = nullptr;
        const char* argv[] = {gamePath.c_str(), nullptr};
        
        auto subprocess = g_subprocess_newv(
            argv,
            G_SUBPROCESS_FLAGS_NONE,
            &error
        );
        
        if (!subprocess) {
            gtk_window_destroy(GTK_WINDOW(statusDialog));
            
            auto errorDialog = gtk_message_dialog_new(
                window,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Failed to launch %s:\n%s",
                gameName.c_str(),
                error ? error->message : "Unknown error"
            );
            
            gtk_window_present(GTK_WINDOW(errorDialog));
            g_signal_connect(errorDialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
            
            if (error) g_error_free(error);
            return;
        }
        
        // Create callback data
        auto* callbackData = new GameLaunchData(this, gameName, statusDialog);
        
        // Wait for process to finish asynchronously
        g_subprocess_wait_async(
            subprocess,
            nullptr,
            onGameLaunchFinished,
            callbackData
        );
        
        g_object_unref(subprocess);
        logMemoryUsage("Async launch started");
    }
    
    // Simplified controls dialog with shorter text
    static void onControlsShow(GtkButton*, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->logMemoryUsage("Controls Dialog Start");
        
        auto controlsWindow = gtk_window_new();
        gtk_window_set_title(GTK_WINDOW(controlsWindow), "Controls");
        gtk_window_set_default_size(GTK_WINDOW(controlsWindow), 450, 350);
        gtk_window_set_transient_for(GTK_WINDOW(controlsWindow), launcher->window);
        gtk_window_set_modal(GTK_WINDOW(controlsWindow), TRUE);
        
        auto textView = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_WORD);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textView), 15);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textView), 15);
        
        auto buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
        
        // Shorter text to reduce memory usage
        const char* controlsText = 
            "CONTROLS\n"
            "========\n\n"
            "Movement: Arrow Keys\n"
            "Jump: Space\n"
            "Confirm: Enter\n"
            "Pause: TAB\n"
            "Fullscreen: F11\n"
            "Mute: M\n"
            "Exit: Escape\n\n"
            "Games run asynchronously - you can\n"
            "use the launcher while they're running.";
            
        gtk_text_buffer_set_text(buffer, controlsText, -1);
        gtk_window_set_child(GTK_WINDOW(controlsWindow), textView);
        
        gtk_window_present(GTK_WINDOW(controlsWindow));
        
        launcher->logMemoryUsage("Controls Dialog End");
    }
    
    static void onFullscreenToggle(GtkButton* button, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->toggleFullscreen(button);
    }
    
    void toggleFullscreen(GtkButton* button = nullptr) {
        isFullscreen = !isFullscreen;
        
        if (isFullscreen) {
            gtk_window_fullscreen(window);
            if (button) gtk_button_set_label(button, "🪟 Windowed");
        } else {
            gtk_window_unfullscreen(window);
            if (button) gtk_button_set_label(button, "⛶ Fullscreen");
        }
        
        // Update header bar button if F11 was pressed
        if (!button) {
            GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(headerBar));
            while (child) {
                if (GTK_IS_BUTTON(child)) {
                    const char* label = gtk_button_get_label(GTK_BUTTON(child));
                    if (label && (g_str_has_prefix(label, "⛶") || g_str_has_prefix(label, "🪟"))) {
                        gtk_button_set_label(GTK_BUTTON(child), isFullscreen ? "🪟 Windowed" : "⛶ Fullscreen");
                        break;
                    }
                }
                child = gtk_widget_get_next_sibling(child);
            }
        }
    }
    
    static gboolean onKeyPressed(GtkEventControllerKey*, guint keyval, guint, GdkModifierType, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        
        if (keyval == GDK_KEY_F11) {
            launcher->toggleFullscreen();
            return TRUE;
        }
        
        return FALSE;
    }
    
    static void onAboutShow(GtkButton*, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->logMemoryUsage("About Dialog");
        
        auto aboutDialog = gtk_about_dialog_new();
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), "Matthew Smith Games");
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutDialog), "2.0 (Async)");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutDialog), 
            "Enhanced Manic Miner & Jet Set Willy\n"
            "Classic ZX Spectrum games with async launching.");
        gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutDialog), "Original games © Matthew Smith");
        
        const char* authors[] = {"Matthew Smith", "Enhanced Ports", NULL};
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(aboutDialog), authors);
        
        gtk_window_set_transient_for(GTK_WINDOW(aboutDialog), launcher->window);
        gtk_window_set_modal(GTK_WINDOW(aboutDialog), TRUE);
        gtk_window_present(GTK_WINDOW(aboutDialog));
        
        g_signal_connect(aboutDialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    }
    
    GtkWidget* createGameCard(const std::string& gameName, const std::string& gameYear, 
                              const std::string& gameDescription, const std::string& gameType, bool gameExists) {
        
        auto frame = gtk_frame_new(NULL);
        gtk_widget_set_margin_start(frame, 8);
        gtk_widget_set_margin_end(frame, 8);
        gtk_widget_set_margin_top(frame, 8);
        gtk_widget_set_margin_bottom(frame, 8);
        
        auto cardBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(cardBox, 12);
        gtk_widget_set_margin_end(cardBox, 12);
        gtk_widget_set_margin_top(cardBox, 12);
        gtk_widget_set_margin_bottom(cardBox, 12);
        gtk_frame_set_child(GTK_FRAME(frame), cardBox);
        
        auto contentBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_box_append(GTK_BOX(cardBox), contentBox);
        
        // Add image if it exists (smaller size for memory efficiency)
        std::string imagePath = getImagePath(gameType);
        if (checkImageExists(imagePath)) {
            auto image = gtk_image_new_from_file(imagePath.c_str());
            gtk_widget_set_size_request(image, 140, 170); // Even smaller
            gtk_box_append(GTK_BOX(contentBox), image);
        } else {
            auto placeholder = gtk_label_new("📦\nNo Image");
            gtk_label_set_justify(GTK_LABEL(placeholder), GTK_JUSTIFY_CENTER);
            gtk_widget_set_size_request(placeholder, 140, 170);
            gtk_widget_add_css_class(placeholder, "placeholder-image");
            gtk_box_append(GTK_BOX(contentBox), placeholder);
        }
        
        auto infoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_hexpand(infoBox, TRUE);
        gtk_widget_set_valign(infoBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(contentBox), infoBox);
        
        // Game title
        std::string titleText = "<span size='large' weight='bold'>" + gameName + " (" + gameYear + ")</span>";
        auto titleLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(titleLabel), titleText.c_str());
        gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
        gtk_widget_add_css_class(titleLabel, "card-title");
        gtk_box_append(GTK_BOX(infoBox), titleLabel);
        
        // Shorter description for memory efficiency
        auto descLabel = gtk_label_new(gameDescription.c_str());
        gtk_label_set_wrap(GTK_LABEL(descLabel), TRUE);
        gtk_label_set_wrap_mode(GTK_LABEL(descLabel), PANGO_WRAP_WORD);
        gtk_widget_set_halign(descLabel, GTK_ALIGN_START);
        gtk_label_set_xalign(GTK_LABEL(descLabel), 0.0);
        gtk_widget_add_css_class(descLabel, "card-description");
        gtk_box_append(GTK_BOX(infoBox), descLabel);
        
        // Status label
        auto statusLabel = gtk_label_new(gameExists ? "✅ Ready (Async)" : "❌ Not Built");
        gtk_widget_add_css_class(statusLabel, gameExists ? "success-text" : "error-text");
        gtk_widget_set_halign(statusLabel, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(infoBox), statusLabel);
        
        // Launch button
        auto launchButton = gtk_button_new_with_label(gameExists ? "🚀 Launch Game" : "⚠️ Build Required");
        gtk_widget_set_sensitive(launchButton, gameExists);
        gtk_widget_set_halign(launchButton, GTK_ALIGN_START);
        gtk_widget_add_css_class(launchButton, "suggested-action");
        g_object_set_data_full(G_OBJECT(launchButton), "game_type", g_strdup(gameType.c_str()), g_free);
        g_signal_connect(launchButton, "clicked", G_CALLBACK(onGameLaunch), this);
        gtk_box_append(GTK_BOX(infoBox), launchButton);
        
        return frame;
    }
    
    void setupCSS() {
        auto cssProvider = gtk_css_provider_new();
        // Minimal CSS for efficiency
        const char* css = 
            ".placeholder-image { font-size: 20px; background-color: #f5f5f5; border: 1px dashed #bbb; }"
            ".success-text { color: #2e7d32; font-weight: bold; }"
            ".error-text { color: #c62828; font-weight: bold; }"
            "window { background-color: #fafafa; }"
            "headerbar { background-color: #e1f5fe; color: #01579b; }"
            "frame { border: 1px solid #e0e0e0; background-color: #ffffff; }"
            ".card-title { color: #1565c0; font-weight: bold; }";
            
        gtk_css_provider_load_from_string(cssProvider, css);
        gtk_style_context_add_provider_for_display(
            gdk_display_get_default(),
            GTK_STYLE_PROVIDER(cssProvider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
        g_object_unref(cssProvider);
    }
    
public:
    GameLauncherGUI() : app(nullptr), window(nullptr), mainBox(nullptr), headerBar(nullptr), isFullscreen(false) {
        logMemoryUsage("Constructor Start");
        
        const char* home = getenv("HOME");
        homeDir = home ? std::string(home) : ("/home/" + std::string(getenv("USER")));
        
        logMemoryUsage("Constructor End");
    }
    
    ~GameLauncherGUI() {
        logMemoryUsage("Destructor");
    }
    
    static void activate(GtkApplication* app, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->createWindow(app);
    }
    
    void createWindow(GtkApplication* gtkApp) {
        logMemoryUsage("Window Creation Start");
        
        app = gtkApp;
        
        window = GTK_WINDOW(gtk_application_window_new(gtkApp));
        gtk_window_set_title(window, "Matthew Smith Games (Async)");
        gtk_window_set_default_size(window, 950, 650); // Smaller window
        gtk_window_set_resizable(window, TRUE);
        
        // Add keyboard event controller
        auto keyController = gtk_event_controller_key_new();
        g_signal_connect(keyController, "key-pressed", G_CALLBACK(onKeyPressed), this);
        gtk_widget_add_controller(GTK_WIDGET(window), keyController);
        
        setupCSS();
        
        // Create header bar
        headerBar = GTK_HEADER_BAR(gtk_header_bar_new());
        gtk_header_bar_set_title_widget(headerBar, gtk_label_new("Matthew Smith Games (Async)"));
        gtk_window_set_titlebar(window, GTK_WIDGET(headerBar));
        
        // Add header buttons
        auto controlsBtn = gtk_button_new_with_label("🎮 Controls");
        auto aboutBtn = gtk_button_new_with_label("ℹ️ About");
        auto fullscreenBtn = gtk_button_new_with_label("⛶ Fullscreen");
        
        gtk_header_bar_pack_end(headerBar, aboutBtn);
        gtk_header_bar_pack_end(headerBar, controlsBtn);
        gtk_header_bar_pack_start(headerBar, fullscreenBtn);
        
        g_signal_connect(controlsBtn, "clicked", G_CALLBACK(onControlsShow), this);
        g_signal_connect(aboutBtn, "clicked", G_CALLBACK(onAboutShow), this);
        g_signal_connect(fullscreenBtn, "clicked", G_CALLBACK(onFullscreenToggle), this);
        
        // Create main container
        auto scrolled = gtk_scrolled_window_new();
        gtk_window_set_child(window, scrolled);
        
        mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(mainBox));
        
        // Add title
        auto titleLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(titleLabel), 
            "<span size='x-large' weight='bold'>Classic ZX Spectrum Games</span>");
        gtk_widget_set_margin_top(titleLabel, 15);
        gtk_widget_set_margin_bottom(titleLabel, 5);
        gtk_box_append(mainBox, titleLabel);
        
        // Add subtitle
        auto subtitleLabel = gtk_label_new("Launch games asynchronously (F11 = fullscreen):");
        gtk_widget_set_margin_bottom(subtitleLabel, 15);
        gtk_box_append(mainBox, subtitleLabel);
        
        // Add game cards
        std::string manicPath = getGamePath("manic");
        std::string jetsetPath = getGamePath("jetset");
        
        bool manicExists = checkGameExists(manicPath);
        bool jetsetExists = checkGameExists(jetsetPath);
        
        auto manicCard = createGameCard(
            "Manic Miner", "1983",
            "Platform adventure. Runs independently.",
            "manic", manicExists
        );
        gtk_box_append(mainBox, manicCard);
        
        auto jetsetCard = createGameCard(
            "Jet Set Willy", "1984",
            "Mansion exploration. Non-blocking launch.",
            "jetset", jetsetExists
        );
        gtk_box_append(mainBox, jetsetCard);
        
        gtk_window_present(window);
        
        logMemoryUsage("Window Creation End");
    }
    
    int run(int argc, char** argv) {
        logMemoryUsage("Application Start");
        
        app = gtk_application_new("com.games.matthew-smith-launcher-async", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), this);
        
        int status = g_application_run(G_APPLICATION(app), argc, argv);
        g_object_unref(app);
        
        logMemoryUsage("Application End");
        return status;
    }
};

// Static member definitions
const std::string GameLauncherGUI::PROJECT_DIR_SUFFIX = "/matthew-smith-games-enhanced";
const std::string GameLauncherGUI::MANIC_PATH_SUFFIX = "/ManicMiner/manicminer";
const std::string GameLauncherGUI::JETSET_PATH_SUFFIX = "/JetSetWilly/jetsetwilly";
const std::string GameLauncherGUI::IMAGE_DIR_SUFFIX = "/Downloads";
const std::string GameLauncherGUI::MANIC_IMAGE_NAME = "/Manic_miner_bugbyte.jpg";
const std::string GameLauncherGUI::JETSET_IMAGE_NAME = "/Jet_Set_Willy_(game_box_art).jpg";

int main(int argc, char** argv) {
    try {
        GameLauncherGUI launcher;
        return launcher.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
