# ABCNote Qt 实施计划

> **给智能工程执行者：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行本计划。步骤使用复选框语法，便于跟踪。

**目标：** 构建一个 Qt 6 C++/QML ABCNote 工程，启动后打开今天的笔记，按年/月/日 JSON 文件保存笔记，支持连续日期滚动，并自动保存编辑内容。

**架构：** C++ 负责存储、模型、控制器和总结生成。QML 负责布局和编辑界面。`NoteModel` 与 `NavigationModel` 向 QML 暴露日期数据，`NoteController` 协调启动、加载、编辑、总结刷新和保存。

**技术栈：** Qt 6、CMake、C++17、QML、Qt Quick Controls、Qt Test。

---

## 文件结构

- 新建 `CMakeLists.txt`：根 CMake 工程，包含应用目标和测试目标。
- 新建 `src/main.cpp`：应用启动和 QML 上下文注入。
- 新建 `src/Note.h`：笔记值对象。
- 新建 `src/NoteStorage.h/.cpp`：JSON 持久化和日期路径映射。
- 新建 `src/SummaryService.h/.cpp`：本地总结生成器。
- 新建 `src/NoteModel.h/.cpp`：右侧连续笔记流 QML 列表模型。
- 新建 `src/NavigationModel.h/.cpp`：左侧年/月/日导航 QML 列表模型。
- 新建 `src/NoteController.h/.cpp`：应用协调和自动保存。
- 修改 `Layout/Main.qml`：Qt 6 界面、左侧导航、连续列表、状态栏。
- 修改 `Layout/NoteItemDelegate.qml`：绑定模型角色的可编辑笔记页。
- 新建 `tests/tst_abcnote.cpp`：用 Qt Test 覆盖存储、总结、模型和控制器行为。
- 新建 `README.md` 和 `README.zh.md`：中英双语构建与使用说明。

## 任务

### 任务 1：创建构建骨架和测试

**文件：**
- 新建：`CMakeLists.txt`
- 新建：`tests/tst_abcnote.cpp`

- [ ] 添加 Qt 6 CMake 工程，包含应用目标和测试目标。
- [ ] 添加初始测试。此时存储、总结和控制器类还不存在，测试应先失败。
- [ ] 运行 `cmake -S . -B build`；在依赖和源文件未齐全时，预期配置失败。

### 任务 2：实现存储和总结

**文件：**
- 新建：`src/Note.h`
- 新建：`src/NoteStorage.h`
- 新建：`src/NoteStorage.cpp`
- 新建：`src/SummaryService.h`
- 新建：`src/SummaryService.cpp`
- 修改：`tests/tst_abcnote.cpp`

- [ ] 测试日期到路径的映射会生成 `data/YYYY/MM/YYYY-MM-DD.json`。
- [ ] 测试缺失笔记会创建为空正文且日期匹配的笔记。
- [ ] 测试保存后的笔记可以从 JSON 重新加载。
- [ ] 测试总结对长文本返回简短本地预览，对空文本返回默认提示。
- [ ] 编写通过这些测试所需的最小实现。

### 任务 3：实现模型

**文件：**
- 新建：`src/NoteModel.h`
- 新建：`src/NoteModel.cpp`
- 新建：`src/NavigationModel.h`
- 新建：`src/NavigationModel.cpp`
- 修改：`tests/tst_abcnote.cpp`

- [ ] 测试 `NoteModel::initializeAroundDate()` 会加载前一天、当天和后一天。
- [ ] 测试 `loadPreviousDays()` 会把更早日期插入开头。
- [ ] 测试 `loadNextDays()` 会把更晚日期追加到末尾。
- [ ] 测试 `NavigationModel` 暴露年、月、日三类行，层级分别为 `0`、`1`、`2`。
- [ ] 实现 QML 使用的稳定角色名：`noteId`、`date`、`dateString`、`contentBody`、`aiSummary`、`level`、`display`。

### 任务 4：实现控制器和自动保存

**文件：**
- 新建：`src/NoteController.h`
- 新建：`src/NoteController.cpp`
- 修改：`tests/tst_abcnote.cpp`

- [ ] 测试启动时会创建今天的笔记。
- [ ] 测试 `updateNoteBody()` 会更新模型数据并标记为待保存。
- [ ] 测试 `flush()` 会写入待保存笔记。
- [ ] 测试刷新总结会更新模型和保存后的 JSON。
- [ ] 使用 `QTimer` 实现延迟自动保存，并提供显式 `flush()`。

### 任务 5：连接 QML 界面

**文件：**
- 新建：`src/main.cpp`
- 修改：`Layout/Main.qml`
- 修改：`Layout/NoteItemDelegate.qml`

- [ ] 把模型和控制器注册为 QML 上下文属性。
- [ ] 启动后定位到今天的笔记。
- [ ] 实现左侧导航点击跳转日期。
- [ ] 实现顶部和底部的无限加载触发。
- [ ] 将委托编辑器绑定到 `contentBody`，并调用 `noteController.updateNoteBody(noteId, text)`。

### 任务 6：文档和验证

**文件：**
- 新建：`README.md`
- 新建：`README.zh.md`

- [ ] 记录前置依赖，包括需要时把 Qt 6 安装到 `D:\Program\dev`。
- [ ] 记录配置、构建、测试、运行命令。
- [ ] 运行新鲜验证命令：
  - `cmake -S . -B build`
  - `cmake --build build`
  - `ctest --test-dir build --output-on-failure`
- [ ] 如果本机缺少 Qt/CMake，记录准确阻塞点和预期命令。

