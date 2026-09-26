# 设计问题

本文档记录尚未解决的设计问题。编号与内部问题清单一致，保持不变。缺少的功能、测试覆盖与性能问题见 [TODO.md](TODO.md)。

排列顺序大致按照「越晚修改代价越高」。第 17、18、19 条应当最先处理。

## 17. 条目的声明类型与布局中的形态

**现状。** 条目有两级类型：声明类型（`ActionItemInfo::Type`）是条目的**身份**，即 registry 认定的条目种类；布局条目的类型（`ActionLayoutEntry::Type`）是条目在某一位置的**形态**。两级类型是有意的设计。例如一个 menu 可以在某处以 action 的形态出现，子项由外部逻辑维护（如最近打开的文件），对 registry 而言它相当于一个 action。约束因此是非对称的：形态可以比身份窄（menu 作为 action 使用），不能比身份宽（action 不能作为 menu 使用）。

**已经成立的部分。** 由 action 到 menu 的方向已被编译器禁止：`findOrInsertItemInfo()` 对 Action 条目只接受 `action` 与 `item` 标签，`<menu id="某个 action">` 报告 inconsistent tag 并退出；`parseLayoutRecursively()` 对 Action 条目无条件产生 Action 形态。条目先于布局解析，重复的标识报错，不存在借助顺序绕过检查的途径。插入使用同一套检查。

**问题。** 反方向无法表达。Menu 与 Group 条目接受的标签为 `group`、`menu`、`menuBar`、`toolBar`、`item`，不含 `action`：

```xml
<items><menu id="r.recentFiles" text="Recent Files" /></items>
<layouts><menu id="r.file"><action id="r.recentFiles" /></menu></layouts>
```

```
qak_aec: r.xml: layout element "r.recentFiles" has inconsistent tag "action" with the item element "menu"
```

即使放行该标签，Menu 与 Group 的分支也只在标签为 `menu` 时产生 Menu 形态，其余一律产生 Group 形态，Action 形态不可达。Quick 后端在运行时支持这种用法（`createAction()` 同时处理 `QQuickAction` 与 `QQuickMenu`，登记为 Menu 组件的标识只设置标题与图标，不递归），但编译器不允许写出。

标签到形态的映射不是一一对应的，塌缩规则是隐含的 else 分支：

| 身份 | 标签 | 形态 | 评价 |
| --- | --- | --- | --- |
| Action | `action` / `item` | Action | 正确 |
| Action | 其他 | 编译错误 | 正确 |
| Menu | `menu` | Menu | 正确 |
| Menu | `item` | Group | 与直觉相反，`item` 应表示条目本来的形态 |
| Menu | `menuBar` / `toolBar` | Group | 这两个标签不表示内联 |
| Menu | `action` | 编译错误 | 缺失的用法 |
| Group | `menu` | Menu | 合法且有意义（将一组条目显示为子菜单），但未写入规范 |
| Group | `group` / `item` / `menuBar` / `toolBar` | Group | 正确 |

**可选的方向。** 倾向第一种。

1. **增加显式的形态属性。** 标签只表示身份，形态由 `as` 属性指定：`<menu id="r.recentFiles" as="action" />`。检查分为两条相互独立的规则：标签须与身份相容，`as` 须属于身份所允许的形态（Action 允许 action，Menu 与 Group 允许 action、group、menu）。省略 `as` 时取身份的自然形态。三种塌缩全部消失，`item` 的含义明确，错误信息可以区分身份不符与形态不允许。
2. **修正标签到形态的映射**，使每个标签只对应一种形态：`menu` 对应 Menu，`group` 对应 Group，`action` 对应 Action，`item` 对应自然形态，`menuBar` 与 `toolBar` 出现在嵌套位置时报错。省去一个属性，但 menu 作为 action 使用时写作 `<action id="r.recentFiles" />`，读者容易误以为引用的是一个 action。
3. **维持现状**，只允许 Menu 与 Group 以 Action 形态出现，并将 `item` 改为自然形态。改动最小，但 `menuBar` 与 `toolBar` 被静默当作 Group 的问题仍然存在。

