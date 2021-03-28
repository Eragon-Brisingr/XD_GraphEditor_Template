// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"

#include "EditorGraph_Template/Blueprint/BlueprintGeneratedClass_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"
#include "EditorGraph_Template/ObjectBase_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

UEditorGraph_Blueprint_Template::UEditorGraph_Blueprint_Template()
{
	DesignerGraph_Template = CreateDefaultSubobject<UBP_Graph_Template>(GET_MEMBER_NAME_CHECKED(UEditorGraph_Blueprint_Template, DesignerGraph_Template));
}

UClass* UEditorGraph_Blueprint_Template::GetBlueprintClass() const
{
	return UBlueprintGeneratedClass_Template::StaticClass();
}

void UEditorGraph_Blueprint_Template::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	AllowedChildrenOfClasses.Add(UObjectBase_Template::StaticClass());
}

bool FDelegateEditorBinding_Template::DoesBindingTargetExist(UEditorGraph_Blueprint_Template* Blueprint) const
{
	return Blueprint->DesignerGraph_Template->GetAllNodes().ContainsByPredicate([this](const UBP_GraphNode_Template* Node) {return Node && Node == Object.Get(); });
}

FDelegateRuntimeBinding_Template FDelegateEditorBinding_Template::ToRuntimeBinding(UEditorGraph_Blueprint_Template* Blueprint) const
{
	FDelegateRuntimeBinding_Template RuntimeBinding;
	RuntimeBinding.FunctionName = GetFunctionName(Blueprint);
	RuntimeBinding.ObjectName = Object.Get()->GetName();
	return RuntimeBinding;
}

FName FDelegateEditorBinding_Template::GetFunctionName(UEditorGraph_Blueprint_Template* Blueprint) const
{
	return Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, MemberFunctionGuid);
}

#undef LOCTEXT_NAMESPACE
