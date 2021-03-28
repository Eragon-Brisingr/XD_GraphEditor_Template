// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "GraphEditor_ClassHelper_Template.generated.h"

/**
 * 
 */

USTRUCT()
struct FGraphEditor_ClassData_Template
{
	GENERATED_USTRUCT_BODY()

	FGraphEditor_ClassData_Template() {}
	FGraphEditor_ClassData_Template(UClass* InClass, const FString& InDeprecatedMessage);
	FGraphEditor_ClassData_Template(const FString& InAssetName, const FString& InGeneratedClassPackage, const FString& InClassName, UClass* InClass);

	FString ToString() const;
	FString GetClassName() const;
	FText GetCategory() const;
	FString GetDisplayName() const;
	UClass* GetClass(bool bSilent = false);
	bool IsAbstract() const;

	FORCEINLINE bool IsBlueprint() const { return AssetName.Len() > 0; }
	FORCEINLINE bool IsDeprecated() const { return DeprecatedMessage.Len() > 0; }
	FORCEINLINE FString GetDeprecatedMessage() const { return DeprecatedMessage; }
	FORCEINLINE FString GetPackageName() const { return GeneratedClassPackage; }

	/** set when child class masked this one out (e.g. always use game specific class instead of engine one) */
	uint32 bIsHidden : 1;

	/** set when class wants to hide parent class from selection (just one class up hierarchy) */
	uint32 bHideParent : 1;

private:

	/** pointer to uclass */
	TWeakObjectPtr<UClass> Class;

	/** path to class if it's not loaded yet */
	UPROPERTY()
	FString AssetName;

	UPROPERTY()
	FString GeneratedClassPackage;

	/** resolved name of class from asset data */
	UPROPERTY()
	FString ClassName;

	/** User-defined category for this class */
	UPROPERTY()
	FText Category;

	/** message for deprecated class */
	FString DeprecatedMessage;
};

struct FGraphEditor_ClassNode_Template
{
	FGraphEditor_ClassData_Template Data;
	FString ParentClassName;

	TSharedPtr<FGraphEditor_ClassNode_Template> ParentNode;
	TArray<TSharedPtr<FGraphEditor_ClassNode_Template> > SubNodes;

	void AddUniqueSubNode(TSharedPtr<FGraphEditor_ClassNode_Template> SubNode);
};

struct FGraphEditor_ClassHelper_Template
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnPackageListUpdated);

	FGraphEditor_ClassHelper_Template(UClass* InRootClass);
	~FGraphEditor_ClassHelper_Template();

	void GatherClasses(const UClass* BaseClass, TArray<FGraphEditor_ClassData_Template>& AvailableClasses);
	static FString GetDeprecationMessage(const UClass* Class);

	void OnAssetAdded(const struct FAssetData& AssetData);
	void OnAssetRemoved(const struct FAssetData& AssetData);
	void InvalidateCache();
	void OnHotReload(bool bWasTriggeredAutomatically);

	static void AddUnknownClass(const FGraphEditor_ClassData_Template& ClassData);
	static bool IsClassKnown(const FGraphEditor_ClassData_Template& ClassData);
	static FOnPackageListUpdated OnPackageListUpdated;

	static int32 GetObservedBlueprintClassCount(UClass* BaseNativeClass);
	static void AddObservedBlueprintClasses(UClass* BaseNativeClass);
	void UpdateAvailableBlueprintClasses();

private:

	UClass* RootNodeClass;
	TSharedPtr<FGraphEditor_ClassNode_Template> RootNode;
	static TArray<FName> UnknownPackages;
	static TMap<UClass*, int32> BlueprintClassCount;

	TSharedPtr<FGraphEditor_ClassNode_Template> CreateClassDataNode(const struct FAssetData& AssetData);
	TSharedPtr<FGraphEditor_ClassNode_Template> FindBaseClassNode(TSharedPtr<FGraphEditor_ClassNode_Template> Node, const FString& ClassName);
	void FindAllSubClasses(TSharedPtr<FGraphEditor_ClassNode_Template> Node, TArray<FGraphEditor_ClassData_Template>& AvailableClasses);

	UClass* FindAssetClass(const FString& GeneratedClassPackage, const FString& AssetName);
	void BuildClassGraph();
	void AddClassGraphChildren(TSharedPtr<FGraphEditor_ClassNode_Template> Node, TArray<TSharedPtr<FGraphEditor_ClassNode_Template> >& NodeList);

	bool IsHidingClass(UClass* Class);
	bool IsHidingParentClass(UClass* Class);
	bool IsPackageSaved(FName PackageName);
};