无论采用哪一种，都须同时完成三件事：错误信息区分身份不符与形态不允许；规范中补充身份与形态的合法组合表；编译器测试覆盖表中每一种组合。第 43、44、46 条是该模型在编译器之外尚未保证的部分。

另见第 45 条：以 Action 形态出现的 menu，其内容不再由应用程序自行维护，最近打开的文件这一类用法须另行设计。

## 18. 目录（catalog）的声明语义与实际语义不一致

`findOrInsertItemInfo()` 中的 `if (info.catalog.isEmpty()) info.catalog = upperCatalog;` 使条目的目录静默继承其布局父节点，包括 group。以 `examples/shared/core-actions.xml` 为例，`core.openFile` 的目录是 `core.fileOpenActions`（一个 group），声明的 `defaultCatalog` 没有被任何条目使用，`core.catalog.others` 成为无用节点。

此外还有三处不一致：布局中的条目继承布局父节点，插入中的条目则使用 `defaultCatalog`，同一个 action 由宿主排入菜单与由插件插入时归属不同；被多处引用时以第一处为准，结果依赖文档顺序；`topLevel` 兼作「不使用 `defaultCatalog`」的标记，使视图层的概念决定了逻辑树的根集合。

应当二选一：目录只取显式声明与 `defaultCatalog`；或者明确规定目录默认镜像布局，并删除 `defaultCatalog`。

## 19. `hash` 的实现语义与使用语义不符，会产生重复的菜单项

`hash` 是清单字节的 SHA-256 摘要，却被用来判断「该扩展的贡献是否已并入用户保存的布局」。修改一行注释或缩进就会改变摘要，`correctLayouts()` 因此将其视为新扩展，再次执行 `applyInsertion()`。保存的布局中已经存在这些插入的条目，而 `applyInsertion()` 不去重，`LayoutsTrait::Unique = false` 也不去重，结果是菜单项重复。

修正方法二选一：对条目、布局与插入的规范化序列化结果计算摘要；或者使合并满足幂等（插入带有稳定的标识，合并前检查目标中是否已有其结果）。

## 20. `defaultLayouts()` 与 `correctLayouts({})` 并不等价

文档称 `defaultLayouts()` 等价于 `correctLayouts(ActionLayouts())`。实际上，`defaultLayouts()` 先将所有扩展的全部条目放入邻接表，再逐个应用插入；`correctLayouts()` 则对每个扩展依次加入其条目并应用其插入。后者中，若扩展 A 的插入目标属于登记在后的扩展 B，此时 B 的条目尚未加入，`applyInsertion()` 直接返回，插入被静默丢弃。触发条件为：用户保存过布局，一次新增两个以上的扩展，且其中一个向另一个的菜单插入条目。

两个函数大部分重复，应当合并为一个。本条由阅读代码得出，尚未构造用例验证，修改前须先编写复现测试。

## 21. 错误一律静默，没有严格模式

- 布局中拼错的标识会隐式声明一个新条目。例如 `p.typoAcion` 推导出文本 `Typo Acion`。
- `<menu>` 上的 `shortcut` 与 `category` 被静默丢弃，由于 `reservedKeys` 的过滤，也不作为自定义属性保留。
- 目标不存在的插入被静默跳过。
- 指向不存在节点的 `catalog` 会生成一个没有 `ActionItemInfo` 的目录节点。

「布局中的标识隐式声明条目」本身是合理的设计，但须配套提供 `--strict` 或 `--warn-undeclared` 选项。

## 22. `version` 既不校验也不使用

`QVersionNumber::fromString("banana")` 返回空版本，`parserVersion() < 空版本` 为假，检查实际无效：`<version>banana</version>` 可以通过编译，并原样写入 `data.version`。运行时没有代码读取 `ActionExtension::version()`，`ACTION_EXTENSION_VERSION` 只用于共享的空对象。应当补充格式校验与运行时的兼容性检查，或者删除该字段。

