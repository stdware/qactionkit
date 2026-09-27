# 设计问题

本文档记录尚未解决的设计问题。编号与内部问题清单一致，保持不变。缺少的功能、测试覆盖与性能问题见 [TODO.md](TODO.md)。

排列顺序大致按照「越晚修改代价越高」。第 17、18、19 条应当最先处理。

## 17. 条目的声明类型与布局中的形态（已定）

**现状。** 条目有两级类型：声明类型（`ActionItemInfo::Type`）是条目的**身份**，即 registry 认定的条目种类；布局条目的类型（`ActionLayoutEntry::Type`）是条目在某一位置的**形态**。Menu 与 Group 可以互换形态：声明为 menu 的条目可以在布局中写作 `<group>`，作为组展开；声明为 group 的条目可以写作 `<menu>`，显示为子菜单。设置页也可以在运行时修改形态（`ActionLayoutsModel::setData()`）。Action 的形态固定为 Action，AEC 拒绝以其他标签引用 action。

**问题。**

1. 标签到形态的映射有隐式的塌缩。引用 Menu 或 Group 条目时，只有 `menu` 标签产生 Menu 形态，其余标签一律产生 Group 形态。声明为 menu 的条目在布局中写作 `<item>`，结果被当作组展开，而 `item` 应当表示条目声明的类型。
2. `menuBar` 与 `toolBar` 编译后与 `<menu topLevel="true">` 没有区别（第 46 条），在引用处却被当作 group。
3. 内容由应用程序维护的菜单无法表达。例如「打开最近的文件」子菜单的内容由应用程序生成，应用程序以 `addAction()` 登记该 `QMenu` 的 `menuAction()`。对 QActionKit 而言它是一个 action，但它不是命令：执行它没有意义，因此不应进入命令面板，也不应绑定快捷键。现有的 action 无法表达这一区别。

**决定。**

1. 删除 `menuBar` 与 `toolBar` 标签。顶层容器一律写作 `<menu topLevel="true">`，菜单栏与工具栏的区别由应用程序登记时决定（`addMenuBar()`、`addToolBar()`）。
2. 布局中的标签决定 Menu 与 Group 条目的形态：`menu` 为 Menu，`group` 为 Group，`item` 为声明的类型。以 `action` 引用 Menu 或 Group 条目仍为错误。
3. `<action>` 新增属性 `external`。`external="true"` 的 action 是一个内容由应用程序维护的菜单：
   - 应用程序以 `addAction()` 登记该菜单的 `menuAction()`。它不经 `addMenu()` 登记，后端不清空也不生成其内容。
   - 身份与形态都是 Action，因此不能以 Menu 或 Group 形态出现，也不能在布局中带有子项，这两条由 action 的现有规则保证。
   - 不能带有 `shortcut` 或 `shortcuts` 属性，AEC 报错。`external` 用于 action 以外的标签也是错误。
   - `ActionItemInfo` 提供查询该属性的访问函数。
4. 身份为 Action 且不是 external 的条目是命令：进入命令面板，可以绑定快捷键。external action、Menu、Group 与 Phony 都不是命令。

身份、标签与形态的合法组合：

| 身份 | 标签 | 形态 |
| --- | --- | --- |
| Action（含 external） | `action` / `item` | Action |
| Action（含 external） | 其他 | 错误 |
| Menu | `menu` / `item` | Menu |
| Menu | `group` | Group |
| Group | `group` / `item` | Group |
| Group | `menu` | Menu |
| Menu / Group | `action` | 错误 |

实现时须同时完成三件事：AEC 的错误信息区分身份不符与形态不允许；规范中补充上表；AEC 测试覆盖表中每一种组合。第 43、44 条是该模型在 AEC 之外尚未保证的部分。

## 18. 目录（catalog）的声明语义与实际语义不一致（已定）

