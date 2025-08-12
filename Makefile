# Matthew Smith Games - Enhanced Collection
# Master Build System

# Project information
PROJECT_NAME = matthew-smith-games-enhanced
VERSION = 1.0.0

# Directories
MANIC_DIR = ManicMiner
JETSET_DIR = JetSetWilly
GUI_LAUNCHER_DIR = GameLauncherGUI
CONSOLE_LAUNCHER_DIR = UnifiedGameLauncher

# Executables
MANIC_BINARY = $(MANIC_DIR)/manicminer
JETSET_BINARY = $(JETSET_DIR)/jetsetwilly
GUI_LAUNCHER_BINARY = $(GUI_LAUNCHER_DIR)/game-launcher-gui
CONSOLE_LAUNCHER_BINARY = $(CONSOLE_LAUNCHER_DIR)/unified-game-launcher

# Installation paths
INSTALL_DIR = ~/.local/bin
DESKTOP_DIR = ~/.local/share/applications

# Default target
all: all-games all-launchers

# Build all games
all-games: manic-miner jet-set-willy

# Build all launchers  
all-launchers: gui-launcher console-launcher

# Individual game targets
manic-miner:
	@echo "🎮 Building Manic Miner..."
	@cd $(MANIC_DIR) && $(MAKE)
	@echo "✅ Manic Miner built successfully!"

jet-set-willy:
	@echo "🎮 Building Jet Set Willy..."
	@cd $(JETSET_DIR) && $(MAKE)
	@echo "✅ Jet Set Willy built successfully!"

# Individual launcher targets
gui-launcher:
	@echo "🎨 Building GUI Launcher..."
	@cd $(GUI_LAUNCHER_DIR) && $(MAKE)
	@echo "✅ GUI Launcher built successfully!"

console-launcher:
	@echo "💻 Building Console Launcher..."
	@cd $(CONSOLE_LAUNCHER_DIR) && $(MAKE) -f simple.mk
	@echo "✅ Console Launcher built successfully!"

# Installation targets
install: install-launchers

install-launchers: all-launchers
	@echo "📦 Installing launchers to $(INSTALL_DIR)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(GUI_LAUNCHER_BINARY) $(INSTALL_DIR)/
	@cp $(CONSOLE_LAUNCHER_BINARY) $(INSTALL_DIR)/
	@echo "🖥️ Installing desktop entry..."
	@mkdir -p $(DESKTOP_DIR)
	@echo "[Desktop Entry]" > $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Name=Matthew Smith Games" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Comment=Enhanced Manic Miner and Jet Set Willy games" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Exec=$(INSTALL_DIR)/game-launcher-gui" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Icon=applications-games" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Terminal=false" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Type=Application" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Categories=Game;ActionGame;" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "Keywords=game;manic;miner;jetset;willy;retro;spectrum;" >> $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "✅ Installation complete!"

install-games: all-games
	@echo "📦 Installing games to $(INSTALL_DIR)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(MANIC_BINARY) $(INSTALL_DIR)/
	@cp $(JETSET_BINARY) $(INSTALL_DIR)/
	@echo "✅ Games installed!"

# Cleaning targets  
clean: clean-games clean-launchers

clean-all: clean

clean-games:
	@echo "🧹 Cleaning game builds..."
	@cd $(MANIC_DIR) && $(MAKE) clean
	@cd $(JETSET_DIR) && $(MAKE) clean
	@echo "✅ Game builds cleaned!"

clean-launchers:
	@echo "🧹 Cleaning launcher builds..."
	@cd $(GUI_LAUNCHER_DIR) && $(MAKE) clean
	@cd $(CONSOLE_LAUNCHER_DIR) && $(MAKE) clean
	@echo "✅ Launcher builds cleaned!"

# Uninstallation
uninstall:
	@echo "🗑️ Uninstalling launchers..."
	@rm -f $(INSTALL_DIR)/game-launcher-gui
	@rm -f $(INSTALL_DIR)/unified-game-launcher
	@rm -f $(INSTALL_DIR)/manicminer
	@rm -f $(INSTALL_DIR)/jetsetwilly
	@rm -f $(DESKTOP_DIR)/matthew-smith-games.desktop
	@echo "✅ Uninstallation complete!"

# Testing targets
test: test-builds test-dependencies

test-builds: all
	@echo "🧪 Testing builds..."
	@echo "Checking Manic Miner..."
	@test -x $(MANIC_BINARY) && echo "  ✅ Manic Miner executable OK" || echo "  ❌ Manic Miner not built"
	@echo "Checking Jet Set Willy..."
	@test -x $(JETSET_BINARY) && echo "  ✅ Jet Set Willy executable OK" || echo "  ❌ Jet Set Willy not built"
	@echo "Checking GUI Launcher..."
	@test -x $(GUI_LAUNCHER_BINARY) && echo "  ✅ GUI Launcher executable OK" || echo "  ❌ GUI Launcher not built"
	@echo "Checking Console Launcher..."
	@test -x $(CONSOLE_LAUNCHER_BINARY) && echo "  ✅ Console Launcher executable OK" || echo "  ❌ Console Launcher not built"