## 23. 表示层的概念进入共享数据，Quick 后端显示字面的 `&`

标识语法中的 `&`（助记符）与 `^`（省略号）是 QtWidgets 的约定，被编入 `text()`：`m.&openFile^` 的文本为 `&Open File...`。`ActionItemInfo` 没有返回去除标记后文本的接口，Quick 后端原样传递，而 QML 不解释 `&`。`parser.cpp` 中的 `simplifyActionText()` 实现了去除标记的功能，但它是死代码，且位于编译器中，运行时无法使用。应当增加返回去除标记后文本的访问函数，或者将这两个标记移出标识语法。

## 24. 翻译的回退被自身抵消

`tryTranslate()` 正确地回退到原文，`translateString()` 却在 `if (!ok) return {}` 中丢弃了结果。因此未安装翻译文件时，`text(true)` 一律返回空字符串，两个后端各自重复实现三级回退。应当在内部回退，或者将 `ok` 提供给调用方。

## 25. 翻译上下文是扩展级的配置，却按条目存储

`ActionExtensionData` 中没有翻译上下文的字段，解析器因此将 `textTr`、`categoryTr`、`descriptionTr` 复制到每个条目的属性中，N 个条目存储 3N 份重复的字符串。此外，`translateString()` 以线性扫描查找键，而 `QMap::find` 即可满足（`ActionAttributeKey` 的 `operator<` 先比较名称再比较命名空间，命名空间为空的键可以确定地查找）。

## 26. `ActionExtension` 没有空对象保护，访问函数不检查边界

`ActionItemInfo` 与 `ActionInsertion` 都有共享的空对象，`ActionExtension{}` 的 `d.data` 却是空指针，任何访问函数都会立即解引用空指针。`item(int)` 与 `insertion(int)` 不检查边界，`item(9999)` 返回一个不为 null 的越界视图。`Data d` 是公开的成员，以便生成的代码进行聚合初始化，其不变量完全依赖代码生成器保证。

## 27. 两个互相竞争的数据来源

`ActionItemInfo::children()` 是扩展声明的默认子项，`ActionRegistry::layouts()` 才是合并插入与用户自定义之后实际生效的布局，两者的主次从名称上看不出来。`children()` 是公开接口，据此构建菜单会忽略用户的自定义与其他插件的插入。至少应当改名，或在文档中注明它只是默认值。

## 28. `Q_GADGET` 不完整

`ActionLayoutEntry` 声明了 `Q_PROPERTY(ActionLayoutEntry::Type type ...)`，却没有 `Q_ENUM(Type)`，元类型系统不认识该枚举，QML 与 `QVariant` 的转换无法取得其值。`ActionLayouts` 的 `Q_GADGET` 中没有任何属性。

## 29. 插入不可组合

多个插件插入同一位置时，顺序完全取决于扩展的登记顺序，清单中无法表达优先级，也无法表达「排在某个插件之后」。`relativeTo` 只按标识匹配，不能指向分隔符（分隔符没有标识）。

## 30. `ActionLayoutEntry` 没有 `operator==`

`buildGraph()` 的 `if constexpr (Trait::Unique)` 分支调用了 `contains()`，需要 `operator==`。目前 `LayoutsTrait::Unique = false`，该分支没有实例化，但一旦启用即无法编译。

## 31. 与类型相关的字段没有体现在模型中

`shortcuts` 只对 Action 解析，`category` 只对 Action 读取，而 `ActionItemInfo` 对所有类型都提供这些访问函数，非 Action 条目返回空值。这一约束依靠约定，而不是类型。

## 43. 身份与形态的不变量在持久化边界失效