`findOrInsertItemInfo()` 中的 `if (info.catalog.isEmpty()) info.catalog = upperCatalog;` 使条目的目录静默继承其布局父节点，包括 group。以 `examples/shared/core-actions.xml` 为例，`core.openFile` 的目录是 `core.fileOpenActions`（一个 group），声明的 `defaultCatalog` 没有被任何条目使用，`core.catalog.others` 成为无用节点。

此外还有三处不一致：布局中的条目继承布局父节点，插入中的条目则使用 `defaultCatalog`，同一个 action 由宿主排入菜单与由插件插入时归属不同；被多处引用时以第一处为准，结果依赖文档顺序；`topLevel` 兼作「不使用 `defaultCatalog`」的标记，使视图层的概念决定了逻辑树的根集合。

目录默认镜像布局的方向不可行，原因有二。第 17 条删除 `toolBar` 标签之后，AEC 无法区分工具栏与菜单，工具栏中的引用也会参与推导。目录在编译时确定，布局却可以由用户在运行时修改，镜像只在默认布局下成立。目录服务于设置页的条目树与应用程序按逻辑分组的查询（例如取出所有面板动作），两者都要求它是条目身份层面的稳定信息。

**决定。** 目录只来自声明，布局与插入都不影响目录。

- 条目的目录依次取：自身的 `catalog` 属性；在 `<items>` 中直接包含它的 `phony`；`defaultCatalog`。
- `<items>` 中的 `phony` 可以带有子元素，子元素是普通的条目声明，可以嵌套 `phony` 以形成多层目录。action、menu 与 group 的声明仍然不能带有子元素，因此 `<items>` 中的嵌套只表示目录，不与布局的嵌套混淆。
- 既没有 `catalog` 也不在其他 `phony` 中的 `phony` 是目录树的根。其余条目在这种情况下归入 `defaultCatalog`，`topLevel` 不再影响目录。
- `catalog` 指向的标识必须由本扩展声明，可以是 phony、menu 或 group，否则 AEC 报错。`defaultCatalog` 同样须由本扩展声明，AEC 不再自动补充 phony，因为补充的条目只能使用从标识推导的文本，无法指定文本与翻译上下文。
- 插件使用宿主的目录节点时，在自己的清单中将它声明为 phony。宿主不存在时，该节点的文本与翻译取自插件。
- 多个扩展声明同一标识时，registry 优先采用不是 phony 的声明，因为拥有节点的扩展才会将它声明为 menu 或 group；同为 phony 时采用先登记的。两个都不是 phony 的声明相冲突，registry 以 `qCWarning` 报告，保留先登记的。

```xml
<items>
    <phony id="core.file" text="File">
        <action id="core.openFile" text="Open File" />
        <action id="core.saveFile" text="Save File" />
        <phony id="core.file.export" text="Export">
            <action id="core.exportMidi" />
        </phony>
    </phony>
</items>
```

## 19. 用户布局的合并方式（已定）

`hash` 是清单字节的 SHA-256 摘要，却被用来判断「该扩展的贡献是否已并入用户保存的布局」。修改一行注释或缩进就会改变摘要，`correctLayouts()` 因此将其视为新扩展，再次执行 `applyInsertion()`。保存的布局中已经存在这些插入的条目，而 `applyInsertion()` 不去重，`LayoutsTrait::Unique = false` 也不去重，结果是菜单项重复。

阅读 `correctLayouts()` 另外得出两个问题。其一，扩展自身菜单中新增的条目不会出现：保存的布局中已有该菜单，合并时整体跳过，新条目只有在用户重置布局后才可见。其二，用户删除的插入条目在插件更新后重新出现，因为插入被全部重新执行。三个问题的共同原因是：保存的是合并后的完整布局，而「已合并的内容」只以整份清单的摘要记录，粒度不足以区分哪些条目是新的。三个问题都尚未以测试复现。

