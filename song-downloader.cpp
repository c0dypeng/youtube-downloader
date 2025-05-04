#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>     // For readlink
#include <limits.h>     // For PATH_MAX
#include <libgen.h>     // For dirname
#include <cstring>      // For strncpy
#include <mach-o/dyld.h>  // <-- Add this for _NSGetExecutablePath


int main() {
    std::string url;
    std::cout << "Enter the YouTube URL: ";
    std::getline(std::cin, url);

    // Step 1: Get path of the executable
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string exePath;

#ifdef __APPLE__
    // macOS doesn't support /proc/self/exe, so use _NSGetExecutablePath
    uint32_t size = sizeof(result);
    if (_NSGetExecutablePath(result, &size) == 0) {
        char* dir = dirname(result);
        exePath = std::string(dir);
    } else {
        std::cerr << "Failed to get executable path." << std::endl;
        return 1;
    }
#else
    if (count != -1) {
        result[count] = '\0';
        exePath = std::string(dirname(result));
    } else {
        std::cerr << "Failed to get executable path." << std::endl;
        return 1;
    }
#endif

    // Step 2: Check if yt-dlp is installed
    int check = std::system("yt-dlp --version > /dev/null 2>&1");
    if (check != 0) {
        std::cerr << "yt-dlp is not installed. Please install it first." << std::endl;
        return 1;
    }

    // Step 3: Run yt-dlp with output to the executable's directory
    std::string command = "yt-dlp -x --audio-format wav -P \"" + exePath + "\" \"" + url + "\"";
    int resultCode = std::system(command.c_str());

    if (resultCode != 0) {
        std::cerr << "Failed to download audio." << std::endl;
        return 1;
    }

    std::cout << "Download completed successfully to: " << exePath << std::endl;
    return 0;
}