编译器保证 action 不能作为 menu 出现，但 `ActionLayouts::fromJsonObject()` 接受任意的类型字符串，`correctLayouts()` 也不做任何类型检查。用户保存的布局文件被手工修改、被旧版本写坏，或者扩展升级后某个标识的身份改变，都可能产生 `{"id": "core.openFile", "type": "Menu"}` 这样的条目。

结果不是报错，而是静默的降级：Widgets 后端调用 `menuForId()`，在 `items` 与 `autoItems` 中都找不到该标识，于是 `createSubMenu()` 创建一个标题为 Open File 的空子菜单，原来的 action 消失。Quick 后端的 `createMenu()` 同理。

registry 同时持有条目表与布局，是唯一能够进行这一检查的地方，应当在 `correctLayouts()` 中丢弃身份与形态不相容的条目并输出警告。本条须在第 17 条选定方案后同时完成，否则编译器的检查只在形式上成立。

## 44. `ActionLayoutsModel` 无法检查身份与形态是否相容

`validateActionLayoutEntry()` 只检查「分隔符与 stretch 当且仅当标识为空」，`validateEntryChange()` 只多检查一个环。模型持有的是邻接表与 `hashList`，不持有 registry 或条目表，因而无法得知某个标识的身份。该模型用于设置页面中的拖放编辑：用户将一个 action 拖为子菜单时，模型照样接受，写出第 43 条所述的错误布局。应当为模型提供 `ActionRegistry *`（或条目信息的查询回调），或者规定模型只负责结构编辑，由 registry 在 `setLayouts()` 时检查。

## 45. Widgets 后端重建容器时清除其全部内容（已定）

`WidgetActionContextPrivate::updateLayouts()` 对每个登记的容器（菜单、菜单栏、工具栏）先移除全部动作，再按布局重新填充。应用程序自行加入容器的动作因此在每次更新布局时被清除。

**决定：维持现状。** 登记给 context 的容器，其内容一律由 QActionKit 管理，应用程序不得自行向其中加入动作。`WidgetActionContext` 的接口文档写明这一约定。第 17 条所述「menu 以 Action 形态出现、子项由外部维护」的用法因此不成立，最近打开的文件这一类动态内容须另行设计。

## 46. `menuBar` 与 `toolBar` 编译后没有区别

`parseItemAttrs()` 对这两个标签都设置 `type = Menu; topLevel = true;`。`info.tag` 只存在于解析器的中间结构中，`generator.cpp` 不输出它，因此 `ActionItemInfo` 无法区分弹出菜单、菜单栏与工具栏，应用程序只能硬编码哪个标识是工具栏。mainwindow 示例即是如此（`addMenuBar("core.mainMenu")`、`addToolBar("core.mainToolBar")`），而清单中两者都写作 `<menu topLevel="true">`。按照「声明类型决定身份」的原则，此处身份定义不足：标签提供了三个词，编译结果只有一种。应当为 `ActionItemInfo` 增加顶层种类（弹出菜单、菜单栏、工具栏），或者规定三个标签为同义词并写入规范。

## 47. `if` 跳过声明后，布局中的引用会静默重建一个降级的条目

`parse()` 对被 `if` 跳过的 `<items>` 元素直接跳过，该元素不进入 `itemInfoMap`。随后布局中的引用经过 `findOrInsertItemInfo()` 的 else 分支，以引用处的标签与属性创建一个新条目。

例如声明为 `<action id="c.debugDump" text="Dump State" category="Debug" shortcut="Ctrl+D" description="dumps" if="ENABLE_DEBUG" />`，布局中只写 `<action id="c.debugDump" />`。关闭开关后，编译结果的文本为 Debug Dump，类别、描述与快捷键为空。条目没有消失，只是全部元数据被降级，而且身份改由引用处的标签决定，违反了「声明类型决定身份」的原则。

应当在 `if` 跳过声明时记录该标识，此后的引用一律报错（提示引用处也须加上 `if`）；或者规定 `if` 只从布局中移除条目，保留其声明。