**决定：只保存用户的改动。** 做法与 IntelliJ 平台的菜单自定义相同：该平台以 `ActionUrl` 记录相对默认菜单的新增、删除与移动（`ADDED`、`DELETED`、`MOVE`），`CustomActionsSchema` 只持久化这些记录，构造菜单时由 `CustomizationUtil.correctActionGroup()` 将记录应用于默认分组。

- 每次均由当前登记的全部扩展重新计算默认布局，再按顺序重放用户的改动记录。计算默认布局的路径只有一条。
- 改动记录描述某个容器中的一个条目被新增、删除或移动。
- 位置以锚点表示，不以下标表示，沿用插入的锚点：`first`、`last`、`after` 与 `before`，后两者带有相邻条目的标识。下标在扩展更新后会错位：默认布局中每多出一个排在前面的条目，用户新增的条目就相对前移一格。锚点表达的是用户的意图（「在某个条目之后」），扩展更新后仍然成立。IntelliJ 平台的 `ActionUrl` 使用下标（`getAbsolutePosition()`），此处不采用。
- 锚点条目不存在时退到容器末尾，与插入的处理相同。被删除或移动的条目不存在时跳过该记录。
- 分隔符没有标识，删除或移动分隔符的记录以相邻条目定位，例如「『打开』之后的分隔符」。
- 对外的接口只读写改动记录：`ActionRegistry` 提供 `layoutChanges()`、`setLayoutChanges()` 与 `addLayoutChange()`，`layouts()` 返回重放之后的布局，只读。删除 `setLayouts()` 与 `resetLayouts()`，恢复默认即清空改动记录。IntelliJ 平台的 `CustomActionsSchema` 同样只提供 `getActions()`、`setActions()` 与 `addAction()`，没有直接设置整棵菜单树的接口。
- 删除 `ActionExtension::hash()`、`ActionLayouts` 的 `hashList`，以及 AEC 计算摘要的代码。
- 扩展更新后，新条目按默认布局出现，用户删除的条目保持删除，插入不会重复。

- 改动记录在保存时比较默认布局与用户编辑后的布局得出，设置页只编辑一份完整的布局，不在每次编辑时产生记录。

## 20. `defaultLayouts()` 与 `correctLayouts({})` 并不等价（随第 19 条消失）

第 19 条的决定之后只剩「计算默认布局」一条路径，`correctLayouts()` 被重放记录取代，本条随之消失。以下为原问题。

文档称 `defaultLayouts()` 等价于 `correctLayouts(ActionLayouts())`。实际上，`defaultLayouts()` 先将所有扩展的全部条目放入邻接表，再逐个应用插入；`correctLayouts()` 则对每个扩展依次加入其条目并应用其插入。后者中，若扩展 A 的插入目标属于登记在后的扩展 B，此时 B 的条目尚未加入，`applyInsertion()` 直接返回，插入被静默丢弃。触发条件为：用户保存过布局，一次新增两个以上的扩展，且其中一个向另一个的菜单插入条目。

两个函数大部分重复，应当合并为一个。本条由阅读代码得出，尚未构造用例验证，修改前须先编写复现测试。

## 21. 错误一律静默，没有严格模式（已定）

- 布局中拼错的标识会隐式声明一个新条目。例如 `p.typoAcion` 推导出文本 `Typo Acion`。
- `<menu>` 上的 `shortcut` 与 `category` 被静默丢弃，由于 `reservedKeys` 的过滤，也不作为自定义属性保留。
- 目标不存在的插入被静默跳过。
- 指向不存在节点的 `catalog` 会生成一个没有 `ActionItemInfo` 的目录节点。

「布局中的标识隐式声明条目」本身是合理的设计，但须配套提供 `--strict` 或 `--warn-undeclared` 选项。

**决定：AEC 默认严格，不设开关。**

