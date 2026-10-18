# 与本仓库协作的约定

## 项目概述

qiconmaker 由 SVG 或 PNG 的源图生成 Windows 的 `.ico` 与 macOS 的 `.icns`，供 CMake 在构建时调用。使用者的仓库因此只保存源图，不提交生成的图标文件。

- **输入**：一张主稿（SVG 或 PNG），以及若干个可选的、指定了尺寸的源图，例如单独绘制、对齐像素网格的 16 与 32 像素小稿。
- **规则**：某个尺寸指定了源图，就使用该源图。SVG 按该尺寸渲染，PNG 必须恰好是该尺寸。未指定的尺寸由主稿生成：SVG 主稿按该尺寸渲染，矢量图不因缩放损失细节；PNG 主稿缩小到该尺寸，**不放大**，主稿小于所需尺寸即报错。
- **输出**：格式由输出文件的扩展名决定，`.ico` 或 `.icns`。每种格式有默认的尺寸集合，可以在命令行上替换。
- **依赖**：只依赖 Qt 6 的 Core、Gui 与 Svg，**不依赖 qmsetup**。渲染使用 QtSvg，与 Qt 程序显示 SVG 图标所用的渲染器相同。

## 目录结构

目录结构仿照 winacp（`stdware/winacp`）。

| 位置 | 内容 |
|---|---|
| `CMakeLists.txt` | 库、命令行程序、测试与安装规则 |
| `cmake/qiconmakerConfig.cmake.in` | 安装后的包配置 |
| `include/qiconmaker/` | 库的公开头文件 |
| `src/` | 库的源文件 |
| `tools/cli/main.cpp` | 命令行程序 `qiconmaker` |
| `tests/` | 测试，一个 `test_XXX.cpp` 对应一个 `XXX.h` |
| `docs/TODO.md` | 尚未实现的功能与测试覆盖的缺口 |

库是静态库，目标名为 `QIconMaker`；命令行程序的目标名为 `qiconmaker`。安装后的包导出两者，使用者以 `find_package(qiconmaker)` 取得 `qiconmaker::qiconmaker`，在 `add_custom_command` 中运行。

## 格式

**不要凭印象描述格式。** 以下是实现所依据的事实，修改编码代码时先核对，结论来自规范、参考实现或实测，并注明出处。

- **ICO**：目录头（保留字、类型 1、图像数）之后是每个图像 16 字节的目录项，再后是图像数据。宽高为 256 时目录项中写 0。256 像素的图像存为 PNG，其余存为 32 位 BGRA 位图，位图头中的高度是像素高度的两倍，包含其后的 AND 掩码，掩码每行按 32 位对齐。资源编译器与 Windows 资源管理器都接受这种组合。
- **ICNS**：文件头为 `icns` 与大端序的总长度，之后每个元素为 4 字节类型、大端序长度（含元素头的 8 字节）与数据。元素数据为 PNG 时，各类型的像素尺寸为：`icp4` 16、`icp5` 32、`ic11` 32（16 点的 2 倍）、`ic12` 64（32 点的 2 倍）、`ic07` 128、`ic13` 256（128 点的 2 倍）、`ic08` 256、`ic14` 512（256 点的 2 倍）、`ic09` 512、`ic10` 1024（512 点的 2 倍）。Apple 没有公开的规格，以 `iconutil` 能否把生成的文件还原为 iconset 为准，macOS 上的测试即这样检查。

## 技术栈

Qt 6 + CMake 3.22 及以上 + C++17。CMake 3.22 起才有测试的 `ENVIRONMENT_MODIFICATION`，Windows 上的测试由它把 Qt 的 `bin` 加入 `PATH`。

- **Qt 的头文件须带模块名**，写 `<QtGui/QImage>`，不写 `<QImage>`。
- 渲染需要 `QGuiApplication`，因为 SVG 中的文字依赖字体数据库。命令行程序在未设置 `QT_QPA_PLATFORM` 时使用 `offscreen` 平台，构建机上没有显示器也能运行。
- 测试使用 QtTest，以 `QT_QPA_PLATFORM=offscreen` 运行。

## 代码编写

