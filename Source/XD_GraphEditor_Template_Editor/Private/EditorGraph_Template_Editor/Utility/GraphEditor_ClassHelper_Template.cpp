// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/Utility/GraphEditor_ClassHelper_Template.h"
#include "UObject/Class.h"
#include "Misc/FeedbackContext.h"
#include "UObject/Package.h"
#include "Engine/Blueprint.h"
#include "AssetRegistryModule.h"
#include "Misc/HotReloadInterface.h"
#include "Editor.h"
#include "UObject/ConstructorHelpers.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"

#define LOCTEXT_NAMESPACE "GraphEditor_ClassHelper_Template"


FGraphEditor_ClassData_Template::FGraphEditor_ClassData_Template(UClass* InClass, const FString& InDeprecatedMessage) :
	bIsHidden(0),
	bHideParent(0),
	Class(InClass),
	DeprecatedMessage(InDeprecatedMessage)
{
	Category = GetCategory();

	if (InClass)
	{
		ClassName = InClass->GetName();
	}
}

FGraphEditor_ClassData_Template::FGraphEditor_ClassData_Template(const FString& InAssetName, const FString& InGeneratedClassPackage, const FString& InClassName, UClass* InClass) :
	bIsHidden(0),
	bHideParent(0),
	Class(InClass),
	AssetName(InAssetName),
	GeneratedClassPackage(InGeneratedClassPackage),
	ClassName(InClassName)
{
	Category = GetCategory();
}

FString FGraphEditor_ClassData_Template::ToString() const
{
	FString ShortName = GetDisplayName();
	if (!ShortName.IsEmpty())
	{
		return ShortName;
	}

	UClass* MyClass = Class.Get();
	if (MyClass)
	{
		FString ClassDesc = MyClass->GetName();

		if (MyClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
		{
			return ClassDesc.LeftChop(2);
		}

		const int32 ShortNameIdx = ClassDesc.Find(TEXT("_"));
		if (ShortNameIdx != INDEX_NONE)
		{
			ClassDesc = ClassDesc.Mid(ShortNameIdx + 1);
		}

		return ClassDesc;
	}

	return AssetName;
}

FString FGraphEditor_ClassData_Template::GetClassName() const
{
	return Class.IsValid() ? Class->GetName() : ClassName;
}

FString FGraphEditor_ClassData_Template::GetDisplayName() const
{
	return Class.IsValid() ? Class->GetMetaData(TEXT("DisplayName")) : FString();
}

FText FGraphEditor_ClassData_Template::GetCategory() const
{
	return Class.IsValid() ? Class->GetMetaDataText(TEXT("Category"), TEXT("UObjectCategory"), Class->GetFullGroupName(false)) : Category;
}

bool FGraphEditor_ClassData_Template::IsAbstract() const
{
	return Class.IsValid() ? Class.Get()->HasAnyClassFlags(CLASS_Abstract) : false;
}

UClass* FGraphEditor_ClassData_Template::GetClass(bool bSilent)
{
	UClass* RetClass = Class.Get();
	if (RetClass == NULL && GeneratedClassPackage.Len())
	{
		GWarn->BeginSlowTask(LOCTEXT("LoadPackage", "Loading Package..."), true);

		UPackage* Package = LoadPackage(NULL, *GeneratedClassPackage, LOAD_NoRedirects);
		if (Package)
		{
			Package->FullyLoad();

			UObject* Object = FindObject<UObject>(Package, *AssetName);

			GWarn->EndSlowTask();

			UBlueprint* BlueprintOb = Cast<UBlueprint>(Object);
			RetClass = BlueprintOb ? *BlueprintOb->GeneratedClass :
				Object ? Object->GetClass() :
				NULL;

			Class = RetClass;
		}
	}

	return RetClass;
}

//////////////////////////////////////////////////////////////////////////
TArray<FName> FGraphEditor_ClassHelper_Template::UnknownPackages;
TMap<UClass*, int32> FGraphEditor_ClassHelper_Template::BlueprintClassCount;
FGraphEditor_ClassHelper_Template::FOnPackageListUpdated FGraphEditor_ClassHelper_Template::OnPackageListUpdated;

FGraphEditor_ClassHelper_Template::FGraphEditor_ClassHelper_Template(UClass* InRootClass)
{
	RootNodeClass = InRootClass;
	// Register with the Asset Registry to be informed when it is done loading up files.
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnFilesLoaded().AddRaw(this, &FGraphEditor_ClassHelper_Template::InvalidateCache);
	AssetRegistryModule.Get().OnAssetAdded().AddRaw(this, &FGraphEditor_ClassHelper_Template::OnAssetAdded);
	AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &FGraphEditor_ClassHelper_Template::OnAssetRemoved);

	// Register to have Populate called when doing a Hot Reload.
	IHotReloadInterface& HotReloadSupport = FModuleManager::LoadModuleChecked<IHotReloadInterface>("HotReload");
	HotReloadSupport.OnHotReload().AddRaw(this, &FGraphEditor_ClassHelper_Template::OnHotReload);

	// Register to have Populate called when a Blueprint is compiled.
	GEditor->OnBlueprintCompiled().AddRaw(this, &FGraphEditor_ClassHelper_Template::InvalidateCache);
	GEditor->OnClassPackageLoadedOrUnloaded().AddRaw(this, &FGraphEditor_ClassHelper_Template::InvalidateCache);

	UpdateAvailableBlueprintClasses();
}

