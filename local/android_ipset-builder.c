#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

void execute(const char *cmd) {
    printf(YELLOW ">> 执行: %s" RESET "\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, RED "错误: 指令执行失败!" RESET "\n");
        exit(1);
    }
}

// 去除字符串末尾的空白字符和特定符号
void trim_trailing_dots(char *str) {
    int len = strlen(str);
    while (len > 0 && (isspace(str[len - 1]) || str[len - 1] == '.')) {
        str[len - 1] = '\0';
        len--;
    }
}

int main() {
    char latest_ver[64] = {0};
    char cmd[1024];

    printf(GREEN "==== Android ipset C-Builder ====" RESET "\n");
    // 1. 初始化基础环境
    execute("mkdir -p workspace/env workspace/toolchain output");

    // 2. 准备交叉编译工具链 (aarch64-linux-musl)
    if (access("workspace/toolchain/aarch64-linux-musl-cross", F_OK) != 0) {
        execute("cd workspace/toolchain && "
                "wget https://github.com/troglobit/misc/releases/download/11-20211120/aarch64-linux-musl-cross.tgz && "
                "tar -xf aarch64-linux-musl-cross.tgz");
    }

    char *old_path = getenv("PATH");
    char new_path[2048];
    char cwd[512];
    getcwd(cwd, sizeof(cwd));
    sprintf(new_path, "PATH=%s/workspace/toolchain/aarch64-linux-musl-cross/bin:%s", cwd, old_path);
    putenv(new_path);

    // 3. 智能探测 ipset 官网最新版本
    printf(YELLOW ">> 正在探测最新版本..." RESET "\n");
    FILE *fp = popen("curl -s https://www.netfilter.org/pub/ipset/ | grep -o 'ipset-[0-9.]*\\.tar\\.bz2' | sort -V | tail -1 | sed 's/ipset-//;s/\\.tar\\.bz2//'", "r");
    
    if (fp) {
        if (fgets(latest_ver, sizeof(latest_ver), fp)) {
            latest_ver[strcspn(latest_ver, "\r\n")] = 0;
            int len = strlen(latest_ver);
            while (len > 0 && (latest_ver[len - 1] == '.' || latest_ver[len - 1] == ' ')) {
                latest_ver[len - 1] = '\0';
                len--;
            }
        }
        pclose(fp);
    }

    if (strlen(latest_ver) == 0) {
        fprintf(stderr, RED "无法获取版本号！" RESET "\n");
        return 1;
    }
    printf(GREEN ">> 成功探测到版本: v%s" RESET "\n", latest_ver);

    // 4. 编译依赖 libmnl
    execute("cd workspace && [ ! -d libmnl-1.0.5 ] && "
            "wget https://netfilter.org/projects/libmnl/files/libmnl-1.0.5.tar.bz2 && tar -xjf libmnl-1.0.5.tar.bz2");
    execute("cd workspace/libmnl-1.0.5 && ./configure --host=aarch64-linux-musl --prefix=$(pwd)/../env --enable-static --disable-shared && make -j$(nproc) && make install");

    // 5. 编译 ipset
    sprintf(cmd, "cd workspace && [ ! -f ipset-%s.tar.bz2 ] && "
                 "wget https://www.netfilter.org/pub/ipset/ipset-%s.tar.bz2 && tar -xjf ipset-%s.tar.bz2", 
                 latest_ver, latest_ver, latest_ver);
    execute(cmd);

    sprintf(cmd, "cd workspace/ipset-%s && "
                 "export PKG_CONFIG_PATH=$(pwd)/../env/lib/pkgconfig && "
                 "export CFLAGS=\"-I$(pwd)/../env/include\" && "
                 "export LDFLAGS=\"-L$(pwd)/../env/lib -static\" && "
                 "./configure --host=aarch64-linux-musl --prefix=$(pwd)/../env --disable-shared --with-kmod=no && "
                 "make LDFLAGS=\"-all-static\" -j$(nproc) && make install", latest_ver);
    execute(cmd);

    // 6. 整理产物并打包
    printf(GREEN ">> 正在打包最终产物..." RESET "\n");
    execute("cp workspace/env/sbin/ipset output/");
    execute("curl -sL https://raw.githubusercontent.com/misakaio/chnroutes2/master/chnroutes.txt -o output/chnroute.txt");
    sprintf(cmd, "zip -j output/ipset-android-v%s.zip output/ipset output/chnroute.txt", latest_ver);
    execute(cmd);

    printf(GREEN "==== 构建成功: output/ipset-android-v%s.zip ====" RESET "\n", latest_ver);
    return 0;
}