- 取消隐式声明。布局与插入中引用的标识必须在本扩展的 `<items>` 中声明，否则报错。规范规定布局与插入只引用本扩展的条目，因此 AEC 能够完整检查。
- 布局与插入中的引用元素只能带有 `id` 与 `if`，其他属性写在声明上。
- `shortcut`、`shortcuts` 与 `category` 只属于 action，写在其他标签上时报错。
- `catalog` 与 `defaultCatalog` 指向本扩展未声明的标识或 action 时报错（第 18 条）。
- 不带命名空间的未知属性报错，自定义属性必须带有命名空间，例如 `diffscope:componentType`。拼错的保留属性因此不再被当作自定义属性保留。
- `anchor` 为 `first` 或 `last`（含省略 `anchor`）时写了 `relativeTo`，AEC 报错，因为它不起作用，多半是锚点写错了。`anchor` 为 `before` 或 `after` 时缺少 `relativeTo`，同样报错。
- 插入的目标通常属于其他扩展，AEC 无法检查。registry 计算默认布局时，目标不存在的插入以 `qCWarning` 报告，目标中不存在 `relativeTo` 所指条目的插入也是如此。

## 22. `version` 既不校验也不使用（已定）

`QVersionNumber::fromString("banana")` 返回空版本，`parserVersion() < 空版本` 为假，检查实际无效：`<version>banana</version>` 可以通过编译，并原样写入 `data.version`。运行时没有代码读取 `ActionExtension::version()`，`ACTION_EXTENSION_VERSION` 只用于共享的空对象。应当补充格式校验与运行时的兼容性检查，或者删除该字段。

**决定：** 格式版本保持 `1.0`，本轮的不兼容改动不提升版本。AEC 对不是有效版本号的字符串报错。运行时的 `ActionExtension::version()` 保留。

## 23. 表示层的概念进入共享数据，Quick 后端显示字面的 `&`（已定）

标识语法中的 `&`（助记符）与 `^`（省略号）是 QtWidgets 的约定，被编入 `text()`：`m.&openFile^` 的文本为 `&Open File...`。`ActionItemInfo` 没有返回去除标记后文本的接口，Quick 后端原样传递，而 QML 不解释 `&`。`parser.cpp` 中的 `simplifyActionText()` 实现了去除标记的功能，但它是死代码，且位于 AEC 中，运行时无法使用。应当增加返回去除标记后文本的访问函数，或者将这两个标记移出标识语法。

**更正。** 「QML 不解释 `&`」不完全成立。Qt 6.11 的 `QQuickAbstractButton::buttonChange()` 以 `QKeySequence::mnemonic()` 为按钮设置快捷键，`MenuItem` 即是 `AbstractButton`，因此 `&` 至少被解释为助记快捷键。Qt 的 dev 分支文档写明助记符标记总是从显示的文本中去除，6.11 是否同样去除尚未实测。

**决定。** 标识语法中的 `&` 与 `^` 保留。第 24 条的 `ActionText` 提供 `withoutMnemonic()`，返回去除助记符标记后的文本，供命令面板、快捷键设置页与工具提示等菜单以外的场合使用，`...` 保留。实现采用 Qt 为原生菜单去除助记符的 `QPlatformTheme::removeMnemonics()`：去除 `&`，`&&` 变为 `&`，并连同前面的空格去除中文界面常见的 `(&O)` 形式，否则 `打开(&O)` 在命令面板中显示为 `打开(O)`。`simplifyActionText()` 是死代码，随之删除。以一个 QML 程序实测 Qt 6.11 的显示，据此确定 Quick 后端是否需要自行去除 `&`。

## 24. 翻译的回退被自身抵消（已定）

`tryTranslate()` 正确地回退到原文，`translateString()` 却在 `if (!ok) return {}` 中丢弃了结果。因此未安装翻译文件时，`text(true)` 一律返回空字符串，两个后端各自重复实现三级回退。应当在内部回退，或者将 `ok` 提供给调用方。

