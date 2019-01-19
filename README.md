# 如何使用模板

## 使用

### 创建图表

![创建图表](./Docs/Images/%E5%88%9B%E5%BB%BA%E5%9B%BE%E8%A1%A8.png)

### 使用

![节点效果](./Docs/Images/%E8%8A%82%E7%82%B9%E6%95%88%E6%9E%9C.png)

### 使用该模板

将所有带_Template的类型改名成需要的类型名就行。

# 制作虚幻图表编辑器

## 继承类型

### UEdGraphSchema

1. EdGraphSchema里定义了图表操作的大部分全局行为
2. 在FBlueprintEditorUtils::CreateNewGraph中将UEdGraph和EdGraphSchema建立映射

### UEdGraph

EdGraph是图表实例的类型，定义了该图表的行为（例如保存图表

### UEdGraphNode

1. EdGraphNode是图表节点实例的类型，定义了节点的行为
2. AutowireNewNode定义了节点的自动连接行为

### FAssetEditorToolkit

1. 可在初始化时调用FAssetEditorToolkit::InitAssetEditor定义面板的布局。（复杂情况参考FApplicationMode）
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

## 节点错误报告

UEdGraphNode中设置ErrorMsg、bHasCompilerMessage、ErrorType（EMessageSeverity::Type）

SGraphNode在节点需要更新错误信息的地方调用UpdateErrorInfo

## 添加编辑器面板

### FWorkflowAllowedTabSet

1. 面板的集合
2. 调用FWorkflowAllowedTabSet::RegisterFactory注册每个面板实例化的定义
3. 调用FWorkflowCentricApplication::PushTabFactories（FBlueprintEditor的一个基类）注册FWorkflowAllowedTabSet

### FWorkflowTabFactory

1. 面板实例化定义的基类
2. 继承并重写FWorkflowTabFactory::CreateTabBody实现特定面板的实例化
3. 向FWorkflowAllowedTabSet的实例中注册该面板实例化定义（FWorkflowAllowedTabSet::RegisterFactory）

## 其它工具类

### FGraphEditor_ClassHelper_Template

GatherClasses 搜集UClass信息

构造函数中FGraphEditor_ClassHelper_Template(UClass* InRootClass); InRootClass表示搜集的根

### FGraphEditorToolkit_Template::HandleTabManagerSpawnTabGraph

创建了图表控件

# 拓展蓝图编辑器

## 效果

![蓝图设计图表](./Docs/Images/%E8%93%9D%E5%9B%BE%E8%AE%BE%E8%AE%A1%E5%9B%BE%E8%A1%A8.png)

![蓝图编码图表](./Docs/Images/%E8%93%9D%E5%9B%BE%E7%BC%96%E7%A0%81%E5%9B%BE%E8%A1%A8.png)

## 继承类型

### UBlueprint

1. 自定义蓝图的资源类型
2. 调用FKismetEditorUtilities::CreateBlueprint创建蓝图资源
3. 重写GetReparentingRules定义该蓝图类型可使用的父类

### UBlueprintGeneratedClass

1. 自定义蓝图的Runtime类型

### FApplicationMode

1. 定义了App的模式
2. 定义了初始的界面布局
3. **FBlueprintEditor::RegisterApplicationModes**中重写以映射操作
4. 调用FWorkflowCentricApplication::PushTabFactories（FBlueprintEditor的一个基类）设置FWorkflowAllowedTabSet（所有面板的定义）
5. FApplicationMode::ToolbarExtender为当前的工具栏，通过FBlueprintEditor::GetToolbarBuilder()->AddCompileToolbar(ToolbarExtender)向工具栏中添加功能，也可调用FExtender::AddToolBarExtension添加自定义的功能

### FKismetCompilerContext

1. 蓝图编译的过程定义
2. 在模块初始化时注册该编译器，FKismetCompilerContext::RegisterCompilerForBP
3. 重写FinishCompilingClass在蓝图编译后进行图表的拷贝、绑定事件的添加等操作
4. 重写CreateClassVariablesFromBlueprint用于设计图表的变量暴露，比FBlueprintEditorUtils::AddMemberVariable的好处为变量在蓝图中不能编辑

## S类型

### SGraphEditor

图表编辑器的S类型

`SNew(SGraphEditor).AdditionalCommands(Commands).GraphToEdit(EdGraph);`

## 运行时绑定事件

### PropertyEditor拓展类型

1. IDetailPropertyExtensionHandler 每个属性的拓展
2. IDetailCustomization 拓展整个Detail面板

### 添加绑定事件

1. FKismetEditorUtilities::FindBoundEventForComponent
2. FKismetEditorUtilities::CreateNewBoundEventForClass

### UDynamicBlueprintBinding类型

1. 蓝图绑定事件的基类，子类UComponentDelegateBinding就是对象绑定
2. UBlueprintGeneratedClass::DynamicBindingObjects中存储了运行时可绑定的事件
3. UDynamicBlueprintBinding::BindDynamicDelegates执行运行时绑定

### 自定义函数绑定

1. 参考UMG模块中的FDelegateEditorBinding与FDelegateRuntimeBinding
2. 自己的蓝图类中定义EditorBinding，GeneratedClass类型中定义RuntimeBinding
3. 自己通过反射写运行时绑定的操作