test-dependencies:
	@echo "🔍 Checking dependencies..."
	@echo "SDL2:"
	@pkg-config --exists sdl2 && echo "  ✅ SDL2 found" || echo "  ❌ SDL2 missing"
	@echo "GTK4:"
	@pkg-config --exists gtk4 && echo "  ✅ GTK4 found" || echo "  ❌ GTK4 missing"
	@echo "Compiler:"
	@which gcc > /dev/null && echo "  ✅ GCC found" || echo "  ❌ GCC missing"
	@which g++ > /dev/null && echo "  ✅ G++ found" || echo "  ❌ G++ missing"

# Quick launch targets (for development)
run-manic: manic-miner
	@echo "🚀 Launching Manic Miner..."
	@cd $(MANIC_DIR) && ./manicminer

run-jetset: jet-set-willy
	@echo "🚀 Launching Jet Set Willy..."
	@cd $(JETSET_DIR) && ./jetsetwilly

run-gui: gui-launcher
	@echo "🚀 Launching GUI Launcher..."
	@cd $(GUI_LAUNCHER_DIR) && ./game-launcher-gui

run-console: console-launcher
	@echo "🚀 Launching Console Launcher..."
	@cd $(CONSOLE_LAUNCHER_DIR) && ./unified-game-launcher

# Package creation (for distribution)
package: clean all
	@echo "📦 Creating distribution package..."
	@mkdir -p dist
	@tar -czf dist/$(PROJECT_NAME)-$(VERSION).tar.gz \
		--exclude='dist' \
		--exclude='*.o' \
		--exclude='*.d' \
		--exclude='.git*' \
		.
	@echo "✅ Package created: dist/$(PROJECT_NAME)-$(VERSION).tar.gz"

# Help target
help:
	@echo "🎮 Matthew Smith Games - Enhanced Collection"
	@echo "==========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all                 - Build everything (default)"
	@echo "  all-games          - Build both games"
	@echo "  all-launchers      - Build both launchers"
	@echo ""
	@echo "Individual builds:"
	@echo "  manic-miner        - Build Manic Miner"
	@echo "  jet-set-willy      - Build Jet Set Willy"
	@echo "  gui-launcher       - Build GTK4 GUI launcher"
	@echo "  console-launcher   - Build console launcher"
	@echo ""
	@echo "Installation:"
	@echo "  install            - Install launchers globally"
	@echo "  install-launchers  - Install launchers to ~/.local/bin"
	@echo "  install-games      - Install games to ~/.local/bin"
	@echo "  uninstall          - Remove all installed components"
	@echo ""
	@echo "Cleaning:"
	@echo "  clean              - Clean all builds"
	@echo "  clean-games        - Clean game builds"
	@echo "  clean-launchers    - Clean launcher builds"
	@echo ""
	@echo "Testing:"
	@echo "  test               - Test builds and dependencies"
	@echo "  test-builds        - Test that all executables exist"
	@echo "  test-dependencies  - Check system dependencies"
	@echo ""
	@echo "Quick Launch (Development):"
	@echo "  run-manic          - Build and run Manic Miner"
	@echo "  run-jetset         - Build and run Jet Set Willy"
	@echo "  run-gui            - Build and run GUI launcher"
	@echo "  run-console        - Build and run console launcher"
	@echo ""
	@echo "Distribution:"
	@echo "  package            - Create distribution package"
	@echo ""
	@echo "System Requirements:"
	@echo "  - SDL2 development libraries"
	@echo "  - GTK4 development libraries (for GUI launcher)"
	@echo "  - GCC/G++ compiler with C++17 support"

# Status target
status:
	@echo "📊 Project Status"
	@echo "================="
	@echo "Games:"
	@test -x $(MANIC_BINARY) && echo "  🎮 Manic Miner: ✅ Built" || echo "  🎮 Manic Miner: ❌ Not built"
	@test -x $(JETSET_BINARY) && echo "  🎮 Jet Set Willy: ✅ Built" || echo "  🎮 Jet Set Willy: ❌ Not built"
	@echo "Launchers:"
	@test -x $(GUI_LAUNCHER_BINARY) && echo "  🎨 GUI Launcher: ✅ Built" || echo "  🎨 GUI Launcher: ❌ Not built"
	@test -x $(CONSOLE_LAUNCHER_BINARY) && echo "  💻 Console Launcher: ✅ Built" || echo "  💻 Console Launcher: ❌ Not built"
	@echo "Installation:"
	@test -x $(INSTALL_DIR)/game-launcher-gui && echo "  🎨 GUI Launcher: ✅ Installed" || echo "  🎨 GUI Launcher: ❌ Not installed"
	@test -x $(INSTALL_DIR)/unified-game-launcher && echo "  💻 Console Launcher: ✅ Installed" || echo "  💻 Console Launcher: ❌ Not installed"

.PHONY: all all-games all-launchers manic-miner jet-set-willy gui-launcher console-launcher \
        install install-launchers install-games uninstall \
        clean clean-all clean-games clean-launchers \
        test test-builds test-dependencies \
        run-manic run-jetset run-gui run-console \
        package help status
