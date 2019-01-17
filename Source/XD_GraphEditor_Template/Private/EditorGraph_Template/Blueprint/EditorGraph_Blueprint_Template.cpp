// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Blueprint_Template.h"
#include "BP_Graph_Template.h"
#include "BP_GraphNode_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

UEditorGraph_Blueprint_Template::UEditorGraph_Blueprint_Template()
{
	DesignerGraph_Template = CreateDefaultSubobject<UBP_Graph_Template>(GET_MEMBER_NAME_CHECKED(UEditorGraph_Blueprint_Template, DesignerGraph_Template));
}

bool FDelegateEditorBinding_Template::DoesBindingTargetExist(UEditorGraph_Blueprint_Template* Blueprint) const
{
	return Blueprint->DesignerGraph_Template->Nodes.ContainsByPredicate([this](const UBP_GraphNode_Template* Node) {return Node && Node == Object.Get(); });
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
	return Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, MemberGuid);
}

#undef LOCTEXT_NAMESPACE
