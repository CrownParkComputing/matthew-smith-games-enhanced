#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

class GameLauncher {
private:
    std::string homeDir;
    std::string manicMinerPath;
    std::string jetSetWillyPath;
    
    bool checkGameExists(const std::string& path) {
        return fs::exists(path) && fs::is_regular_file(path);
    }
    
    void clearScreen() {
        std::system("clear");
    }
    
    void showBanner() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════╗\n";
        std::cout << "║              MATTHEW SMITH GAME LAUNCHER             ║\n";
        std::cout << "║                                                      ║\n";
        std::cout << "║       Classic ZX Spectrum Games - Enhanced          ║\n";
        std::cout << "╚══════════════════════════════════════════════════════╝\n";
        std::cout << "\n";
    }
    
    void showGameInfo() {
        std::cout << "Available Games:\n";
        std::cout << "================\n\n";
        
        // Check Manic Miner
        if (checkGameExists(manicMinerPath)) {
            std::cout << "✓ [1] Manic Miner (1983)\n";
            std::cout << "      Path: " << manicMinerPath << "\n";
            std::cout << "      The original platform adventure game\n";
        } else {
            std::cout << "✗ [1] Manic Miner (NOT FOUND)\n";
            std::cout << "      Expected: " << manicMinerPath << "\n";
        }
        
        std::cout << "\n";
        
        // Check Jet Set Willy
        if (checkGameExists(jetSetWillyPath)) {
            std::cout << "✓ [2] Jet Set Willy (1984)\n";
            std::cout << "      Path: " << jetSetWillyPath << "\n";
            std::cout << "      The sequel with 60 rooms to explore\n";
        } else {
            std::cout << "✗ [2] Jet Set Willy (NOT FOUND)\n";
            std::cout << "      Expected: " << jetSetWillyPath << "\n";
        }
        
        std::cout << "\n";
        std::cout << "[3] Show Controls\n";
        std::cout << "[4] About\n";
        std::cout << "[Q] Quit\n";
        std::cout << "\n";
    }
    
    void showControls() {
        clearScreen();
        showBanner();
        std::cout << "Game Controls:\n";
        std::cout << "==============\n\n";
        std::cout << "Movement:\n";
        std::cout << "  Arrow Keys  - Move left/right\n";
        std::cout << "  Space       - Jump\n";
        std::cout << "  Enter       - Confirm/Select\n";
        std::cout << "  Escape      - Return to title screen\n\n";
        
        std::cout << "Enhanced Features:\n";
        std::cout << "  F11         - Toggle Fullscreen/Windowed mode\n";
        std::cout << "  TAB/Pause   - Pause/Unpause game\n";
        std::cout << "  M           - Mute/Unmute audio\n\n";
        
        std::cout << "Window Features:\n";
        std::cout << "  • Resizable windows (drag corners)\n";
        std::cout << "  • Proper scaling with aspect ratio maintained\n";
        std::cout << "  • Centered display with black borders\n";
        std::cout << "  • Works on Wayland and X11\n\n";
        
        std::cout << "Additional (Jet Set Willy):\n";
        std::cout << "  Any Key     - Skip copy protection during loading\n\n";
        
        std::cout << "Press Enter to return to main menu...\n";
        std::cin.ignore();
        std::cin.get();
    }
    
    void showAbout() {
        clearScreen();
        showBanner();
        std::cout << "About These Games:\n";
        std::cout << "==================\n\n";
        
        std::cout << "Original Games by Matthew Smith:\n";
        std::cout << "• Manic Miner (1983) - Bug-Byte/Software Projects\n";
        std::cout << "• Jet Set Willy (1984) - Software Projects\n\n";
        
        std::cout << "These Enhanced C Ports Feature:\n";
        std::cout << "• SDL2-based modern compatibility\n";
        std::cout << "• Per-pixel coloring (no color clash)\n";
        std::cout << "• 16-color palette\n";
        std::cout << "• Polyphonic music reproduction\n";
        std::cout << "• Stereo sound effects with panning\n";
        std::cout << "• Square wave audio for authentic retro feel\n";
        std::cout << "• Bug fixes from original versions\n";
        std::cout << "• Enhanced windowing system\n\n";
        
        std::cout << "Port Information:\n";
        std::cout << "• Based on original ZX Spectrum versions\n";
        std::cout << "• Written in C using SDL2 library\n";
        std::cout << "• 100% identical gameplay to originals\n";
        std::cout << "• Enhanced for modern systems\n\n";
        
        std::cout << "Press Enter to return to main menu...\n";
        std::cin.ignore();
        std::cin.get();
    }
    
    void launchGame(const std::string& gamePath, const std::string& gameName) {
        if (!checkGameExists(gamePath)) {
            std::cout << "\n❌ Error: " << gameName << " executable not found!\n";
            std::cout << "Expected location: " << gamePath << "\n";
            std::cout << "Please build the game first using 'make' in the game directory.\n\n";
            std::cout << "Press Enter to continue...\n";
            std::cin.ignore();
            std::cin.get();
            return;
        }
        
        std::cout << "\n🚀 Launching " << gameName << "...\n";
        std::cout << "Game will start in a few seconds.\n";
        std::cout << "Remember: Press F11 to toggle fullscreen!\n\n";
        
        // Launch the game
        int result = std::system(gamePath.c_str());
        
        if (result != 0) {
            std::cout << "\n⚠️  Game exited with code: " << result << "\n";
        } else {
            std::cout << "\n✓ " << gameName << " finished successfully.\n";
        }
        
        std::cout << "Press Enter to return to launcher...\n";
        std::cin.ignore();
        std::cin.get();
    }
    
public:
    GameLauncher() {
        // Get home directory
        const char* home = getenv("HOME");
        if (home) {
            homeDir = std::string(home);
        } else {
            homeDir = "/home/" + std::string(getenv("USER"));
        }
        
        // Set game paths
        manicMinerPath = homeDir + "/ManicMiner/manicminer";
        jetSetWillyPath = homeDir + "/JetSetWilly/jetsetwilly";
    }
    
    void run() {
        char choice;
        
        while (true) {
            clearScreen();
            showBanner();
            showGameInfo();
            
            std::cout << "Enter your choice: ";
            std::cin >> choice;
            
            switch (choice) {
                case '1':
                    launchGame(manicMinerPath, "Manic Miner");
                    break;
                    
                case '2':
                    launchGame(jetSetWillyPath, "Jet Set Willy");
                    break;
                    
                case '3':
                    showControls();
                    break;
                    
                case '4':
                    showAbout();
                    break;
                    
                case 'q':
                case 'Q':
                    std::cout << "\nThanks for playing! Goodbye! 👋\n";
                    return;
                    
                default:
                    std::cout << "\n❌ Invalid choice! Please try again.\n";
                    std::cout << "Press Enter to continue...\n";
                    std::cin.ignore();
                    std::cin.get();
                    break;
            }
        }
    }
};

int main() {
    try {
        GameLauncher launcher;
        launcher.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
