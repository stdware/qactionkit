# 动作扩展清单规范

动作扩展清单声明一个组件向应用程序提供的动作、菜单与工具栏及其布局。动作扩展编译器（`qak_aec`）将清单转换为存放静态数据的 C++ 源文件，运行时不解析清单。

本文档描述清单格式的 `1.0` 版。

## 处理流程

```
manifest.xml ──qak_aec──> qak_manifest.cpp ──> ActionExtension ──> ActionRegistry
```

一份清单对应一个 `ActionExtension`。应用程序为每个组件登记一个扩展，`ActionRegistry` 将全部扩展合并为一个目录（catalog）与一个布局图。

## 根元素

```xml
<?xml version="1.0" encoding="UTF-8"?>
<actionExtension>
    <version>1.0</version>
    <id>com.example.application</id>

    <configuration> ... </configuration>
    <items> ... </items>
    <layouts> ... </layouts>
    <insertions> ... </insertions>
</actionExtension>
```

根元素必须是不属于任何命名空间的 `actionExtension`。

| 元素 | 必需 | 含义 |
| --- | --- | --- |
| `version` | 是 | 清单格式的版本。高于编译器所支持的版本时编译失败。 |
| `id` | 是 | 扩展的标识。登记两个标识相同的扩展时输出警告，保留先登记的一个。 |
| `configuration` | 否 | 清单其余部分的默认值。至多一个。 |
| `items` | 否 | 条目声明。 |
| `layouts` | 否 | 本扩展所拥有的菜单的组成。 |
| `insertions` | 否 | 向其他扩展所拥有的菜单插入的条目。 |

只识别**不属于任何命名空间**的元素，属于命名空间的元素一律忽略。清单借此为其他工具携带数据。

## 配置

```xml
<configuration>
    <defaultCatalog>core.catalog.others</defaultCatalog>

    <translationContext
        text="Application::ActionText"
        category="Application::ActionCategory"
        description="Application::ActionDescription"
    />

    <vars>
        <var key="APP_NAME" value="Application" />
    </vars>
</configuration>
```

`defaultCatalog` 指定目录节点，未指明目录的条目都归入该节点。若没有条目声明该节点，编译器自动为它创建一个 `phony` 条目。

`translationContext` 指定三个可翻译字段的 Qt 翻译上下文，未指定的字段分别使用内置的上下文 `QActionKit::ActionText`、`QActionKit::ActionCategory`、`QActionKit::ActionDescription`。条目可以用 `textTr`、`categoryTr`、`descriptionTr` 属性分别覆盖。这三个属性不出现在 `ActionItemInfo::attributes()` 中。

`vars` 声明供 `${...}` 展开的变量。命令行上以 `-D` 定义的变量优先于清单中的定义，构建过程因此可以在不修改清单的情况下为其指定参数。

### 保留变量

以下变量总有定义，`<var>` 不能覆盖：

| 名称 | 值 |
| --- | --- |
| `_ID_` | 扩展的标识 |
| `_VERSION_` | 清单的版本 |
| `_FILENAME_` | 清单的文件名 |
| `_FILEBASENAME_` | 清单去掉后缀的文件名 |

### 变量展开

每个属性值以及 `defaultCatalog` 的文本都经过展开：

- `${NAME}` 展开为 `NAME` 的值，`NAME` 未定义时展开为空字符串。
- 展开可以嵌套，例如 `${${WHICH}_NAME}`。
- `$$` 表示字面的 `$`。
- 含有不配对的 `${` 时，整个值展开为空字符串。

## 条件元素

`items`、`layouts`、`insertions` 中的任何元素都可以带 `if` 属性。属性值是**变量名**，而不是表达式：

```xml
<action id="core.debugDump" if="ENABLE_DEBUG_ACTIONS" />
```

变量为假时，跳过该元素及其子元素。空值、未定义以及 `false`、`no`、`0`、`n`、`off`（不区分大小写）为假，其他值均为真。因此在命令行上写 `-DENABLE_DEBUG_ACTIONS` 即可启用该元素。

## 条目

```xml
<items>
    <action id="core.openFile" text="Open File" category="File" shortcut="Ctrl+O" />
    <menu id="core.mainMenu" topLevel="true" />
    <toolBar id="core.mainToolBar" />
    <phony id="core.catalog.plugins" />
</items>
```

标签决定条目的类型：