以空字符串表示「未翻译」是有意的设计，使调用方能够判断翻译是否成功，但空字符串与合法的空值冲突：`category` 与 `description` 未指定时为空，`category(true)` 返回空时无法区分「未翻译」与「没有类别」。最常见的需求「有译文用译文，否则用原文」则须由每个调用方重复实现。

**决定。** `text()`、`description()` 与 `category()` 去掉 `bool` 参数，返回同一个结构体：

```cpp
struct ActionText {
    QString source;                      ///< The text written in the manifest
    std::optional<QString> translation;  ///< The installed translation, or std::nullopt if none

    /// Returns the translation if one exists, and the source text otherwise.
    inline QString toString() const {
        return translation.value_or(source);
    }

    /// Returns the result of toString() with the mnemonic markers removed.
    QString withoutMnemonic() const;
};
```

显示时调用 `toString()`，判断是否已翻译时检查 `translation`，需要原文时读取 `source`。两个后端中重复的回退随之删除。

## 25. 翻译上下文是扩展级的配置，却按条目存储（已定）

`ActionExtensionData` 中没有翻译上下文的字段，解析器因此将 `textTr`、`categoryTr`、`descriptionTr` 复制到每个条目的属性中，N 个条目存储 3N 份重复的字符串。此外，`translateString()` 以线性扫描查找键，而 `QMap::find` 即可满足（`ActionAttributeKey` 的 `operator<` 先比较名称再比较命名空间，命名空间为空的键可以确定地查找）。复制出的键还出现在 `ActionItemInfo::attributes()` 中，应用程序无法区分清单中写出的属性与 AEC 补入的属性。

**决定。** 与第 24 条一同实现，清单的写法不变。

- `ActionExtensionData` 增加三个扩展级翻译上下文字段，由 AEC 写入一次。
- 条目只在清单中写出 `textTr`、`categoryTr` 或 `descriptionTr` 时记录覆盖值，存放在专门的字段中，不进入 `attributes()`。`attributes()` 只包含自定义属性，与第 21 条「自定义属性必须带有命名空间」一致。
- 翻译上下文依次取条目的覆盖值、扩展级上下文与内置默认值，不再扫描属性表。

## 26. `ActionExtension` 没有空对象保护，访问函数不检查边界（已定）

`ActionItemInfo` 与 `ActionInsertion` 都有共享的空对象，`ActionExtension{}` 的 `d.data` 却是空指针，任何访问函数都会立即解引用空指针。`item(int)` 与 `insertion(int)` 不检查边界，`item(9999)` 返回一个不为 null 的越界视图。`Data d` 是公开的成员，以便生成的代码进行聚合初始化，其不变量完全依赖代码生成器保证。

**决定。** 公开的 `Data d` 保留。它与 `QMetaObject` 的 `struct Data { // private data ... } d;` 相同，由生成的代码聚合初始化。第 48 条之后，应用程序只从生成的函数取得 `const ActionExtension *`，不自行构造，因此不增加空对象。访问函数以 `Q_ASSERT` 检查 `d.data` 非空，`item(int)` 与 `insertion(int)` 以 `Q_ASSERT` 检查下标范围，与 `QList::at()` 的做法相同。不增加 `items()`、`insertions()` 等遍历接口。

## 27. 两个互相竞争的数据来源（已定）

`ActionItemInfo::children()` 是扩展声明的默认子项，`ActionRegistry::layouts()` 才是合并插入与用户自定义之后实际生效的布局，两者的主次从名称上看不出来。`children()` 是公开接口，据此构建菜单会忽略用户的自定义与其他插件的插入。至少应当改名，或在文档中注明它只是默认值。

**决定：** 不改名。`children()` 的接口注释写明它是清单声明的默认子项，实际生效的布局由 `ActionRegistry::layouts()` 给出。

## 28. `Q_GADGET` 不完整（已定）

`ActionLayoutEntry` 声明了 `Q_PROPERTY(ActionLayoutEntry::Type type ...)`，却没有 `Q_ENUM(Type)`，元类型系统不认识该枚举，QML 与 `QVariant` 的转换无法取得其值。`ActionLayouts` 的 `Q_GADGET` 中没有任何属性。

