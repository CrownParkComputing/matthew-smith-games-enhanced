#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>

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
    std::string scoresDir;
    
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
            }
            file.close();
            
            // Remove the temporary score file
            std::remove(scoreFile.c_str());
        }
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
        
        // Keep only top 5
        if (scores.size() > 5) {
            scores.resize(5);
        }
    }
    
    std::string formatScoresDisplay(const std::vector<ScoreEntry>& scores, const std::string& unit = "points") {
        if (scores.empty()) {
            return "No scores recorded yet";
        }
        
        std::string display = "\n🏆 Top Scores:\n";
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
    
    static void onScoreDialogCancel(GtkButton*, gpointer user_data) {
        gtk_window_destroy(GTK_WINDOW(user_data));
    }
    
    static void onScoreDialogSave(GtkButton* button, gpointer) {
        auto nameEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "nameEntry"));
        auto scoreEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "scoreEntry"));
        auto dialog = GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog"));
        auto gameName = (const char*)g_object_get_data(G_OBJECT(button), "gameName");
        auto scoreType = (const char*)g_object_get_data(G_OBJECT(button), "scoreType");
        auto launcher = static_cast<GameLauncherGUI*>(g_object_get_data(G_OBJECT(button), "launcher"));
        
        auto nameBuffer = gtk_entry_get_buffer(nameEntry);
        auto scoreBuffer = gtk_entry_get_buffer(scoreEntry);
        
        std::string name = gtk_entry_buffer_get_text(nameBuffer);
        std::string scoreText = gtk_entry_buffer_get_text(scoreBuffer);
        
        if (!name.empty() && !scoreText.empty()) {
            try {
                int scoreValue = std::stoi(scoreText);
                
                if (std::string(gameName) == "Manic Miner") {
                    launcher->addScore(launcher->manicMinerScores, name, scoreValue);
                    launcher->saveScores("manic_miner_scores.txt", launcher->manicMinerScores);
                } else if (std::string(gameName) == "Jet Set Willy") {
                    launcher->addScore(launcher->jetSetWillyScores, name, scoreValue);
                    launcher->saveScores("jet_set_willy_scores.txt", launcher->jetSetWillyScores);
                }
                
                // Show confirmation
                std::string unit = (std::string(scoreType) == "Score") ? "points" : "items";
                launcher->showInfoDialog("Score Saved", 
                    "Score saved successfully!\n\n" + name + ": " + 
                    std::to_string(scoreValue) + " " + unit);
                
            } catch (const std::exception& e) {
                launcher->showInfoDialog("Invalid Score", 
                    "Invalid score value: " + scoreText);
            }
        }
        
        gtk_window_destroy(dialog);
    }
    
    void showScoreEntryDialog(const std::string& gameName, const std::string& scoreType) {
        // Create a simple window for score entry
        auto dialog = gtk_window_new();
        gtk_window_set_title(GTK_WINDOW(dialog), ("New Score - " + gameName).c_str());
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 250);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        
        auto vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        gtk_widget_set_margin_start(vbox, 20);
        gtk_widget_set_margin_end(vbox, 20);
        gtk_widget_set_margin_top(vbox, 20);
        gtk_widget_set_margin_bottom(vbox, 20);
        gtk_window_set_child(GTK_WINDOW(dialog), vbox);
        
        auto grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
        gtk_box_append(GTK_BOX(vbox), grid);
        
        // Info label
        auto infoLabel = gtk_label_new(("Enter your " + scoreType + " for " + gameName + ":").c_str());
        gtk_widget_set_halign(infoLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), infoLabel, 0, 0, 2, 1);
        
        // Name entry
        auto nameLabel = gtk_label_new("Player Name:");
        gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
        auto nameEntry = gtk_entry_new();
        auto nameBuffer = gtk_entry_buffer_new("Player", -1);
        gtk_entry_set_buffer(GTK_ENTRY(nameEntry), nameBuffer);
        
        // Score entry
        auto scoreLabel = gtk_label_new((scoreType + ":").c_str());
        gtk_widget_set_halign(scoreLabel, GTK_ALIGN_START);
        auto scoreEntry = gtk_entry_new();
        auto scoreBuffer = gtk_entry_buffer_new("0", -1);
        gtk_entry_set_buffer(GTK_ENTRY(scoreEntry), scoreBuffer);
        
        gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), nameEntry, 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), scoreLabel, 0, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), scoreEntry, 1, 2, 1, 1);
        
        // Button box
        auto buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_widget_set_halign(buttonBox, GTK_ALIGN_END);
        gtk_box_append(GTK_BOX(vbox), buttonBox);
        
        auto cancelButton = gtk_button_new_with_label("Cancel");
        auto saveButton = gtk_button_new_with_label("Save Score");
        gtk_widget_add_css_class(saveButton, "suggested-action");
        
        gtk_box_append(GTK_BOX(buttonBox), cancelButton);
        gtk_box_append(GTK_BOX(buttonBox), saveButton);
        
        // Store entry widgets as data on the buttons
        g_object_set_data(G_OBJECT(saveButton), "nameEntry", nameEntry);
        g_object_set_data(G_OBJECT(saveButton), "scoreEntry", scoreEntry);
        g_object_set_data(G_OBJECT(saveButton), "dialog", dialog);
        g_object_set_data_full(G_OBJECT(saveButton), "gameName", g_strdup(gameName.c_str()), g_free);
        g_object_set_data_full(G_OBJECT(saveButton), "scoreType", g_strdup(scoreType.c_str()), g_free);
        g_object_set_data(G_OBJECT(saveButton), "launcher", this);
        
        g_signal_connect(cancelButton, "clicked", G_CALLBACK(onScoreDialogCancel), dialog);
        g_signal_connect(saveButton, "clicked", G_CALLBACK(onScoreDialogSave), nullptr);
        
        gtk_window_present(GTK_WINDOW(dialog));
    }
    
    static void onInfoDialogOk(GtkButton*, gpointer user_data) {
        gtk_window_destroy(GTK_WINDOW(user_data));
    }
    
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
        
        g_signal_connect(okButton, "clicked", G_CALLBACK(onInfoDialogOk), dialog);
        
        gtk_window_present(GTK_WINDOW(dialog));
    }
    
    void loadGameStats() {
        statsFilePath = homeDir + "/.config/matthew-smith-games-stats";
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
        // Create config directory if it doesn't exist
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
        std::chrono::system_clock::time_point startTime;
        
        GameLaunchData(GameLauncherGUI* l, const std::string& name, GtkWidget* dialog) 
            : launcher(l), gameName(name), statusDialog(dialog), startTime(std::chrono::system_clock::now()) {}
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
        
        // Restore the main launcher window
        data->launcher->logMemoryUsage("Restoring launcher window");
        gtk_widget_set_visible(GTK_WIDGET(data->launcher->window), TRUE);
        gtk_window_present(data->launcher->window);
        gtk_window_unminimize(data->launcher->window);
        
        // Process events to ensure window restoration completes
        while (g_main_context_pending(NULL)) {
            g_main_context_iteration(NULL, FALSE);
        }
        
        // Small delay to ensure window is fully visible before showing result
        g_usleep(200000); // 200ms
        
        // Update game statistics if successful
        if (success && !error) {
            int exitStatus = g_subprocess_get_exit_status(G_SUBPROCESS(source));
            if (exitStatus == 0) {
                auto endTime = std::chrono::system_clock::now();
                auto playDuration = std::chrono::duration_cast<std::chrono::seconds>(endTime - data->startTime);
                
                auto* stats = data->launcher->getGameStats(data->gameName);
                if (stats) {
                    stats->playCount++;
                    stats->totalPlayTime += playDuration;
                    stats->lastPlayed = endTime;
                    data->launcher->saveGameStats();
                }
                
                // Check for automatically saved scores from the games
                data->launcher->checkForLastScore(data->gameName == "Manic Miner" ? "manic" : "jetset");
            }
        }
        
        // Show completion dialog with stats
        std::string message;
        GtkMessageType msgType;
        
        if (success && !error) {
            int exitStatus = g_subprocess_get_exit_status(G_SUBPROCESS(source));
            if (exitStatus == 0) {
                auto endTime = std::chrono::system_clock::now();
                auto sessionTime = std::chrono::duration_cast<std::chrono::minutes>(endTime - data->startTime);
                auto* stats = data->launcher->getGameStats(data->gameName);
                
                message = "Game finished successfully!\n";
                if (stats && sessionTime.count() > 0) {
                    message += "Session: " + std::to_string(sessionTime.count()) + " minutes\n";
                    message += "Total plays: " + std::to_string(stats->playCount) + "\n";
                    message += "Total time: " + stats->formatPlayTime();
                }
                message += "\n\nScores automatically saved!";
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
            "%s\n\n%s",
            data->gameName.c_str(),
            message.c_str()
        );
        
        gtk_window_present(GTK_WINDOW(resultDialog));
        g_signal_connect(resultDialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
        
        data->launcher->logMemoryUsage("Game Launch Complete");
        delete data;
    }
    
    static void onRecordScore(GtkButton* button, gpointer) {
        auto* data = static_cast<GameLaunchData*>(g_object_get_data(G_OBJECT(button), "data"));
        auto* dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
        
        if (data->gameName == "Manic Miner") {
            data->launcher->showScoreEntryDialog(data->gameName, "Score");
        } else if (data->gameName == "Jet Set Willy") {
            data->launcher->showScoreEntryDialog(data->gameName, "Items Collected");
        }
        
        gtk_window_destroy(GTK_WINDOW(dialog));
        delete data;
    }
    
    static void onSkipScore(GtkButton* button, gpointer) {
        auto* data = static_cast<GameLaunchData*>(g_object_get_data(G_OBJECT(button), "data"));
        auto* dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
        
        gtk_window_destroy(GTK_WINDOW(dialog));
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
        logMemoryUsage("Before window preparation");
        
        // If fullscreen, exit fullscreen first to ensure game gets focus
        if (isFullscreen) {
            gtk_window_unfullscreen(window);
            isFullscreen = false;
            // Update fullscreen button label
            GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(headerBar));
            while (child) {
                if (GTK_IS_BUTTON(child)) {
                    const char* label = gtk_button_get_label(GTK_BUTTON(child));
                    if (label && g_str_has_prefix(label, "🪟")) {
                        gtk_button_set_label(GTK_BUTTON(child), "⛶ Fullscreen");
                        break;
                    }
                }
                child = gtk_widget_get_next_sibling(child);
            }
        }
        
        // Minimize and hide the launcher window during game execution
        logMemoryUsage("Minimizing launcher window");
        gtk_window_minimize(window);
        gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
        
        // Process pending events to ensure window state changes take effect
        while (g_main_context_pending(NULL)) {
            g_main_context_iteration(NULL, FALSE);
        }
        
        // Delay to ensure window hide/minimize completes
        g_usleep(300000); // 300ms
        
        // Create launch status dialog (non-modal) - brief notification
        auto statusDialog = gtk_message_dialog_new(
            nullptr, // No parent since main window is hidden
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_NONE,
            "Launching %s...\nLauncher will return when game closes.",
            gameName.c_str()
        );
        
        gtk_window_present(GTK_WINDOW(statusDialog));
        
        // Auto-close the status dialog after 2 seconds
        g_timeout_add(2000, [](gpointer data) -> gboolean {
            GtkWidget* dialog = GTK_WIDGET(data);
            if (GTK_IS_WIDGET(dialog)) {
                gtk_window_destroy(GTK_WINDOW(dialog));
            }
            return FALSE; // Don't repeat
        }, statusDialog);
        
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
    
    static void onWindowFocus(GtkEventControllerFocus*, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        // Check for any new scores when the launcher window gains focus
        launcher->checkForLastScore("manic");
        launcher->checkForLastScore("jetset");
    }
    
    static void onStatsShow(GtkButton*, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->logMemoryUsage("Statistics Dialog Start");
        
        auto statsWindow = gtk_window_new();
        gtk_window_set_title(GTK_WINDOW(statsWindow), "Game Statistics");
        gtk_window_set_default_size(GTK_WINDOW(statsWindow), 500, 400);
        gtk_window_set_transient_for(GTK_WINDOW(statsWindow), launcher->window);
        gtk_window_set_modal(GTK_WINDOW(statsWindow), TRUE);
        
        auto textView = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_WORD);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textView), 20);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textView), 20);
        gtk_text_view_set_top_margin(GTK_TEXT_VIEW(textView), 15);
        gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(textView), 15);
        
        auto buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
        
        // Build statistics text
        std::string statsText = "GAME STATISTICS\n===============\n\n";
        
        for (const auto& stat : launcher->gameStats) {
            statsText += stat.gameName + ":\n";
            statsText += "  Times Played: " + std::to_string(stat.playCount) + "\n";
            if (stat.playCount > 0) {
                statsText += "  Total Time: " + stat.formatPlayTime() + "\n";
                auto avgMinutes = stat.totalPlayTime.count() / (stat.playCount * 60);
                statsText += "  Average Session: " + std::to_string(avgMinutes) + " minutes\n";
            } else {
                statsText += "  Total Time: Never played\n";
                statsText += "  Average Session: N/A\n";
            }
            statsText += "\n";
        }
        
        statsText += "Statistics saved to:\n" + launcher->statsFilePath;
        
        gtk_text_buffer_set_text(buffer, statsText.c_str(), -1);
        gtk_window_set_child(GTK_WINDOW(statsWindow), textView);
        
        gtk_window_present(GTK_WINDOW(statsWindow));
        
        launcher->logMemoryUsage("Statistics Dialog End");
    }
    
    static void onResetScoresConfirm(GtkButton* button, gpointer) {
        auto* launcher = static_cast<GameLauncherGUI*>(g_object_get_data(G_OBJECT(button), "launcher"));
        auto* dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
        
        // Clear all high scores
        launcher->manicMinerScores.clear();
        launcher->jetSetWillyScores.clear();
        
        // Save empty score files
        launcher->saveScores("manic_miner_scores.txt", launcher->manicMinerScores);
        launcher->saveScores("jet_set_willy_scores.txt", launcher->jetSetWillyScores);
        
        // Close dialog
        gtk_window_destroy(GTK_WINDOW(dialog));
        
        // Show confirmation
        launcher->showInfoDialog("Scores Reset", "All high scores have been cleared!");
    }
    
    static void onResetScoresCancel(GtkButton*, gpointer user_data) {
        gtk_window_destroy(GTK_WINDOW(user_data));
    }
    
    static void onResetScoresShow(GtkButton*, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        
        auto dialog = gtk_window_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "Reset High Scores");
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 200);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), launcher->window);
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        
        auto vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        gtk_widget_set_margin_start(vbox, 20);
        gtk_widget_set_margin_end(vbox, 20);
        gtk_widget_set_margin_top(vbox, 20);
        gtk_widget_set_margin_bottom(vbox, 20);
        gtk_window_set_child(GTK_WINDOW(dialog), vbox);
        
        auto label = gtk_label_new("⚠️ Are you sure you want to reset all high scores?\n\nThis will permanently delete all recorded scores for both games and cannot be undone.");
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(vbox), label);
        
        auto buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(vbox), buttonBox);
        
        auto cancelButton = gtk_button_new_with_label("Cancel");
        auto resetButton = gtk_button_new_with_label("Reset All Scores");
        gtk_widget_add_css_class(resetButton, "destructive-action");
        
        gtk_box_append(GTK_BOX(buttonBox), cancelButton);
        gtk_box_append(GTK_BOX(buttonBox), resetButton);
        
        // Store data for callbacks
        g_object_set_data(G_OBJECT(resetButton), "dialog", dialog);
        g_object_set_data(G_OBJECT(resetButton), "launcher", launcher);
        
        g_signal_connect(cancelButton, "clicked", G_CALLBACK(onResetScoresCancel), dialog);
        g_signal_connect(resetButton, "clicked", G_CALLBACK(onResetScoresConfirm), resetButton);
        
        gtk_window_present(GTK_WINDOW(dialog));
    }
    
    static void onAboutShow(GtkButton*, gpointer userData) {
        auto* launcher = static_cast<GameLauncherGUI*>(userData);
        launcher->logMemoryUsage("About Dialog");
        
        auto aboutDialog = gtk_about_dialog_new();
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), "Matthew Smith Games");
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutDialog), "2.1 (Enhanced)");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutDialog), 
            "Enhanced Manic Miner & Jet Set Willy\n"
            "Classic ZX Spectrum games with async launching and play statistics.");
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
        std::string descriptionWithScores = gameDescription;
        
        // Add high scores display
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
        gtk_box_append(GTK_BOX(infoBox), descLabel);
        
        // Status label with statistics
        std::string statusText;
        if (gameExists) {
            auto* stats = getGameStats(gameName);
            if (stats && stats->playCount > 0) {
                statusText = "✅ Ready • Played " + std::to_string(stats->playCount) + " times • " + stats->formatPlayTime();
            } else {
                statusText = "✅ Ready (Never played)";
            }
        } else {
            statusText = "❌ Not Built";
        }
        
        auto statusLabel = gtk_label_new(statusText.c_str());
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
        // Improved CSS with better contrast and readability
        const char* css = 
            ".placeholder-image { font-size: 20px; background-color: #f5f5f5; border: 1px dashed #bbb; color: #666; }"
            ".success-text { color: #1b5e20; font-weight: bold; }"
            ".error-text { color: #b71c1c; font-weight: bold; }"
            "window { background-color: #f8f9fa; }"
            "headerbar { background-color: #e3f2fd; color: #0d47a1; }"
            "frame { border: 1px solid #dee2e6; background-color: #ffffff; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }"
            ".card-title { color: #0d47a1; font-weight: bold; }"
            ".card-description { color: #212529; font-size: 14px; }"
            "label { color: #212529; }"
            "button { margin: 4px; padding: 8px 16px; }"
            "button.suggested-action { background-color: #1976d2; color: white; border-radius: 6px; }"
            "button.suggested-action:hover { background-color: #1565c0; }"
            "button.suggested-action:disabled { background-color: #bdbdbd; color: #757575; }"
            "button.destructive-action { background-color: #d32f2f; color: white; border-radius: 6px; }"
            "button.destructive-action:hover { background-color: #b71c1c; }";
            
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
        
        // Load game statistics and high scores
        loadGameStats();
        ensureScoresDirectory();
        loadScores("manic_miner_scores.txt", manicMinerScores);
        loadScores("jet_set_willy_scores.txt", jetSetWillyScores);
        
        // Check for recent game scores
        checkForLastScore("manic");
        checkForLastScore("jetset");
        
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
        
        // Add focus event handler to check for new scores
        auto focusController = gtk_event_controller_focus_new();
        g_signal_connect(focusController, "enter", G_CALLBACK(onWindowFocus), this);
        gtk_widget_add_controller(GTK_WIDGET(window), focusController);
        
        setupCSS();
        
        // Create header bar
        headerBar = GTK_HEADER_BAR(gtk_header_bar_new());
        gtk_header_bar_set_title_widget(headerBar, gtk_label_new("Matthew Smith Games (Async)"));
        gtk_window_set_titlebar(window, GTK_WIDGET(headerBar));
        
        // Add header buttons
        auto controlsBtn = gtk_button_new_with_label("🎮 Controls");
        auto statsBtn = gtk_button_new_with_label("📊 Stats");
        auto resetBtn = gtk_button_new_with_label("🗑️ Reset Scores");
        auto aboutBtn = gtk_button_new_with_label("ℹ️ About");
        auto fullscreenBtn = gtk_button_new_with_label("⛶ Fullscreen");
        
        gtk_header_bar_pack_end(headerBar, aboutBtn);
        gtk_header_bar_pack_end(headerBar, resetBtn);
        gtk_header_bar_pack_end(headerBar, statsBtn);
        gtk_header_bar_pack_end(headerBar, controlsBtn);
        gtk_header_bar_pack_start(headerBar, fullscreenBtn);
        
        g_signal_connect(controlsBtn, "clicked", G_CALLBACK(onControlsShow), this);
        g_signal_connect(statsBtn, "clicked", G_CALLBACK(onStatsShow), this);
        g_signal_connect(resetBtn, "clicked", G_CALLBACK(onResetScoresShow), this);
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
const std::string GameLauncherGUI::IMAGE_DIR_SUFFIX = "/matthew-smith-games-enhanced/assets/images";
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