- 文件名采用大驼峰，与其中的主要类型同名；入口 `main.cpp` 小写。
- 命名空间为 `qiconmaker`。类型采用大驼峰，函数、参数、变量采用小驼峰，枚举成员采用大驼峰。私有数据成员使用 `m_` 前缀。
- 头文件中引用本项目的头文件时使用尖括号与完整路径，写 `<qiconmaker/IcoFormat.h>`；源文件中引用同名头文件使用双引号。头文件保护按 include 路径命名，如 `QICONMAKER_ICOFORMAT_H`。
- 可能不存在结果的函数返回 `std::optional<T>`，不要使用「bool 加输出参数」，也不要用某个特定值表示「不存在」。
- 初始化表达式为指针时写 `auto name = ...`，不写 `auto *name = ...`。命名空间结束处不添加注释。
- 修改后只对自己改动过的文件运行 `clang-format`。

文体（注释、文档、README、帮助文本、诊断消息通用）：

- **采用正式的技术写作文体。** 每句陈述一项事实、约束或理由，不写铺垫、感想和修辞。
- **不拟人。** 代码、文件、格式、程序和测试不作为有意志的主语：不写 says、tells、knows、asks、wants、means、decides，也不写「它说」「它知道」「它想要」。改用 returns、indicates、records、specifies、reports、requires、rejects，或「返回」「表示」「记录」「规定」「报告」「拒绝」。
- **用术语，不用描述性转述。** 没有通用术语时，首次出现给出定义，之后始终沿用同一名称。
- **标题和列表标签用名词或名词短语。** 主语从句不可用，宾语从句少用。条件用 if，不用 where。`one` 回指只用于同一个句子。
- **句子完整。** 不写片段句、逗号粘连句和反问句，不以 So、Hence 开头叙述因果，不对读者使用第二人称。
- **函数说明以动词开头**（Returns、Encodes、Renders、Rejects），`\return` 写明每种情况的返回值。
- 中文文本同样适用：用书面语，不用「别」「搞」「就行」「得（表必须）」「其实」等口语词。

注释：

- LLVM 风格，`///` 写在声明上方，不使用 `\brief`。类的 private 成员与 `.cpp` 中的实现细节使用 `//`。
- Doxygen 命令使用 `\c` `\a` `\note` `<tt>`，不使用反引号或引号。
- 注释中不使用破折号，也不用分号连接从句。使用美式拼写。C++ 注释按 100 列折行。
- 注释说明约束与「为何只能如此实现」，不复述签名已经表明的内容。不写考古式注释，但记录曾经的实现错误的注释必须保留。

Markdown 一段即一行，不手动折行，文件末尾不留多余空行。

## 构建与验证

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH=<qt> -DQICONMAKER_BUILD_TESTS=ON
cmake --build build
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

在 Windows 上须先加载 Visual Studio 的编译环境（`vcvars64.bat`）。

- **确认测试通过之前，先确认构建的退出码为 0。** 构建失败时 ctest 运行的是上一次构建的旧程序。
- 新增测试后确认断言确实被执行，空的测试集同样会「通过」。
- **完成一项行为的测试后，将缺陷放回，确认测试会失败。** 修改一行、重新构建、运行、还原。
- 生成的图标须由读取方验证，而不只是检查自己写出的字节：ICO 以 Qt 的 ICO 读取插件读回，ICNS 在 macOS 上以 `iconutil` 还原。

## 提交

- **提交信息不带任何 AI 署名**，不写 `Co-Authored-By`，不写 `Generated with`。
- **只写一行**，使用英文、首字母大写的祈使句和美式拼写，不写正文。设计理由写在代码注释与文档中。
- **一个提交只做一件事。**
- **未经授权不要 commit，更不要 push。**
- 用户可见的改动写入 `README.md` 或 `docs/TODO.md` 的对应条目。

## 判断与沟通

- **断言之前先验证。** 结论必须来自编译器、运行时探针或参考实现，不要凭记忆断言。
- **探针本身也可能出错。** 结果异常时先怀疑探针。
- 用户的质疑通常是正确的，应先重新验证，而不是辩护。
- 不确定时明确说明不确定，不要以推测填补。

## 已知问题

- **`sed -i` 会将 CRLF 转换为 LF。** 修改前用 `grep -qU $'\r'` 判断，CRLF 文件改用编辑工具修改。
- **绝不使用 bash heredoc 编写脚本**，也不要经由 shell 传递含反斜杠的文本，应先写入文件再执行。