**决定：** 为 `ActionLayoutEntry` 补上 `Q_ENUM(Type)`。`ActionLayouts` 的内容随第 19 条重新设计，其 `Q_GADGET` 届时再定。

**实现中的发现。** 只补 `Q_ENUM` 并不足够：属性原先声明为 `Q_PROPERTY(ActionLayoutEntry::Type ...)`，moc 记录的作用域是 `ActionLayoutEntry`，而类的全名是 `QAK::ActionLayoutEntry`，`QMetaProperty` 按作用域查找枚举时找不到，`isEnumType()` 仍为假。属性类型须写作类内的 `Type`。

## 29. 插入不可组合（已定）

多个插件插入同一位置时，顺序完全取决于扩展的登记顺序，清单中无法表达优先级，也无法表达「排在某个插件之后」。`relativeTo` 只按标识匹配，不能指向分隔符（分隔符没有标识）。

**决定：** `<insertion>` 增加整数属性 `priority`，省略时为 1000，做法参照 GNU C 的 `__attribute__((constructor(priority)))`。

- 只在位置相同的插入之间比较，即 `target`、`anchor` 与 `relativeTo` 都相同的插入。
- 数值小的插入在菜单中排在前面。「在前」指最终的位置而不是执行的顺序：`last` 依次追加，数值小的先执行；`first` 依次放到开头，数值小的后执行。
- 优先级相同时按扩展的登记顺序，同一扩展内按清单中出现的顺序。
- 不像 GNU C 那样保留一段数值。
- 分隔符没有标识，不能作为锚点。插入某一段时，以该段的 group 为锚点，规范中补充这一说明。

## 30. `ActionLayoutEntry` 没有 `operator==`（已定）

`buildGraph()` 的 `if constexpr (Trait::Unique)` 分支调用了 `contains()`，需要 `operator==`。目前 `LayoutsTrait::Unique = false`，该分支没有实例化，但一旦启用即无法编译。

**决定：** 增加比较类型与标识的 `operator==` 与 `operator!=`，测试可以直接以 `QCOMPARE` 比较条目。

## 31. 与类型相关的字段没有体现在模型中（已定）

`shortcuts` 只对 Action 解析，`category` 只对 Action 读取，而 `ActionItemInfo` 对所有类型都提供这些访问函数，非 Action 条目返回空值。这一约束依靠约定，而不是类型。

**决定：** 保持一个类。只对某些类型有意义的访问函数（`shortcuts()`、`category()`、external 的查询、`topLevel()`、`children()`）在接口注释中写明适用的类型，以及其他类型返回的空值。第 21 条之后 AEC 拒绝将这些属性写在不适用的标签上，运行时不会出现被静默忽略的值。

## 43. 身份与形态的不变量在持久化边界失效（已定）

AEC 保证 action 不能作为 menu 出现，但 `ActionLayouts::fromJsonObject()` 接受任意的类型字符串，`correctLayouts()` 也不做任何类型检查。用户保存的布局文件被手工修改、被旧版本写坏，或者扩展升级后某个标识的身份改变，都可能产生 `{"id": "core.openFile", "type": "Menu"}` 这样的条目。

结果不是报错，而是静默的降级：Widgets 后端调用 `menuForId()`，在 `items` 与 `autoItems` 中都找不到该标识，于是 `createSubMenu()` 创建一个标题为 Open File 的空子菜单，原来的 action 消失。Quick 后端的 `createMenu()` 同理。

registry 同时持有条目表与布局，是唯一能够进行这一检查的地方，应当在 `correctLayouts()` 中按第 17 条的组合表丢弃身份与形态不相容的条目并输出警告。本条须与第 17 条同时完成，否则 AEC 的检查只在形式上成立。