FGraphEditor_ClassHelper_Template::~FGraphEditor_ClassHelper_Template()
{
	// Unregister with the Asset Registry to be informed when it is done loading up files.
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().OnFilesLoaded().RemoveAll(this);
		AssetRegistryModule.Get().OnAssetAdded().RemoveAll(this);
		AssetRegistryModule.Get().OnAssetRemoved().RemoveAll(this);

		// Unregister to have Populate called when doing a Hot Reload.
		if (FModuleManager::Get().IsModuleLoaded(TEXT("HotReload")))
		{
			IHotReloadInterface& HotReloadSupport = FModuleManager::GetModuleChecked<IHotReloadInterface>("HotReload");
			HotReloadSupport.OnHotReload().RemoveAll(this);
		}

		// Unregister to have Populate called when a Blueprint is compiled.
		if (UObjectInitialized())
		{
			// GEditor can't have been destructed before we call this or we'll crash.
			GEditor->OnBlueprintCompiled().RemoveAll(this);
			GEditor->OnClassPackageLoadedOrUnloaded().RemoveAll(this);
		}
	}
}

void FGraphEditor_ClassNode_Template::AddUniqueSubNode(TSharedPtr<FGraphEditor_ClassNode_Template> SubNode)
{
	for (int32 Idx = 0; Idx < SubNodes.Num(); Idx++)
	{
		if (SubNode->Data.GetClassName() == SubNodes[Idx]->Data.GetClassName())
		{
			return;
		}
	}

	SubNodes.Add(SubNode);
}

void FGraphEditor_ClassHelper_Template::GatherClasses(const UClass* BaseClass, TArray<FGraphEditor_ClassData_Template>& AvailableClasses)
{
	const FString BaseClassName = BaseClass->GetName();
	if (!RootNode.IsValid())
	{
		BuildClassGraph();
	}

	TSharedPtr<FGraphEditor_ClassNode_Template> BaseNode = FindBaseClassNode(RootNode, BaseClassName);
	FindAllSubClasses(BaseNode, AvailableClasses);
}

FString FGraphEditor_ClassHelper_Template::GetDeprecationMessage(const UClass* Class)
{
	static FName MetaDeprecated = TEXT("DeprecatedNode");
	static FName MetaDeprecatedMessage = TEXT("DeprecationMessage");
	FString DefDeprecatedMessage("Please remove it!");
	FString DeprecatedPrefix("DEPRECATED");
	FString DeprecatedMessage;

	if (Class && Class->HasAnyClassFlags(CLASS_Native) && Class->HasMetaData(MetaDeprecated))
	{
		DeprecatedMessage = DeprecatedPrefix + TEXT(": ");
		DeprecatedMessage += Class->HasMetaData(MetaDeprecatedMessage) ? Class->GetMetaData(MetaDeprecatedMessage) : DefDeprecatedMessage;
	}

	return DeprecatedMessage;
}

bool FGraphEditor_ClassHelper_Template::IsClassKnown(const FGraphEditor_ClassData_Template& ClassData)
{
	return !ClassData.IsBlueprint() || !UnknownPackages.Contains(*ClassData.GetPackageName());
}

void FGraphEditor_ClassHelper_Template::AddUnknownClass(const FGraphEditor_ClassData_Template& ClassData)
{
	if (ClassData.IsBlueprint())
	{
		UnknownPackages.AddUnique(*ClassData.GetPackageName());
	}
}

bool FGraphEditor_ClassHelper_Template::IsHidingParentClass(UClass* Class)
{
	static FName MetaHideParent = TEXT("HideParentNode");
	return Class && Class->HasAnyClassFlags(CLASS_Native) && Class->HasMetaData(MetaHideParent);
}

bool FGraphEditor_ClassHelper_Template::IsHidingClass(UClass* Class)
{
	static FName MetaHideInEditor = TEXT("HiddenNode");
	return Class && Class->HasAnyClassFlags(CLASS_Native) && Class->HasMetaData(MetaHideInEditor);
}

