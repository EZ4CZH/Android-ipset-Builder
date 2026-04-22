# Android ipset Static Builder  by AI.

这是一个由聪明的AI打造的用于为 Android (aarch64) 设备全自动编译 **纯静态版 ipset** 的开源项目。
完美解决支持IPset的安卓系统上提示‘缺少二进制’导致无法生效的问题。

每次构建都会**自动从官网获取最新版的 “ipset” 源码**进行编译，并**自动拉取最新的 IP 路由表 (“chnroute.txt”)** 一并打包。

## 🚀 获取成品

如果你不想折腾编译，可以直接使用 GitHub Actions 为你自动打包好的成品：
1. 进入本仓库的 **Actions** 页面。
2. 选择左侧的 `构建并发布 Android ipset`，点击右上角的 `Run workflow` 手动触发。
3. 等待约 30 秒钟，下载页面下方的 **`ipset-android-aarch64-latest`** 压缩包即可。

## 💻 本地克隆与一键编译

如果你有 Ubuntu 虚拟机、WSL 或者手机上装了 Termux，你可以直接克隆本项目在本地全自动编译。
