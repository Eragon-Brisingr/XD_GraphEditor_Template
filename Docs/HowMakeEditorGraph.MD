# 如何使用模板
## 使用
### 创建图表

![创建图表](.\Images\创建图表.png)

### 使用

![节点效果](.\Images\节点效果.png)

### 使用该模板

将所有带_Template的类型改名成需要的类型名就行。

# 制作虚幻图表编辑器

## 继承类型

### UEdGraphSchema

EdGraphSchema里定义了图表操作的大部分全局行为

### UEdGraph

EdGraph是图表实例的类型，定义了该图表的行为（例如保存图表

### UEdGraphNode

1. EdGraphNode是图表节点实例的类型，定义了节点的行为
2. AutowireNewNode定义了节点的自动连接行为

### FAssetEditorToolkit

1. 定义编辑器面板布局，添加Tab页面等操作。

2. 通过向FUICommandList的变量中注册事件（复制、粘贴、拷贝、删除等等）定义基础编辑器操作

### 节点行为类型

1. FConnectionDrawingPolicy
2. FEdGraphSchemaAction

### 显示类型

继承S类型实现显示行为

#### SGraphNode

节点的显示定义

通过UEdGraphNode::CreateVisualWidget将SNode类型和UNode类型映射上

#### SGraphPin

Pin的显示定义（蓝图中就是参数之类可以连线的地方）

通过SGraphNode::CreatePinWidgets将SPin与SNode映射上

### 资源行为类型

#### FAssetTypeActions_Base

1. 定义创建资源时的分类 GetCategories
2. 定义了资源被请求打开时的操作 OpenAssetEditor，将FAssetEditorToolkit与运行时使用的资源（UBP_Graph_Template）映射上

#### UFactory

工厂类，编辑器用于实例化图表资源

## 添加右键菜单

### 图表右键菜单

重写UEdGraphSchema中的GetGraphContextActions

### 节点右键菜单

重写UEdGraphNode中的GetContextMenuActions

重写UEdGraphSchema中的GetContextMenuActions给全局的Node或者Pin添加行为

## 错误报告
UEdGraphNode中设置ErrorMsg、bHasCompilerMessage、ErrorType（EMessageSeverity::Type）

SGraphNode在节点需要更新错误信息的地方调用UpdateErrorInfo

## 其它工具类

### FGraphEditor_ClassHelper_Template

GatherClasses 搜集UClass信息

构造函数中FGraphEditor_ClassHelper_Template(UClass* InRootClass); InRootClass表示搜集的根
