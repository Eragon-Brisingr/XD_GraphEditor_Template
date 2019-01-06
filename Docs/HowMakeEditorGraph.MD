# 制作虚幻图表编辑器

## 继承类型

### UEdGraphSchema

### UEdGraph

### UEdGraphNode

### FAssetEditorToolkit

1. 定义编辑器面板布局，添加Tab页面等操作。

2. 通过向FUICommandList的变量中注册事件（复制、粘贴、拷贝、删除等等）定义基础编辑器操作

### 节点行为类型

1. FConnectionDrawingPolicy
2. FEdGraphSchemaAction

### 显示类型

1. SGraphNode
2. SGraphPin

### 资源行为类型

1. FAssetTypeActions_Base
2. UFactory



## 添加右键菜单

### 图表右键菜单

重写UEdGraphSchema中的GetGraphContextActions

### 节点右键菜单

重写UEdGraphNode中的GetContextMenuActions

重写UEdGraphSchema中的GetContextMenuActions给全局的Node或者Pin添加行为

## 错误报告
UEdGraphNode中设置ErrorMsg、bHasCompilerMessage、ErrorType（EMessageSeverity::Type）

SGraphNode在节点需要更新错误信息的地方调用UpdateErrorInfo

## FGraphEditor_ClassHelper_Template
GatherClasses 搜集UClass信息

构造函数中FGraphEditor_ClassHelper_Template(UClass* InRootClass); InRootClass表示搜集的根