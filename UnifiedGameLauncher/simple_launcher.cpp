#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

namespace fs = std::filesystem;

class UnifiedGameLauncher {
private:
    GtkApplication* app;
    GtkWindow* window;
    GtkBox* mainBox;
    GtkHeaderBar* headerBar;
    
    std::string homeDir;
    std::string scoresDir;
    std::string projectDir;
    
    // Score entry structure
    struct ScoreEntry {
        std::string name;
        int value;
        std::string date;
        
        ScoreEntry(const std::string& n = "", int v = 0, const std::string& d = "") 
            : name(n), value(v), date(d) {}
    };
    
    // Game statistics tracking
    struct GameStats {
        std::string gameName;
        int playCount = 0;
        std::chrono::seconds totalPlayTime{0};
        std::chrono::system_clock::time_point lastPlayed;
        
        std::string formatPlayTime() const {
            auto hours = std::chrono::duration_cast<std::chrono::hours>(totalPlayTime);
            auto mins = std::chrono::duration_cast<std::chrono::minutes>(totalPlayTime % std::chrono::hours(1));
            if (hours.count() > 0) {
                return std::to_string(hours.count()) + "h " + std::to_string(mins.count()) + "m";
            }
            return std::to_string(mins.count()) + "m";
        }
    };
    
    std::vector<GameStats> gameStats;
    std::string statsFilePath;
    
    // High score tracking
    std::vector<ScoreEntry> manicMinerScores;
    std::vector<ScoreEntry> jetSetWillyScores;
    
