#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

// 颜色配置
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

const std::string VERSION = "0.45";

// 彩色输出辅助函数
void printError(const std::string& message) {
    std::cerr << BOLD << RED << "[✗ ERROR] " << RESET << message << "\n";
}

void printSuccess(const std::string& message) {
    std::cout << BOLD << GREEN << "[✓] " << RESET << message << "\n";
}

void printWarning(const std::string& message) {
    std::cout << BOLD << YELLOW << "[!] " << RESET << message << "\n";
}

void printInfo(const std::string& message) {
    std::cout << BOLD << BLUE << "[i] " << RESET << message << "\n";
}

bool checkFileSize(const std::string& file, size_t max_size = 5 * 1024 * 1024) {
    struct stat file_stat;
    if (stat(file.c_str(), &file_stat)) {
        printError("无法获取文件状态: " + file);
        return false;
    }
    return file_stat.st_size <= max_size;
}

bool isTextFile(const std::string& file) {
    std::ifstream inFile(file);
    if (!inFile) {
        printError("无法打开文件: " + file);
        return false;
    }
    
    char ch;
    while (inFile.get(ch)) {
        if (ch == '\0') {
            printWarning("检测到二进制文件特征: " + file);
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string model = "deepseek-coder:6.7b";
    std::string message, file;
    bool has_input = false;

    // 参数解析
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-r") {
            model = "deepseek-r1:7b";
            printInfo("已切换至 R1 模型");
        }
        else if (arg == "-m") {
            if (++i >= argc) {
                printError("选项 -m 需要参数");
                return EXIT_FAILURE;
            }
            message = argv[i];
            has_input = true;
        }
        else if (arg == "-f") {
            if (++i >= argc) {
                printError("选项 -f 需要参数");
                return EXIT_FAILURE;
            }
            file = argv[i];
            has_input = true;
        }
        else if (arg == "-c") {
            printWarning("选项 -c 已废弃，默认使用 Coder 模型");
            return EXIT_FAILURE;
        }
        else if (arg == "-v") {
            std::cout << BOLD << MAGENTA << "AI 版本: " 
                      << GREEN << VERSION << RESET << "\n";
            return EXIT_SUCCESS;
        }
        else if (arg == "-h") {
            std::cout << BOLD << "使用方法: " << CYAN << "ai [选项]\n" << RESET
                      << BOLD << GREEN << "  -r    " << RESET << "使用 R1 模型 (默认: Coder)\n"
                      << BOLD << GREEN << "  -m   " << RESET << YELLOW << "<消息> " << RESET << "设置提示信息\n"
                      << BOLD << GREEN << "  -f   " << RESET << YELLOW << "<文件> " << RESET << "附加文件内容\n"
                      << BOLD << GREEN << "  -v    " << RESET << "显示版本信息\n"
                      << BOLD << GREEN << "  -h    " << RESET << "显示帮助信息\n";
            return EXIT_SUCCESS;
        }
        else {
            printError("无效选项: " + arg);
            return EXIT_FAILURE;
        }
    }

    // 直接运行模式
    if (!has_input) {
        printInfo("进入交互模式，使用 " + model + " 模型");
        int status = system(("ollama run " + model).c_str());
        if (WEXITSTATUS(status) != 0) {
            printError("命令执行失败 (代码: " + std::to_string(WEXITSTATUS(status)) + ")");
        }
        return status;
    }

    // 输入处理模式
    std::ostringstream prompt;
    if (!message.empty()) {
        prompt << "用户消息: \"" << message << "\"\n";
        printSuccess("已设置用户提示");
    }
    
    if (!file.empty()) {
        printInfo("正在处理文件: " + file);
        if (!checkFileSize(file)) {
            printError("无法打开文件或文件大小超过限制: " + file);
            return EXIT_FAILURE;
        }
        if (!isTextFile(file)) {
            printError("无效的文本文件: " + file);
            return EXIT_FAILURE;
        }

        std::ifstream inFile(file);
        if (!inFile) {
            printError("无法打开文件: " + file);
            return EXIT_FAILURE;
        }
        prompt << "文件内容 [" << file << "]:\n" << inFile.rdbuf();
        printSuccess("文件内容已加载: " + file);
    }

    // 创建临时文件
    char temp_file[] = "/tmp/ds_temp_XXXXXX";
    int fd = mkstemp(temp_file);
    if (fd == -1) {
        printError("无法创建临时文件");
        return EXIT_FAILURE;
    }
    
    std::string content = prompt.str();
    if (write(fd, content.c_str(), content.size()) == -1) {
        printError("写入临时文件失败");
        close(fd);
        return EXIT_FAILURE;
    }
    close(fd);

    // 执行命令
    std::cout << BOLD << CYAN << "\n[🤖 DeepSeek]" << RESET << "\n";
    int status = system(("ollama run " + model + " < " + temp_file).c_str());
    std::remove(temp_file);
    
    if (WEXITSTATUS(status) != 0) {
        printError("模型运行失败 (代码: " + std::to_string(WEXITSTATUS(status)) + ")");
        return EXIT_FAILURE;
    }

    std::cout << BOLD << GREEN << "\n[✓ 处理完成]" << RESET << "\n";
    return EXIT_SUCCESS;
}