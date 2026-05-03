#include <ui/ThemeManager.h>
#include <fstream>

ThemeManager::ThemeManager() {
    loadFromFile("assets/themes/theme.json");
}

const Theme& ThemeManager::getCurrentTheme() const {
    return currentTheme;
}

bool ThemeManager::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string s;
    std::string currentKey = "";
    std::vector<unsigned char> rgba;

    while (file >> s) {
        // Strip away all JSON punctuation
        std::string token = "";
        for (char c : s) {
            // Keep only letters and numbers
            if (std::isalnum(c)) {
                token += c;
            }
        }

        // Skip if the token was just a comma or bracket
        if (token.empty()) continue;

        // If the token starts with a letter, it's a key
        if (std::isalpha(token[0])) {
            currentKey = token;
            rgba.clear(); // Reset the color array for the new key
        } 
        // If it starts with a number, it's part of the color array
        else if (std::isdigit(token[0])) {
            rgba.push_back(static_cast<unsigned char>(std::stoi(token)));
            
            if (rgba.size() == 4) {
                Color c = { rgba[0], rgba[1], rgba[2], rgba[3] };
                
                if (currentKey == "bg") currentTheme.bg = c;
                else if (currentKey == "panel") currentTheme.panel = c;
                else if (currentKey == "accent") currentTheme.accent = c;
                else if (currentKey == "success") currentTheme.success = c;
                else if (currentKey == "error") currentTheme.error = c;
                else if (currentKey == "textMain") currentTheme.textMain = c;
                else if (currentKey == "textMuted") currentTheme.textMuted = c;
                else if (currentKey == "edgeCol") currentTheme.edgeCol = c;

                currentKey = "";
            }
        }
    }
    
    saveToFile();
    return true;
}

void ThemeManager::saveToFile() {
    const std::string filepath = "assets/themes/theme.json";
    std::ofstream file(filepath);
    if (!file.is_open()) return;

    auto writeColor = [&file](const std::string& key, Color c, bool isLast) {
        file << "    \"" << key << "\": [" 
             << (int)c.r << ", " 
             << (int)c.g << ", " 
             << (int)c.b << ", " 
             << (int)c.a << "]";
             
        if (!isLast) file << ",";
        file << "\n";
    };

    file << "{\n";
    writeColor("bg", currentTheme.bg, false);
    writeColor("panel", currentTheme.panel, false);
    writeColor("accent", currentTheme.accent, false);
    writeColor("success", currentTheme.success, false);
    writeColor("error", currentTheme.error, false);
    writeColor("textMain", currentTheme.textMain, false);
    writeColor("textMuted", currentTheme.textMuted, false);
    writeColor("edgeCol", currentTheme.edgeCol, true);
    file << "}\n";
}