bool FGraphEditor_ClassHelper_Template::IsPackageSaved(FName PackageName)
{
	const bool bFound = FPackageName::SearchForPackageOnDisk(PackageName.ToString());
	return bFound;
}

void FGraphEditor_ClassHelper_Template::OnAssetAdded(const struct FAssetData& AssetData)
{
	TSharedPtr<FGraphEditor_ClassNode_Template> Node = CreateClassDataNode(AssetData);

	TSharedPtr<FGraphEditor_ClassNode_Template> ParentNode;
	if (Node.IsValid())
	{
		ParentNode = FindBaseClassNode(RootNode, Node->ParentClassName);

		if (!IsPackageSaved(AssetData.PackageName))
		{
			UnknownPackages.AddUnique(AssetData.PackageName);
		}
		else
		{
			const int32 PrevListCount = UnknownPackages.Num();
			UnknownPackages.RemoveSingleSwap(AssetData.PackageName);

			if (UnknownPackages.Num() != PrevListCount)
			{
				OnPackageListUpdated.Broadcast();
			}
		}
	}

	if (ParentNode.IsValid())
	{
		ParentNode->AddUniqueSubNode(Node);
		Node->ParentNode = ParentNode;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if (!AssetRegistryModule.Get().IsLoadingAssets())
	{
		UpdateAvailableBlueprintClasses();
	}
}

void FGraphEditor_ClassHelper_Template::OnAssetRemoved(const struct FAssetData& AssetData)
{
	FString AssetClassName;
	if (AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, AssetClassName))
	{
		ConstructorHelpers::StripObjectClass(AssetClassName);
		AssetClassName = FPackageName::ObjectPathToObjectName(AssetClassName);

		TSharedPtr<FGraphEditor_ClassNode_Template> Node = FindBaseClassNode(RootNode, AssetClassName);
		if (Node.IsValid() && Node->ParentNode.IsValid())
		{
			Node->ParentNode->SubNodes.RemoveSingleSwap(Node);
		}
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if (!AssetRegistryModule.Get().IsLoadingAssets())
	{
		UpdateAvailableBlueprintClasses();
	}
}

void FGraphEditor_ClassHelper_Template::InvalidateCache()
{
	RootNode.Reset();
	UpdateAvailableBlueprintClasses();
}

void FGraphEditor_ClassHelper_Template::OnHotReload(bool bWasTriggeredAutomatically)
{
	InvalidateCache();
}

TSharedPtr<FGraphEditor_ClassNode_Template> FGraphEditor_ClassHelper_Template::CreateClassDataNode(const struct FAssetData& AssetData)
{
	TSharedPtr<FGraphEditor_ClassNode_Template> Node;

	FString AssetClassName;
	FString AssetParentClassName;
	if (AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, AssetClassName) && AssetData.GetTagValue(FBlueprintTags::ParentClassPath, AssetParentClassName))
	{
		UObject* Outer1(NULL);
		ResolveName(Outer1, AssetClassName, false, false);

		UObject* Outer2(NULL);
		ResolveName(Outer2, AssetParentClassName, false, false);

		Node = MakeShareable(new FGraphEditor_ClassNode_Template);
		Node->ParentClassName = AssetParentClassName;

		UObject* AssetOb = AssetData.IsAssetLoaded() ? AssetData.GetAsset() : NULL;
		UBlueprint* AssetBP = Cast<UBlueprint>(AssetOb);
		UClass* AssetClass = AssetBP ? *AssetBP->GeneratedClass : AssetOb ? AssetOb->GetClass() : NULL;

		FGraphEditor_ClassData_Template NewData(AssetData.AssetName.ToString(), AssetData.PackageName.ToString(), AssetClassName, AssetClass);
		Node->Data = NewData;
	}

	return Node;
}

TSharedPtr<FGraphEditor_ClassNode_Template> FGraphEditor_ClassHelper_Template::FindBaseClassNode(TSharedPtr<FGraphEditor_ClassNode_Template> Node, const FString& ClassName)
{
	TSharedPtr<FGraphEditor_ClassNode_Template> RetNode;
	if (Node.IsValid())
	{
		if (Node->Data.GetClassName() == ClassName)
		{
			return Node;
		}

		for (int32 i = 0; i < Node->SubNodes.Num(); i++)
		{
			RetNode = FindBaseClassNode(Node->SubNodes[i], ClassName);
			if (RetNode.IsValid())
			{
				break;
			}
		}
	}

	return RetNode;
}

void FGraphEditor_ClassHelper_Template::FindAllSubClasses(TSharedPtr<FGraphEditor_ClassNode_Template> Node, TArray<FGraphEditor_ClassData_Template>& AvailableClasses)
{
	if (Node.IsValid())
	{
		if (!Node->Data.IsAbstract() && !Node->Data.IsDeprecated() && !Node->Data.bIsHidden)
		{
			AvailableClasses.Add(Node->Data);
		}

		for (int32 i = 0; i < Node->SubNodes.Num(); i++)
		{
			FindAllSubClasses(Node->SubNodes[i], AvailableClasses);
		}
	}
}

