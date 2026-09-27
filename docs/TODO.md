# 待办事项

尚未解决的设计问题见 [design-issues.md](design-issues.md)。

## 缺少的功能

- **设置界面。** `src/widgets/settings/keymapsettingswidget.cpp` 与 `layoutssettingswidget.cpp` 是空文件。作为其数据来源的 `ActionCatalogModel` 与 `ActionLayoutsModel` 已经实现。
- **图标主题。** 图标清单按主题声明图标，`ActionFamily` 也按主题存储，但两种后端一律使用默认主题，见 `quickactioninstantiatorattachedtype.cpp` 与 `widgetactioncontext.cpp` 中的 `// TODO: theme`。上下文需要记录当前主题，并在主题改变时重建图标。
- **布局中的控件工厂。** `WidgetActionContext::addWidgetFactory()` 登记工厂后，构建出的布局会放置其动作，但清单中无法声明「此处放置一个控件」，该标识只能声明为 action。

## 测试覆盖

- **`ActionLayoutsModel` 没有测试。** 共 715 行，提供 `setData`、`insertRows`、`removeRows`、`moveRows` 等编辑接口，以及 `wouldCreateCycle()` 中的环检测，均未经测试。
- **AEC 的编译结果没有测试。** `tests/auto/tools/aec` 测试 AEC 接受与拒绝哪些清单，但不检查生成的数据。应当将清单编入测试程序，通过 `ActionRegistry` 检查条目、布局与插入。
- **`ActionCatalogModel` 没有测试。**
- **没有持续集成。** 仓库中没有 `.github/`，推送时不运行测试，也不在 Linux 或 macOS 上构建。

## 性能

- **`ActionLayoutsModelPrivate::cachePathId()` 线性扫描整个路径缓存**，每次调用 `index()` 都执行一次，模型的开销因此与可见节点数的平方成正比。缓存只增不减，没有淘汰。以路径到标识的散列表代替即可同时解决两个问题。
- **`ActionLayoutsModelPrivate::rebuildCache()` 枚举有向无环图中的全部路径。** 被多个父节点共用的菜单使路径数成倍增加，含有共用子菜单的大型布局可能使路径数急剧膨胀。树视图需要路径，但应在展开节点时按需生成。

## 代码整理

- `ActionRegistryPrivate::defaultLayouts()` 与 `correctLayouts()` 的函数体大部分重复。文档称 `defaultLayouts()` 等价于 `correctLayouts(ActionLayouts())`，实际上是另一套实现。按设计问题第 19 条的决定，`correctLayouts()` 将被重放用户改动记录取代，本条随之消失。
- `parser.cpp` 中的 `simplifyActionText()` 与 `isStringDigits()` 未被使用。
- `ActionFamily::actionIcon(theme, id)` 以同一个字符串同时查找用户覆盖与主题图标，前者需要动作标识，后者需要图标标识。调用方传入 `info.icon()`，因此条目的图标标识与其自身标识不同时，覆盖查找的结果错误。
