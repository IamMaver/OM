#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstring>
#include "om.hpp"

using namespace std;

string GetDirName(int numberDir) {
    string result;
    while (true) {
        cout << "Please input name of directory #" << numberDir << endl;
        cin >> result;
        if (std::filesystem::exists(result) && std::filesystem::is_directory(result)) {
            break;
        } else {
            cout << "Directory does not exist or is not a directory. Please try again." << endl;
        }
    }
    return result;
}

class BinaryFile {
private:
    std::filesystem::directory_entry _directory_entry;
    unsigned int _hash;

    void setHash() {
        std::ifstream in(_directory_entry.path().string(), ios::binary);
        _hash = 0;
        char tmp;
        while (in.read(&tmp, 1)) {
            _hash += static_cast<unsigned char>(tmp);
        }
        if (in.gcount() > 0) {
            _hash += static_cast<unsigned char>(tmp);
        }
        in.close();
    }

public:
    BinaryFile(const std::filesystem::directory_entry &de) {
        _directory_entry = de;
        setHash();
    }

    string getInfo() const {
        return _directory_entry.path().string();
    }

    int getHash() const {
        return _hash;
    }

    bool CheckEq(const BinaryFile &other) const {
        std::ifstream in, in_other;
        char tmp = 0, tmp_other = 0;
        bool result = true;
        in.open(this->_directory_entry.path().string(), ios::binary);
        in_other.open(other._directory_entry.path().string(), ios::binary);
        in.seekg(0, in.end);
        in_other.seekg(0, in_other.end);
        if (in.tellg() != in_other.tellg()) {
            result = false;
        } else {
            in.seekg(0, in.beg);
            in_other.seekg(0, in_other.beg);
            while (!in.eof() && !in_other.eof()) {
                in.read(&tmp, 1);
                in_other.read(&tmp_other, 1);
                if (tmp != tmp_other) {
                    result = false;
                    break;
                }
            }
        }
        in.close();
        in_other.close();
        return result;
    }

    bool operator==(const BinaryFile &other) const {
        return this->CheckEq(other);
    }
};

class DuplicateFinder {
private:
    std::unordered_map<int, std::vector<BinaryFile>> fileMap;

    void
    checkForDuplicates(const std::vector<BinaryFile> &files, std::vector<std::vector<BinaryFile>> &duplicates) const {
        std::vector<bool> visited(files.size(), false);

        for (size_t i = 0; i < files.size(); ++i) {
            if (visited[i]) continue;

            std::vector<BinaryFile> currentGroup;
            currentGroup.push_back(files[i]);
            visited[i] = true;

            for (size_t j = i + 1; j < files.size(); ++j) {
                if (!visited[j] && files[i] == files[j]) {
                    currentGroup.push_back(files[j]);
                    visited[j] = true;
                }
            }

            if (currentGroup.size() > 1) {
                duplicates.push_back(currentGroup);
            }
        }
    }

public:
    void addFilesFromDirectory(const std::string &path) {
        for (auto &p: std::filesystem::directory_iterator(path)) {
            if (p.is_regular_file()) {
                BinaryFile bf(p);
                fileMap[bf.getHash()].push_back(bf);
            }
        }
    }

    std::vector<std::vector<BinaryFile>> findDuplicates() const {
        std::vector<std::vector<BinaryFile>> duplicates;
        for (const auto &entry: fileMap) {
            if (entry.second.size() > 1) {
                const auto &files = entry.second;
                checkForDuplicates(files, duplicates);
            }
        }
        return duplicates;
    }
};

int main() {
    DuplicateFinder finder;

    for (int i = 0; i < 2; i++) {
        std::string path = GetDirName(i + 1);
        finder.addFilesFromDirectory(path);
    }

    std::vector<std::vector<BinaryFile>> duplicates = finder.findDuplicates();

    for (const auto &dupGroup: duplicates) {
        if (dupGroup.size() > 1) {
            std::cout << "These files are exact duplicates:\n";
            for (const auto &bf: dupGroup) {
                std::cout << bf.getInfo() << endl;
            }
            std::cout << endl;
        }
    }

    return 0;
}