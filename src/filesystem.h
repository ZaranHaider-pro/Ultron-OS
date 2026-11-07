#pragma once
#include "ultron.h"
#include "logger.h"

// =======================================================
//  FILE SYSTEM
//  Task 10: File creation & deletion
//  Task 11: File read / write operations
//  Task 12: Directory management
// =======================================================
class FileSystem {
private:
    // Directory tree: path → { name → FSFile }
    std::map<std::string, std::map<std::string, FSFile>> tree;
    std::string currentDir;
    Logger&     logger;

    // Normalise path (handles ".." and ".")
    std::string resolvePath(const std::string& path) const {
        if (path.empty() || path[0] == '/') return path.empty() ? "/" : path;
        // relative → absolute
        std::string base = (currentDir == "/") ? "" : currentDir;
        std::string full = base + "/" + path;
        // simplify
        std::vector<std::string> parts;
        std::stringstream ss(full);
        std::string tok;
        while (std::getline(ss, tok, '/')) {
            if (tok == ".." && !parts.empty()) parts.pop_back();
            else if (!tok.empty() && tok != ".") parts.push_back(tok);
        }
        std::string result;
        for (const auto& p : parts) result += "/" + p;
        return result.empty() ? "/" : result;
    }

    std::string parentOf(const std::string& path) const {
        size_t pos = path.rfind('/');
        if (pos == 0 || pos == std::string::npos) return "/";
        return path.substr(0, pos);
    }

    std::string nameOf(const std::string& path) const {
        size_t pos = path.rfind('/');
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }

public:
    FileSystem(Logger& log) : currentDir("/"), logger(log) {
        // Bootstrap root and some default directories
        tree["/"];
        mkdir("/bin",  "root");
        mkdir("/home", "root");
        mkdir("/tmp",  "root");
        mkdir("/home/zaran",    "zaran");
    }

    std::string pwd() const { return currentDir; }

    // ==================================================
    //  Task 12: mkdir
    // ==================================================
    bool mkdir(const std::string& rawPath, const std::string& owner) {
        std::string path = resolvePath(rawPath);
        std::string parent = parentOf(path);
        std::string dname  = nameOf(path);

        if (!tree.count(parent)) {
            logger.error("FS", "mkdir: parent '" + parent + "' does not exist.");
            return false;
        }
        if (tree.count(path) || tree[parent].count(dname)) {
            logger.warn("FS", "mkdir: '" + path + "' already exists.");
            return false;
        }
        tree[path];                                   // create empty dir entry
        FSFile dir(dname, owner, true);
        tree[parent][dname] = dir;
        logger.info("FS", "mkdir: created directory '" + path + "'");
        return true;
    }

    // ==================================================
    //  Task 12: cd
    // ==================================================
    bool cd(const std::string& rawPath) {
        std::string path = resolvePath(rawPath);
        if (!tree.count(path)) {
            logger.error("FS", "cd: directory '" + path + "' not found.");
            return false;
        }
        currentDir = path;
        return true;
    }

    // ==================================================
    //  Task 12: ls – list directory
    // ==================================================
    void ls(const std::string& rawPath = "") const {
        std::string path = rawPath.empty() ? currentDir : resolvePath(rawPath);
        if (!tree.count(path)) {
            std::cout << RED << "  ls: '" << path << "' not found.\n" << RESET;
            return;
        }
        const auto& entries = tree.at(path);
        std::cout << BOLD << "\n  Contents of " << path << ":\n" << RESET;
        if (entries.empty()) {
            std::cout << YELLOW << "  (empty)\n" << RESET;
            return;
        }
        for (const auto& [name, f] : entries) {
            if (f.isDirectory)
                std::cout << CYAN  << "  📁 " << name << "/\n" << RESET;
            else
                std::cout << WHITE << "  📄 " << name
                          << "  (" << f.content.size() << " bytes)"
                          << "  owner:" << f.owner << "\n" << RESET;
        }
    }

