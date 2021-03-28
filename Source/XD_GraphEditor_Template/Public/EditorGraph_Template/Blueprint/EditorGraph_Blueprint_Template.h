// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "EditorGraph_Template/Blueprint/BlueprintGeneratedClass_Template.h"
#include "EditorGraph_Blueprint_Template.generated.h"

class UEditorGraph_Blueprint_Template;

/**
 * 
 */
USTRUCT()
struct XD_GRAPHEDITOR_TEMPLATE_API FDelegateEditorBinding_Template
{
	GENERATED_BODY()
public:
	/** The member widget the binding is on, must be a direct variable of the UUserWidget. */
	UPROPERTY()
	TSoftObjectPtr<UObject> Object;

	UPROPERTY()
	FName PropertyName;

	/** If it's an actual Function Graph in the blueprint that we're bound to, there's a GUID we can use to lookup that function, to deal with renames better.  This is that GUID. */
	UPROPERTY()
	FGuid MemberFunctionGuid;

	bool DoesBindingTargetExist(UEditorGraph_Blueprint_Template* Blueprint) const;

	FDelegateRuntimeBinding_Template ToRuntimeBinding(UEditorGraph_Blueprint_Template* Blueprint) const;

	FName GetFunctionName(UEditorGraph_Blueprint_Template* Blueprint) const;
};


UCLASS()
class XD_GRAPHEDITOR_TEMPLATE_API UEditorGraph_Blueprint_Template : public UBlueprint
{
	GENERATED_BODY()
public:
	UEditorGraph_Blueprint_Template();

	UClass* GetBlueprintClass() const override;
	void GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const override;
public:
	UPROPERTY()
	class UBP_Graph_Template* DesignerGraph_Template;

	UPROPERTY()
	TArray<FDelegateEditorBinding_Template> Bindings;
};
