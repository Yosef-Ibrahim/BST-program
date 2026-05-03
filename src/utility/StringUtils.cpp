#include <utility/StringUtils.h>

namespace StringUtils {

    std::vector<std::string> Wrap(const std::string& txt, int maxPx, int fontSize) {
        std::vector<std::string> lines;
        std::istringstream words(txt);
        std::string word;
        std::string currentLine;

        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;

            int textWidth = MeasureText(testLine.c_str(), fontSize);

            if (textWidth > maxPx && !currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            } else {
                currentLine = testLine;
            }
        }

        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }

        return lines;
    }

}