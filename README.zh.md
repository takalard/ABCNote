# ABCNote

随记，根据日期随记。

本意是我自己开发给自己用的，现在开放出来，想改的都可以发挥你的想象力，进行修改。

# ABCNote

ABCNote 是一个 Qt 6 C++/QML 随身笔记应用，面向电脑端并为后续手机端适配预留空间。它按日期组织笔记，启动时自动打开今天的笔记，在右侧以连续滚动方式显示笔记，并把编辑内容自动保存为本地 JSON 文件。

## 前置依赖

- Qt 6.5 或更新版本，包含 Qt Quick、Qt Quick Controls 2 和 Qt Test。
- CMake 3.21 或更新版本。
- Qt 支持的 C++17 编译器。

如果本机需要安装 Qt 或相关开发工具，请安装到：

```text
D:\Program\dev
```

## 构建

本工作区已经包含适配本机 MSVC 和 Qt 工具链的辅助脚本：

```powershell
.\tools\configure-msvc-qt.cmd
.\tools\build-msvc-qt.cmd
.\tools\test-msvc-qt.cmd
```

运行应用：

```powershell
.\build-msvc-qt3\ABCNote.exe
```

这些脚本使用：

- CMake
- Ninja：通过 winget 安装
- Visual Studio Build Tools(2022)
- Qt(6.8.3)

脚本还会显式添加 Windows SDK 路径，因为当前 Build Tools 安装不会自动加入这些路径。

## VSCode 调试

仓库已经包含 `.vscode/` 配置。

使用方式：

- 按 `Ctrl+Shift+B`，使用 `ABCNote: Build (MSVC/Qt)` 构建。
- 按 `F5`，选择 `Debug ABCNote (MSVC/Qt)` 启动调试。
- 在 `Terminal > Run Task...` 中选择 `ABCNote: Test (MSVC/Qt)` 运行单元测试。
- 在 `Terminal > Run Task...` 中选择 `ABCNote: Run App` 直接运行应用。

调试配置会自动加入 Qt 运行路径，包括 `Qt6*.dll`、QML imports 和 platform plugins。

## 数据存储

只有非空笔记会保存到应用数据目录下的 JSON 文件中。编辑器里显示的空白日期只是内存临时页面，不会写入磁盘。

内部笔记结构如下：

```text
data/
  2026/
    05/
      2026-05-02.json
```

每篇笔记包含日期、正文、自动生成的总结和最后更新时间。

如果已经持久化的笔记被清空正文，它的 JSON 文件会被删除，该日期也会从左侧导航中消失。左侧导航只显示已经持久化且非空的笔记，年/月行支持展开和收缩。

## 当前 AI 总结行为

第一版使用本地总结生成器。它会从正文中提取前几句有意义的内容。后续可以替换为 OpenAI、本地模型或自定义 API 实现。