    std::string getCurrentDate() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d");
        return ss.str();
    }
    
    void ensureScoresDirectory() {
        scoresDir = homeDir + "/.config/matthew-smith-games";
        fs::create_directories(scoresDir);
    }
    
    void loadScores(const std::string& filename, std::vector<ScoreEntry>& scores) {
        std::string filepath = scoresDir + "/" + filename;
        std::ifstream file(filepath);
        if (!file.is_open()) return;
        
        scores.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string name, date;
            int value;
            
            if (iss >> std::quoted(name) >> value >> std::quoted(date)) {
                scores.emplace_back(name, value, date);
            }
        }
        
        // Sort by value (descending)
        std::sort(scores.begin(), scores.end(), 
                  [](const ScoreEntry& a, const ScoreEntry& b) {
                      return a.value > b.value;
                  });
    }
    
    void saveScores(const std::string& filename, const std::vector<ScoreEntry>& scores) {
        ensureScoresDirectory();
        
        std::string filepath = scoresDir + "/" + filename;
        std::ofstream file(filepath);
        if (!file.is_open()) return;
        
        for (const auto& entry : scores) {
            file << std::quoted(entry.name) << " " 
                 << entry.value << " " 
                 << std::quoted(entry.date) << "\n";
        }
    }
    
    void addScore(std::vector<ScoreEntry>& scores, const std::string& name, int value) {
        scores.emplace_back(name, value, getCurrentDate());
        
        // Sort by value (descending)
        std::sort(scores.begin(), scores.end(), 
                  [](const ScoreEntry& a, const ScoreEntry& b) {
                      return a.value > b.value;
                  });
        
        // Keep only top 10
        if (scores.size() > 10) {
            scores.resize(10);
        }
    }
    
    std::string formatScoresDisplay(const std::vector<ScoreEntry>& scores, const std::string& unit = "points") {
        if (scores.empty()) {
            return "\nNo scores recorded yet";
        }
        
        std::string display = "\n🏆 High Scores:\n";
        for (size_t i = 0; i < scores.size() && i < 5; ++i) {
            display += std::to_string(i + 1) + ". " + scores[i].name + ": " + 
                      std::to_string(scores[i].value) + " " + unit;
            if (!scores[i].date.empty()) {
                display += " (" + scores[i].date + ")";
            }
            display += "\n";
        }
        
        return display;
    }
    
    void loadGameStats() {
        statsFilePath = homeDir + "/.config/matthew-smith-games-unified-stats";
        gameStats.clear();
        
        // Initialize default stats
        gameStats.push_back({"Manic Miner", 0, std::chrono::seconds(0), std::chrono::system_clock::now()});
        gameStats.push_back({"Jet Set Willy", 0, std::chrono::seconds(0), std::chrono::system_clock::now()});
        
        std::ifstream file(statsFilePath);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string name;
            int count;
            long seconds;
            long timePoint;
            
            if (iss >> name >> count >> seconds >> timePoint) {
                for (auto& stat : gameStats) {
                    if (stat.gameName == name) {
                        stat.playCount = count;
                        stat.totalPlayTime = std::chrono::seconds(seconds);
                        stat.lastPlayed = std::chrono::system_clock::from_time_t(timePoint);
                        break;
                    }
                }
            }
        }
    }
    
    void saveGameStats() {
        std::string configDir = homeDir + "/.config";
        fs::create_directories(configDir);
        
        std::ofstream file(statsFilePath);
        if (!file.is_open()) return;
        
        for (const auto& stat : gameStats) {
            file << stat.gameName << " " 
                 << stat.playCount << " "
                 << stat.totalPlayTime.count() << " "
                 << std::chrono::system_clock::to_time_t(stat.lastPlayed) << "\n";
        }
    }
    
    GameStats* getGameStats(const std::string& gameName) {
        for (auto& stat : gameStats) {
            if (stat.gameName == gameName) {
                return &stat;
            }
        }
        return nullptr;
    }
    
    void checkForLastScore(const std::string& game) {
        std::string scoreFile;
        std::vector<ScoreEntry>* targetScores;
        std::string persistentScoreFile;
        
        if (game == "manic") {
            scoreFile = scoresDir + "/manic_miner_last_score.txt";
            targetScores = &manicMinerScores;
            persistentScoreFile = "manic_miner_scores.txt";
        } else if (game == "jetset") {
            scoreFile = scoresDir + "/jet_set_willy_last_score.txt";
            targetScores = &jetSetWillyScores;
            persistentScoreFile = "jet_set_willy_scores.txt";
        } else {
            return;
        }
        
        std::ifstream file(scoreFile);
        if (file.is_open()) {
            int score, highScore;
            if (file >> score >> highScore) {
                // Add the score with a default name and current date
                addScore(*targetScores, "Player", score);
                saveScores(persistentScoreFile, *targetScores);
                
                // Show notification
                std::string unit = (game == "manic") ? "points" : "items";
                showInfoDialog("New Score Recorded", 
                    "Your final " + unit + ": " + std::to_string(score) + 
                    "\nThis score has been automatically added to the high score table.");
            }
            file.close();
            
            // Remove the temporary score file
            std::remove(scoreFile.c_str());
        }
    }
    
    void refreshInterface() {
        // Reload scores and update display
        loadScores("manic_miner_scores.txt", manicMinerScores);
        loadScores("jet_set_willy_scores.txt", jetSetWillyScores);
        
        // For a full implementation, we'd rebuild the game cards here
        // For now, we'll just show a message
        std::cout << "Interface refreshed with updated scores" << std::endl;
    }
    
    // Dialog functions
    void showInfoDialog(const std::string& title, const std::string& message) {
        auto dialog = gtk_window_new();
        gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
        gtk_window_set_default_size(GTK_WINDOW(dialog), 350, 150);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        
        auto vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        gtk_widget_set_margin_start(vbox, 20);
        gtk_widget_set_margin_end(vbox, 20);
        gtk_widget_set_margin_top(vbox, 20);
        gtk_widget_set_margin_bottom(vbox, 20);
        gtk_window_set_child(GTK_WINDOW(dialog), vbox);
        
        auto label = gtk_label_new(message.c_str());
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(vbox), label);
        
        auto okButton = gtk_button_new_with_label("OK");
        gtk_widget_set_halign(okButton, GTK_ALIGN_CENTER);
        gtk_widget_add_css_class(okButton, "suggested-action");
        gtk_box_append(GTK_BOX(vbox), okButton);
        
        g_signal_connect_swapped(okButton, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
        
        gtk_window_present(GTK_WINDOW(dialog));
    }
    
    // Game launch functions
    static void onGameLaunch(GtkButton* button, gpointer userData) {
        auto* launcher = static_cast<UnifiedGameLauncher*>(userData);
        const char* gameType = (const char*)g_object_get_data(G_OBJECT(button), "game_type");
        
        if (!gameType) {
            std::cerr << "Error: No game type data found" << std::endl;
            return;
        }
        
        std::string gameName = (std::string(gameType) == "manic") ? "Manic Miner" : "Jet Set Willy";
        launcher->launchGame(gameType, gameName);
    }
    
    void launchGame(const std::string& gameType, const std::string& gameName) {
        // Build game executable path
        std::string gamePath;
        if (gameType == "manic") {
            gamePath = projectDir + "/ManicMiner/manicminer";
        } else if (gameType == "jetset") {
            gamePath = projectDir + "/JetSetWilly/jetsetwilly";
        } else {
            showInfoDialog("Error", "Unknown game type: " + gameType);
            return;
        }
        
        // Check if game exists
        if (!fs::exists(gamePath) || !fs::is_regular_file(gamePath)) {
            showInfoDialog("Game Not Found", 
                gameName + " executable not found at:\n" + gamePath + 
                "\n\nPlease build the game first using 'make' in the game directory.");
            return;
        }
        
        // Hide the launcher window
        gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
        
        // Record start time
        auto startTime = std::chrono::system_clock::now();
        
        // Launch the game using fork/exec
        pid_t pid = fork();
        if (pid == 0) {
            // Child process - execute the game
            execl(gamePath.c_str(), gamePath.c_str(), nullptr);
            // If we get here, exec failed
            std::cerr << "Failed to execute " << gamePath << std::endl;
            exit(1);
        } else if (pid > 0) {
            // Parent process - wait for child to finish
            int status;
            waitpid(pid, &status, 0);
            
            // Record end time and update stats
            auto endTime = std::chrono::system_clock::now();
            auto playDuration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
            
            auto* stats = getGameStats(gameName);
            if (stats) {
                stats->playCount++;
                stats->totalPlayTime += playDuration;
                stats->lastPlayed = endTime;
                saveGameStats();
            }
            
            // Show the launcher window again
            gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
            gtk_window_present(window);
            
            // Check for new high scores (this will happen automatically due to our file-based approach)
            checkForLastScore((gameType == "manic") ? "manic" : "jetset");
            
            // Refresh interface with any new scores
            refreshInterface();
            
        } else {
            // Fork failed
            showInfoDialog("Launch Error", "Failed to launch " + gameName);
            gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
        }
    }
    
    // UI Creation
    GtkWidget* createGameCard(const std::string& gameName, const std::string& gameYear, 
                              const std::string& gameDescription, const std::string& gameType) {
        
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
        
        // Game title
        std::string titleText = "<span size='large' weight='bold'>" + gameName + " (" + gameYear + ")</span>";
        auto titleLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(titleLabel), titleText.c_str());
        gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
        gtk_widget_add_css_class(titleLabel, "card-title");
        gtk_box_append(GTK_BOX(cardBox), titleLabel);
        
        // Description with scores
        std::string descriptionWithScores = gameDescription;
        
        if (gameName == "Manic Miner") {
            descriptionWithScores += formatScoresDisplay(manicMinerScores, "points");
        } else if (gameName == "Jet Set Willy") {
            descriptionWithScores += formatScoresDisplay(jetSetWillyScores, "items");
        }
        
        auto descLabel = gtk_label_new(descriptionWithScores.c_str());
        gtk_label_set_wrap(GTK_LABEL(descLabel), TRUE);
        gtk_label_set_wrap_mode(GTK_LABEL(descLabel), PANGO_WRAP_WORD);
        gtk_widget_set_halign(descLabel, GTK_ALIGN_START);
        gtk_label_set_xalign(GTK_LABEL(descLabel), 0.0);
        gtk_widget_add_css_class(descLabel, "card-description");
        gtk_box_append(GTK_BOX(cardBox), descLabel);
        
        // Stats
        auto* stats = getGameStats(gameName);
        std::string statusText;
        if (stats && stats->playCount > 0) {
            statusText = "✅ Played " + std::to_string(stats->playCount) + " times • " + stats->formatPlayTime();
        } else {
            statusText = "✅ Ready to play";
        }
        
        auto statusLabel = gtk_label_new(statusText.c_str());
        gtk_widget_add_css_class(statusLabel, "success-text");
        gtk_widget_set_halign(statusLabel, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(cardBox), statusLabel);
        
        // Launch button
        auto launchButton = gtk_button_new_with_label("🚀 Play Game");
        gtk_widget_set_halign(launchButton, GTK_ALIGN_START);
        gtk_widget_add_css_class(launchButton, "suggested-action");
        g_object_set_data_full(G_OBJECT(launchButton), "game_type", g_strdup(gameType.c_str()), g_free);
        g_signal_connect(launchButton, "clicked", G_CALLBACK(onGameLaunch), this);
        gtk_box_append(GTK_BOX(cardBox), launchButton);
        
        return frame;
    }
    
    void setupCSS() {
        auto cssProvider = gtk_css_provider_new();
        const char* css = 
            ".success-text { color: #1b5e20; font-weight: bold; }"
            "window { background-color: #f8f9fa; }"
            "headerbar { background-color: #e3f2fd; color: #0d47a1; }"
            "frame { border: 1px solid #dee2e6; background-color: #ffffff; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }"
            ".card-title { color: #0d47a1; font-weight: bold; }"
            ".card-description { color: #212529; font-size: 14px; }"
            "label { color: #212529; }"
            "button { margin: 4px; padding: 8px 16px; }"
            "button.suggested-action { background-color: #1976d2; color: white; border-radius: 6px; }"
            "button.suggested-action:hover { background-color: #1565c0; }";
            
        gtk_css_provider_load_from_string(cssProvider, css);
        gtk_style_context_add_provider_for_display(
            gdk_display_get_default(),
            GTK_STYLE_PROVIDER(cssProvider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
        g_object_unref(cssProvider);
    }
    
    // Menu functions
    static void onAboutShow(GtkButton*, gpointer userData) {
        auto* launcher = static_cast<UnifiedGameLauncher*>(userData);
        
        auto aboutDialog = gtk_about_dialog_new();
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), "Matthew Smith Games - Unified");
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutDialog), "3.0");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutDialog), 
            "Unified Manic Miner & Jet Set Willy Launcher\n"
            "Classic ZX Spectrum games with automatic high score tracking.");
        gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutDialog), "Original games © Matthew Smith");
        
        const char* authors[] = {"Matthew Smith", "Enhanced Unified Launcher", NULL};
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(aboutDialog), authors);
        
        gtk_window_set_transient_for(GTK_WINDOW(aboutDialog), launcher->window);
        gtk_window_set_modal(GTK_WINDOW(aboutDialog), TRUE);
        gtk_window_present(GTK_WINDOW(aboutDialog));
        
        g_signal_connect(aboutDialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    }