| 标签 | 类型 | 说明 |
| --- | --- | --- |
| `action` | Action | 用户可以触发的叶节点 |
| `group` | Group | 具名的条目列表，其子项直接放入父节点 |
| `menu` | Menu | 菜单，`topLevel` 表示弹出菜单或菜单栏 |
| `menuBar`、`toolBar` | Menu | 与 `menu` 相同，但总是顶层 |
| `phony` | Phony | 只作为目录节点，不出现在任何视图中 |

条目声明不得有子元素。嵌套关系在 `layouts` 中描述。

### 属性

| 属性 | 适用于 | 默认值 |
| --- | --- | --- |
| `id` | 全部 | 必需 |
| `text` | 全部 | 由标识推导，见下文 |
| `category` | action | 空 |
| `description` | 全部 | 空 |
| `icon` | 全部 | 条目的标识 |
| `shortcut`、`shortcuts` | action | 无 |
| `catalog` | 全部 | 所在的布局条目，其次为 `defaultCatalog` |
| `topLevel` | group、menu | `false` |
| `textTr`、`categoryTr`、`descriptionTr` | 全部 | 取自 `configuration` |

`shortcuts` 为以 `;` 分隔的列表，优先于 `shortcut`。`\` 转义其后的一个字符，字面的分号写作 `\;`。

`category` 是命令面板显示在文本之前的类别标签，例如「File: Open」中的 `File`。它与 `catalog` 无关，后者决定条目在设置页层级中的位置。

`topLevel` 只有在展开后恰为字符串 `true` 时才为真。`menuBar` 与 `toolBar` 总是顶层。

其他属性原样保留，应用程序通过 `ActionItemInfo::attributes()` 读取，键为属性名**与命名空间 URI**。属于命名空间的属性即使本地名与保留属性相同也会保留：`x:text` 是自定义属性，`text` 则不是。

### 条目标识

标识由以 `.` 连接的若干段组成。各段由 ASCII 字母、数字和 `_` 组成。另有两个字符可以使用，且**只能出现在最后一段**：

- `&` 标记其后字符为助记符。该字符从标识中去除，保留在推导出的文本中。
- 位于标识末尾的 `^` 表示该动作打开一个对话框。该字符从标识中去除，在推导出的文本末尾写作 `...`。

`core.&openFile^` 的标识为 `core.openFile`，推导出的文本为 `&Open File...`。`&` 出现在前面的段中、连写的 `&&`、空段、非 ASCII 字符或其他标点均导致编译失败。

### 推导文本

未指定 `text` 时，由标识的最后一段推导：该段在每个大写字母之前断开，各部分转为小写，再将首字母大写，但位于首尾之外的短功能词（`a`、`the`、`of`、`to`、`with` 等）除外。例如 `core.openRecentFile` 推导为 `Open Recent File`，`core.tableOfContents` 推导为 `Table of Contents`。

## 布局

```xml
<layouts>
    <menu id="core.mainMenu">
        <menu id="core.file">
            <group id="core.fileOpenActions">
                <action id="core.openFile" />
                <action id="core.saveFile" />
            </group>
            <separator />
            <action id="core.exit" />
        </menu>
    </menu>
</layouts>
```

`layouts` 描述本扩展所拥有的菜单的组成。出现在布局中而未在 `items` 中声明的标识视为隐式声明，其属性取自布局元素，因此简短的清单可以省略 `items`。

以下两个标签没有标识：

- `separator` 表示分隔符。两种后端都会去除开头、结尾和连续的分隔符；`group` 在两侧各带一个分隔符，因此组与相邻条目之间总有分隔。
- `stretch` 表示可伸展的空白。只对工具栏有效，菜单中忽略。

编译器检查以下规则：

- 标签须与声明的类型相符。action 接受 `action` 与 `item`，group 与 menu 接受 `group`、`menu`、`menuBar`、`toolBar` 与 `item`。
- `phony` 条目不得出现在布局中。
- 容器的子项只能指定一次。在两处为 `core.file` 指定子项是错误。
- 布局不得递归。路径中再次出现已经包含的标识是错误。
- `separator` 与 `stretch` 不得有子元素。

条目在布局中的类型由标签决定：只有 `menu` 产生菜单条目，其他容器标签一律产生组条目。

首次出现在布局中的条目以所在元素的标识作为其目录。设置页面通常据此展示该条目。

## 插入

扩展以插入的方式向其他扩展所拥有的菜单添加条目：

```xml
<insertions>
    <insertion target="core.help" anchor="after" relativeTo="core.documentations">
        <action id="plugin.showHello" />
        <separator />
    </insertion>
