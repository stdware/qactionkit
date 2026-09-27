# 待办事项

设计问题及其决定见 [design-issues.md](design-issues.md)。

## 缺少的功能

- **设置界面。** `src/widgets/settings/keymapsettingswidget.cpp` 与 `layoutssettingswidget.cpp` 是空文件。作为其数据来源的 `ActionCatalogModel` 与 `ActionLayoutsModel` 已经实现。
- **图标主题。** 图标清单按主题声明图标，`ActionFamily` 也按主题存储，但两种后端一律使用默认主题，见 `quickactioninstantiatorattachedtype.cpp` 与 `widgetactioncontext.cpp` 中的 `// TODO: theme`。上下文需要记录当前主题，并在主题改变时重建图标。
- **布局中的控件工厂。** `WidgetActionContext::addWidgetFactory()` 登记工厂后，构建出的布局会放置其动作，但清单中无法声明「此处放置一个控件」，该标识只能声明为 action。

## 测试覆盖

- **`ActionCatalogModel` 没有测试。**
- **没有持续集成。** 仓库中没有 `.github/`，推送时不运行测试，也不在 Linux 或 macOS 上构建。