public:
    UnifiedGameLauncher() : app(nullptr), window(nullptr), mainBox(nullptr), headerBar(nullptr) {
        const char* home = getenv("HOME");
        homeDir = home ? std::string(home) : ("/home/" + std::string(getenv("USER")));
        projectDir = homeDir + "/matthew-smith-games-enhanced";
        
        // Load game statistics and high scores
        loadGameStats();
        ensureScoresDirectory();
        loadScores("manic_miner_scores.txt", manicMinerScores);
        loadScores("jet_set_willy_scores.txt", jetSetWillyScores);
        
        // Note: checkForLastScore moved to createWindow after GTK is initialized
    }
    
    ~UnifiedGameLauncher() = default;
    
    static void activate(GtkApplication* app, gpointer userData) {
        auto* launcher = static_cast<UnifiedGameLauncher*>(userData);
        launcher->createWindow(app);
    }
    
    void createWindow(GtkApplication* gtkApp) {
        app = gtkApp;
        
        window = GTK_WINDOW(gtk_application_window_new(gtkApp));
        gtk_window_set_title(window, "Matthew Smith Games - Unified");
        gtk_window_set_default_size(window, 800, 600);
        gtk_window_set_resizable(window, TRUE);
        
        setupCSS();
        
        // Create header bar
        headerBar = GTK_HEADER_BAR(gtk_header_bar_new());
        gtk_header_bar_set_title_widget(headerBar, gtk_label_new("Matthew Smith Games - Unified"));
        gtk_window_set_titlebar(window, GTK_WIDGET(headerBar));
        
        // Add header buttons
        auto aboutBtn = gtk_button_new_with_label("ℹ️ About");
        gtk_header_bar_pack_end(headerBar, aboutBtn);
        g_signal_connect(aboutBtn, "clicked", G_CALLBACK(onAboutShow), this);
        
        // Create main container
        auto scrolled = gtk_scrolled_window_new();
        gtk_window_set_child(window, scrolled);
        
        mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), GTK_WIDGET(mainBox));
        
        // Add title
        auto titleLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(titleLabel), 
            "<span size='x-large' weight='bold'>Classic ZX Spectrum Games - Unified Edition</span>");
        gtk_widget_set_margin_top(titleLabel, 15);
        gtk_widget_set_margin_bottom(titleLabel, 5);
        gtk_box_append(mainBox, titleLabel);
        
        // Add subtitle
        auto subtitleLabel = gtk_label_new("Select a game to play with automatic high score tracking:");
        gtk_widget_set_margin_bottom(subtitleLabel, 15);
        gtk_box_append(mainBox, subtitleLabel);
        
        // Add game cards
        auto manicCard = createGameCard(
            "Manic Miner", "1983",
            "Navigate underground caverns as Miner Willy collecting keys while avoiding deadly creatures.",
            "manic"
        );
        gtk_box_append(mainBox, manicCard);
        
        auto jetsetCard = createGameCard(
            "Jet Set Willy", "1984", 
            "Explore Willy's mansion, collecting objects from 60 rooms in this classic platform adventure.",
            "jetset"
        );
        gtk_box_append(mainBox, jetsetCard);
        
        gtk_window_present(window);
        
        // Now that GTK is initialized, check for recent game scores
        checkForLastScore("manic");
        checkForLastScore("jetset");
    }
    
    int run(int argc, char** argv) {
        app = gtk_application_new("com.games.matthew-smith-unified", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), this);
        
        int status = g_application_run(G_APPLICATION(app), argc, argv);
        g_object_unref(app);
        
        return status;
    }
};

int main(int argc, char** argv) {
    try {
        UnifiedGameLauncher launcher;
        return launcher.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