</insertions>
```

| 属性 | 必需 | 含义 |
| --- | --- | --- |
| `target` | 是 | 插入目标条目的标识 |
| `anchor` | 否 | `last` / `back`、`first` / `front`、`before`、`after`，默认为 `last` |
| `relativeTo` | `before` 与 `after` 时必需 | `target` 中作为插入位置参照的条目 |

`target` 不存在的插入不报错，直接跳过，插件因此可以为宿主中不一定存在的菜单提供插入。`relativeTo` 不存在的 `after` 或 `before` 插入同样跳过。

插入的元素不得有子元素，`phony` 条目不得插入。

## 编译

```
qak_aec [options] <manifest>
```

| 选项 | 含义 |
| --- | --- |
| `-o <file>` | 将源文件写入文件，而非标准输出 |
| `--header <file>` | 同时生成声明获取函数的头文件。省略时，源文件自行声明该函数 |
| `--function <name>` | 获取函数的名称，必需 |
| `--namespace <namespace>` | 获取函数所在的命名空间，例如 `hello::daw`，省略时为全局命名空间 |
| `--export-directive <macro>` | 放在函数声明之前的宏，用于从动态库导出或隐藏该函数 |
| `--export-file-name <header>` | 定义该宏的头文件，以尖括号包含。只给出本选项而没有 `--export-directive` 时报错 |
| `-D <key>[=<value>]` | 定义变量。省略值时，值为键本身，为真 |
| `--text-translation-context <ctx>` | 覆盖 `text` 的翻译上下文 |
| `--category-translation-context <ctx>` | 覆盖 `category` 的翻译上下文 |
| `--description-translation-context <ctx>` | 覆盖 `description` 的翻译上下文 |

函数名、命名空间的各段与导出宏须为由 ASCII 字母、数字与 `_` 组成、不以数字开头的标识符，否则报错。任何错误都输出到标准错误，并以退出码 `1` 结束。

在 CMake 中：

```cmake
qak_add_action_extension(_src "core-actions.xml"
    FUNCTION coreActions
    NAMESPACE hello::daw
    EXPORT_DIRECTIVE HELLOUTAU_WIDGETS_EXPORT
    EXPORT_FILE_NAME helloutau/Widgets/HelloUtauWidgetsGlobal.h
    DEFINES ENABLE_DEBUG_ACTIONS=1
)
target_sources(MyApp PRIVATE ${_src})
target_include_directories(MyApp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

`FUNCTION` 必需，其余选项与命令行选项一一对应。输出变量包含生成的源文件与头文件。头文件名为清单去掉后缀的文件名加 `.qak.h`，位于 `CMAKE_CURRENT_BINARY_DIR`，包含它的目标须将该目录加入包含路径。

## 使用编译结果

生成的头文件声明获取函数，源文件定义它：

```cpp
// core-actions.qak.h
#include <QAKCore/actionextension.h>

#include <helloutau/Widgets/HelloUtauWidgetsGlobal.h>

namespace hello::daw {
    HELLOUTAU_WIDGETS_EXPORT const QAK::ActionExtension *coreActions();
}
```

```cpp
#include "core-actions.qak.h"

registry->addExtension(hello::daw::coreActions());
```

导出宏在构建库与使用库时分别展开为什么，由定义它的头文件决定。

`ActionExtension::hash()` 是清单字节的 SHA-256 摘要。`ActionRegistry` 将其与用户保存的布局一同存储，扩展增加或修改时只合并新的条目，用户的其余自定义保持不变。

## 翻译

生成文件的末尾有一个位于 `#if 0` 中的函数，对每个不同的文本、类别与描述，在运行时查找它的每个上下文中各调用一次 `QCoreApplication::translate()`。该函数不参与编译，只供 `lupdate` 提取字符串。`lupdate` 应处理生成的源文件，而非清单。

运行时 `ActionItemInfo::text(true)` 在 `textTr` 属性指定的上下文中查找译文，未指定时使用 `configuration` 中的上下文，二者都未指定时使用内置的上下文。未安装翻译时返回空字符串，调用方须回退到 `text(false)`。
