# QActionKit

Declarative menu and action framework for Qt.

QActionKit moves the menu structure of an application from C++ into XML manifests. A plugin contributes items to menus that it does not own, and the user reorders the result and assigns other shortcuts. The same description serves a QtWidgets and a QtQuick front end.

The manifests are compiled to static C++ data at build time, and no manifest is parsed at startup.

## Architecture

```
manifest.xml ──qak_aec──> qak_manifest.cpp (static data)
                                │
                                ▼
                         ActionRegistry (application scope)
                    catalog · layouts · keymap · icons
                                │
                    ┌───────────┴───────────┐
            WidgetActionContext      QuickActionContext   (window scope)
```

- **`ActionExtension`**: one compiled manifest. It lists the items that a component declares, the layout of the menus that the component owns, and the items that it inserts into menus owned by other components.
- **`ActionRegistry`**: merges every extension into a *catalog*, the logical tree presented in a settings page, and into *layouts*, the graph from which the menus are built. It also holds the shortcut and icon overrides of the user.
- **`ActionContext`**: builds the objects of one window. `WidgetActionContext` populates `QMenuBar`, `QMenu` and `QToolBar`, and `QuickActionContext` instantiates QML components.

A layout customized by the user is stored with the hash of every extension from which it was built. When a plugin is added or updated, its new items are merged into the customized layout, which is not discarded.

## Example

```xml
<!-- core-actions.xml -->
<actionExtension>
    <version>1.0</version>
    <id>com.example.application</id>
    <items>
        <action id="core.openFile" text="Open File" shortcut="Ctrl+O" />
        <menu id="core.mainMenu" topLevel="true" />
    </items>
    <layouts>
        <menu id="core.mainMenu">
            <menu id="core.file">
                <action id="core.openFile" />
            </menu>
        </menu>
    </layouts>
</actionExtension>
```

```cmake
qak_add_action_extension(_core_src "core-actions.xml")
target_sources(MyApp PRIVATE ${_core_src})
target_link_libraries(MyApp PRIVATE QActionKit::Widgets)
```

```cpp
// Declared in the global namespace, as QAK_STATIC_ACTION_EXTENSION requires
static auto coreActions() {
    return QAK_STATIC_ACTION_EXTENSION(core_actions);
}

auto registry = new QAK::ActionRegistry(this);
registry->setExtensions({coreActions()});

auto context = new QAK::WidgetActionContext(this);
registry->addContext(context);
context->addMenuBar(QStringLiteral("core.mainMenu"), menuBar());

registry->updateContext(QAK::AE_Layouts);
```

After the last call, the menu bar holds the menus of the manifest. `WidgetActionContext::actionTriggered()` reports each invocation. Alternatively, an application registers its own `QAction` for an id with `addAction()`, and QActionKit places the action and keeps its text, icon and shortcut up to date.

The manifest format is specified in [docs/action-extension-spec.md](docs/action-extension-spec.md). Runnable examples are in [examples/](examples/).

## Building

Requirements:

- Qt 6, with Widgets and Quick for the corresponding modules
- CMake 3.19 or newer
- [qmsetup](https://github.com/stdware/qmsetup), which is not vendored and must be installed beforehand

```bash
cmake -S . -B build -G Ninja \
    -DCMAKE_PREFIX_PATH=<qt-prefix> \
    -Dqmsetup_DIR=<qmsetup-prefix>/lib/cmake/qmsetup
cmake --build build
cmake --install build
```

## License

QActionKit is licensed under the [Apache 2.0 License](./LICENSE).