**决定。** 第 19 条之后，默认布局完全由清单算出，AEC 已保证其中身份与形态相容。不一致只可能来自用户的改动记录：保存文件被修改或损坏，扩展更新后某个标识的身份改变，或者记录引用的条目与容器已经不存在。registry 回放改动记录时逐条检查，参照 IntelliJ 平台 `CustomizationUtil` 的宽松做法，不满足条件的记录跳过，其余记录照常回放：

- 记录所在的容器不存在，或其身份不是 Menu 或 Group：跳过。
- 记录中的条目没有任何已登记的扩展声明（分隔符与 stretch 除外）：跳过。IntelliJ 平台在 `getComponentAction()` 返回空时同样跳过。
- 条目的形态与第 17 条的组合表不相容：跳过。IntelliJ 平台没有身份与形态的区分，本项为 QActionKit 所增加。
- 类型字符串无法识别：跳过，不再像 `actionLayoutEntryFromJson()` 那样默认为 Action。
- 锚点不存在：放到容器末尾（第 19 条）。

每条被跳过的记录以 `qCWarning` 报告，IntelliJ 平台在此处不输出任何信息。结果是菜单总是有效的，最坏情况是用户的某项自定义失效。

## 44. `ActionLayoutsModel` 无法检查身份与形态是否相容

`validateActionLayoutEntry()` 只检查「分隔符与 stretch 当且仅当标识为空」，`validateEntryChange()` 只多检查一个环。模型持有的是邻接表与 `hashList`，不持有 registry 或条目表，因而无法得知某个标识的身份。该模型用于设置页面中的拖放编辑：用户将一个 action 拖为子菜单时，模型照样接受，写出第 43 条所述的错误布局。应当为模型提供 `ActionRegistry *`（或条目信息的查询回调），或者规定模型只负责结构编辑，由 registry 在 `setLayouts()` 时检查。

## 45. Widgets 后端重建容器时清除其全部内容（已定）

`WidgetActionContextPrivate::updateLayouts()` 对每个登记的容器（菜单、菜单栏、工具栏）先移除全部动作，再按布局重新填充。应用程序自行加入容器的动作因此在每次更新布局时被清除。

**决定：维持现状。** 登记给 context 的容器，其内容一律由 QActionKit 管理，应用程序不得自行向其中加入动作。`WidgetActionContext` 的接口文档写明这一约定。内容由应用程序维护的菜单（如最近打开的文件）声明为 external action，以 `addAction()` 登记其 `menuAction()`，不经 `addMenu()` 登记，因此不受本条影响，见第 17 条。

## 46. `menuBar` 与 `toolBar` 编译后没有区别（已定）

**决定：** 随第 17 条删除 `menuBar` 与 `toolBar` 标签。顶层容器一律写作 `<menu topLevel="true">`，菜单栏与工具栏的区别由应用程序登记时决定。以下为原问题。

`parseItemAttrs()` 对这两个标签都设置 `type = Menu; topLevel = true;`。`info.tag` 只存在于解析器的中间结构中，`generator.cpp` 不输出它，因此 `ActionItemInfo` 无法区分弹出菜单、菜单栏与工具栏，应用程序只能硬编码哪个标识是工具栏。mainwindow 示例即是如此（`addMenuBar("core.mainMenu")`、`addToolBar("core.mainToolBar")`），而清单中两者都写作 `<menu topLevel="true">`。按照「声明类型决定身份」的原则，此处身份定义不足：标签提供了三个词，编译结果只有一种。应当为 `ActionItemInfo` 增加顶层种类（弹出菜单、菜单栏、工具栏），或者规定三个标签为同义词并写入规范。

## 47. `if` 跳过声明后，布局中的引用会静默重建一个降级的条目（随第 21 条解决）

第 21 条取消隐式声明之后，`if` 跳过了声明而引用处没有 `if` 时，引用的是一个未声明的标识，AEC 报错，提示引用处也须加上 `if`。以下为原问题。