UClass* FGraphEditor_ClassHelper_Template::FindAssetClass(const FString& GeneratedClassPackage, const FString& AssetName)
{
	UPackage* Package = FindPackage(NULL, *GeneratedClassPackage);
	if (Package)
	{
		UObject* Object = FindObject<UObject>(Package, *AssetName);
		if (Object)
		{
			UBlueprint* BlueprintOb = Cast<UBlueprint>(Object);
			return BlueprintOb ? *BlueprintOb->GeneratedClass : Object->GetClass();
		}
	}

	return NULL;
}

void FGraphEditor_ClassHelper_Template::BuildClassGraph()
{
	TArray<TSharedPtr<FGraphEditor_ClassNode_Template> > NodeList;
	TArray<UClass*> HideParentList;
	RootNode.Reset();

	// gather all native classes
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* TestClass = *It;
		if (TestClass->HasAnyClassFlags(CLASS_Native) && TestClass->IsChildOf(RootNodeClass))
		{
			TSharedPtr<FGraphEditor_ClassNode_Template> NewNode = MakeShareable(new FGraphEditor_ClassNode_Template);
			NewNode->ParentClassName = TestClass->GetSuperClass()->GetName();

			FString DeprecatedMessage = GetDeprecationMessage(TestClass);
			FGraphEditor_ClassData_Template NewData(TestClass, DeprecatedMessage);

			NewData.bHideParent = IsHidingParentClass(TestClass);
			if (NewData.bHideParent)
			{
				HideParentList.Add(TestClass->GetSuperClass());
			}

			NewData.bIsHidden = IsHidingClass(TestClass);

			NewNode->Data = NewData;

			if (TestClass == RootNodeClass)
			{
				RootNode = NewNode;
			}

			NodeList.Add(NewNode);
		}
	}

	// find all hidden parent classes
	for (int32 i = 0; i < NodeList.Num(); i++)
	{
		TSharedPtr<FGraphEditor_ClassNode_Template> TestNode = NodeList[i];
		if (HideParentList.Contains(TestNode->Data.GetClass()))
		{
			TestNode->Data.bIsHidden = true;
		}
	}

	// gather all blueprints
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> BlueprintList;

	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);

	for (int32 i = 0; i < BlueprintList.Num(); i++)
	{
		TSharedPtr<FGraphEditor_ClassNode_Template> NewNode = CreateClassDataNode(BlueprintList[i]);
		NodeList.Add(NewNode);
	}

	// build class tree
	AddClassGraphChildren(RootNode, NodeList);
}

void FGraphEditor_ClassHelper_Template::AddClassGraphChildren(TSharedPtr<FGraphEditor_ClassNode_Template> Node, TArray<TSharedPtr<FGraphEditor_ClassNode_Template> >& NodeList)
{
	if (!Node.IsValid())
	{
		return;
	}

	const FString NodeClassName = Node->Data.GetClassName();
	for (int32 i = NodeList.Num() - 1; i >= 0; i--)
	{
		if (NodeList[i]->ParentClassName == NodeClassName)
		{
			TSharedPtr<FGraphEditor_ClassNode_Template> MatchingNode = NodeList[i];
			NodeList.RemoveAt(i);

			MatchingNode->ParentNode = Node;
			Node->SubNodes.Add(MatchingNode);

			AddClassGraphChildren(MatchingNode, NodeList);
		}
	}
}

int32 FGraphEditor_ClassHelper_Template::GetObservedBlueprintClassCount(UClass* BaseNativeClass)
{
	return BlueprintClassCount.FindRef(BaseNativeClass);
}

void FGraphEditor_ClassHelper_Template::AddObservedBlueprintClasses(UClass* BaseNativeClass)
{
	BlueprintClassCount.Add(BaseNativeClass, 0);
}

void FGraphEditor_ClassHelper_Template::UpdateAvailableBlueprintClasses()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		const bool bSearchSubClasses = true;

		TArray<FName> ClassNames;
		TSet<FName> DerivedClassNames;

		for (TMap<UClass*, int32>::TIterator It(BlueprintClassCount); It; ++It)
		{
			ClassNames.Reset();
			ClassNames.Add(It.Key()->GetFName());

			DerivedClassNames.Empty(DerivedClassNames.Num());
			AssetRegistryModule.Get().GetDerivedClassNames(ClassNames, TSet<FName>(), DerivedClassNames);

			int32& Count = It.Value();
			Count = DerivedClassNames.Num();
		}
	}
}

#undef LOCTEXT_NAMESPACE