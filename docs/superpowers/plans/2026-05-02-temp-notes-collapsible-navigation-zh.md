# 临时笔记与可折叠导航实施计划

> **给智能工程执行者：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans` 按任务执行本计划。步骤使用复选框语法，便于跟踪。

**目标：** 只持久化非空笔记，并让左侧年/月导航支持展开和收缩。

**架构：** 存储层按正文内容决定保存或删除，不再为空白缺失笔记创建文件。右侧笔记流可以保存内存临时笔记。导航从已持久化日期重建，并保留年/月展开状态。

**技术栈：** Qt 6、C++17、QML、Qt Test。

---

## 任务

### 任务 1：存储语义

**文件：**
- 修改：`src/NoteStorage.h`
- 修改：`src/NoteStorage.cpp`
- 修改：`tests/tst_abcnote.cpp`

- [ ] 增加非创建式加载和空笔记删除测试。
- [ ] 实现 `load(const QDate&)`、`exists(const QDate&)`、`remove(const QDate&)` 和按内容处理的 `save(Note)`。

### 任务 2：笔记模型与控制器

**文件：**
- 修改：`src/NoteModel.cpp`
- 修改：`src/NoteController.cpp`
- 修改：`tests/tst_abcnote.cpp`

- [ ] 增加启动和滚动不会创建空文件的测试。
- [ ] 增加非空编辑会持久化、清空笔记会删除文件的测试。
- [ ] 更新模型加载逻辑，让缺失日期成为临时笔记。
- [ ] 更新控制器 flush，让非空笔记保存，空笔记删除。

### 任务 3：可折叠导航

**文件：**
- 修改：`src/NavigationModel.h`
- 修改：`src/NavigationModel.cpp`
- 修改：`Layout/Main.qml`
- 修改：`tests/tst_abcnote.cpp`

- [ ] 增加年/月展开收缩测试。
- [ ] 增加 `expanded` 和 `expandable` 角色。
- [ ] 增加 `toggleExpanded(row)` 并保留展开状态。
- [ ] 更新 QML：年/月行点击切换展开，日行点击跳转。

### 任务 4：验证

**文件：**
- 修改：`README.md`
- 修改：`README.zh.md`

- [ ] 更新中英文行为说明。
- [ ] 运行 `tools\build-msvc-qt.cmd`。
- [ ] 运行 `tools\test-msvc-qt.cmd`。