    // ==================================================
    //  Task 10: touch / create file
    // ==================================================
    bool touch(const std::string& rawPath, const std::string& owner) {
        std::string path   = resolvePath(rawPath);
        std::string parent = parentOf(path);
        std::string fname  = nameOf(path);

        if (!tree.count(parent)) {
            logger.error("FS", "touch: parent dir '" + parent + "' not found.");
            return false;
        }
        if (tree[parent].count(fname)) {
            // File exists – just update timestamp (no-op here)
            return true;
        }
        FSFile f(fname, owner, false);
        tree[parent][fname] = f;
        logger.info("FS", "Created file '" + path + "' (owner:" + owner + ")");
        return true;
    }

    // ==================================================
    //  Task 10: rm – delete file or directory
    // ==================================================
    bool rm(const std::string& rawPath, const std::string& caller) {
        std::string path   = resolvePath(rawPath);
        std::string parent = parentOf(path);
        std::string fname  = nameOf(path);

        if (!tree.count(parent) || !tree[parent].count(fname)) {
            logger.error("FS", "rm: '" + path + "' not found.");
            return false;
        }
        auto& f = tree[parent][fname];
        if (f.owner != caller && caller != "root") {
            logger.error("FS", "rm: permission denied for '" + caller + "' on '" + path + "'");
            return false;
        }
        if (f.isDirectory && tree.count(path) && !tree[path].empty()) {
            logger.warn("FS", "rm: directory '" + path + "' is not empty.");
            return false;
        }
        if (f.isDirectory) tree.erase(path);
        tree[parent].erase(fname);
        logger.info("FS", "Deleted '" + path + "' by " + caller);
        return true;
    }

    // ==================================================
    //  Task 11: write to file
    // ==================================================
    bool write(const std::string& rawPath, const std::string& content,
               const std::string& caller, bool append = false) {
        std::string path   = resolvePath(rawPath);
        std::string parent = parentOf(path);
        std::string fname  = nameOf(path);

        // Auto-create if not exists
        if (!tree.count(parent)) {
            logger.error("FS", "write: parent '" + parent + "' not found.");
            return false;
        }
        if (!tree[parent].count(fname)) {
            touch(rawPath, caller);
        }
        auto& f = tree[parent][fname];
        if (f.isDirectory) {
            logger.error("FS", "write: '" + path + "' is a directory.");
            return false;
        }
        if (append)
            f.content += content;
        else
            f.content = content;

        logger.info("FS", std::string(append ? "Appended" : "Wrote") +
                    " to '" + path + "' (" + std::to_string(f.content.size()) + " bytes)");
        return true;
    }

    // ==================================================
    //  Task 11: read file
    // ==================================================
    bool read(const std::string& rawPath) const {
        std::string path   = resolvePath(rawPath);
        std::string parent = parentOf(path);
        std::string fname  = nameOf(path);

        if (!tree.count(parent) || !tree.at(parent).count(fname)) {
            std::cout << RED << "  cat: '" << path << "' not found.\n" << RESET;
            return false;
        }
        const auto& f = tree.at(parent).at(fname);
        if (f.isDirectory) {
            std::cout << RED << "  cat: '" << path << "' is a directory.\n" << RESET;
            return false;
        }
        std::cout << CYAN << "  -- " << path << " ----------------------\n" << RESET;
        if (f.content.empty())
            std::cout << YELLOW << "  (empty file)\n" << RESET;
        else
            std::cout << "  " << f.content << "\n";
        return true;
    }

    // -- cp – copy file ---------------------------------
    bool cp(const std::string& srcRaw, const std::string& dstRaw, const std::string& caller) {
        std::string src = resolvePath(srcRaw);
        std::string srcParent = parentOf(src);
        std::string srcName   = nameOf(src);

        if (!tree.count(srcParent) || !tree[srcParent].count(srcName)) {
            logger.error("FS", "cp: source '" + src + "' not found.");
            return false;
        }
        const auto& f = tree[srcParent][srcName];
        write(dstRaw, f.content, caller);
        logger.info("FS", "Copied '" + src + "' → '" + resolvePath(dstRaw) + "'");
        return true;
    }

    // -- mv – move file ---------------------------------
    bool mv(const std::string& srcRaw, const std::string& dstRaw, const std::string& caller) {
        if (cp(srcRaw, dstRaw, caller)) {
            rm(srcRaw, caller);
            logger.info("FS", "Moved '" + resolvePath(srcRaw) +
                        "' → '" + resolvePath(dstRaw) + "'");
            return true;
        }
        return false;
    }
};