`parse()` 对被 `if` 跳过的 `<items>` 元素直接跳过，该元素不进入 `itemInfoMap`。随后布局中的引用经过 `findOrInsertItemInfo()` 的 else 分支，以引用处的标签与属性创建一个新条目。

例如声明为 `<action id="c.debugDump" text="Dump State" category="Debug" shortcut="Ctrl+D" description="dumps" if="ENABLE_DEBUG" />`，布局中只写 `<action id="c.debugDump" />`。关闭开关后，编译结果的文本为 Debug Dump，类别、描述与快捷键为空。条目没有消失，只是全部元数据被降级，而且身份改由引用处的标签决定，违反了「声明类型决定身份」的原则。

应当在 `if` 跳过声明时记录该标识，此后的引用一律报错（提示引用处也须加上 `if`）；或者规定 `if` 只从布局中移除条目，保留其声明。

## 48. 获取扩展依赖宏与隐式的命名（已定）

**现状。** AEC 在生成的源文件中定义 `qakGetStaticActionExtension_<标识符>()`，但不生成它的声明。`QAK_STATIC_ACTION_EXTENSION(name)` 展开为立即调用的 lambda，在块作用域中声明该函数。块作用域中的 `extern` 声明属于外围的命名空间，因此该宏只能在全局命名空间中使用，调用方须在全局命名空间中另写一个包装函数。标识符默认取清单的文件名，C++ 代码与 CMake 中的文件名之间因此存在隐式的约定，名称写错时报告的是链接错误而不是编译错误。生成代码中的 `QT_MANGLE_NAMESPACE` 用于 Qt 自身编译在命名空间中的情形，不适用于应用程序的生成代码。

**决定。** AEC 同时生成一个头文件，声明获取扩展的函数。选项的命名参照 `qt_add_qml_module()` 中 qmltc 的 `QMLTC_EXPORT_DIRECTIVE` 与 `QMLTC_EXPORT_FILE_NAME`：

```cmake
qak_add_action_extension(_src core_actions.xml
    NAMESPACE hello::daw
    FUNCTION coreActions
    EXPORT_DIRECTIVE HELLOUTAU_WIDGETS_EXPORT
    EXPORT_FILE_NAME helloutau/Widgets/HelloUtauWidgetsGlobal.h
)
```

生成的头文件：

```cpp
#include <QAKCore/actionextension.h>

#include <helloutau/Widgets/HelloUtauWidgetsGlobal.h>

namespace hello::daw {
    HELLOUTAU_WIDGETS_EXPORT const QAK::ActionExtension *coreActions();
}
```

- `FUNCTION` 必需，`NAMESPACE` 可选。函数返回 `const QAK::ActionExtension *`，与 `ActionRegistry::addExtension()` 的参数一致。
- `EXPORT_DIRECTIVE` 是插入在函数声明之前的宏名，用于从动态库中导出或隐藏该函数。宏在构建与使用时分别展开为什么，由定义它的头文件决定。`EXPORT_FILE_NAME` 是定义该宏的头文件。只给出 `EXPORT_FILE_NAME` 而没有 `EXPORT_DIRECTIVE` 时报错，两者都省略时不加宏。
- 头文件名为 `<清单文件名>.qak.h`，位于生成目录中，该目录加入调用方的包含路径。生成的源文件包含该头文件，使定义处可以看到带有宏的声明。
- 删除 `QAK_STATIC_ACTION_EXTENSION` 宏，以及生成代码中的 `QT_MANGLE_NAMESPACE`。
- 删除 `IDENTIFIER` 选项与 AEC 的 `-i` 选项。标识符原本用于区分同名清单的获取函数，该作用由 `FUNCTION` 与 `NAMESPACE` 承担。生成代码内部的命名空间改为匿名命名空间，只供 lupdate 扫描的翻译声明函数使用固定的名字。保留变量 `_IDENTIFIER_` 一并删除。
