// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Template_Editor/Compiler/BP_Compiler_Template.h"
#include <UObject/UnrealType.h>
#include <Kismet2/KismetReinstanceUtilities.h>

#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"
#include "EditorGraph_Template/Blueprint/BlueprintGeneratedClass_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

#define CPF_Instanced (CPF_PersistentInstance | CPF_ExportObject | CPF_InstancedReference)

FBP_Compiler_Template::FBP_Compiler_Template(UEditorGraph_Blueprint_Template* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions)
	: FKismetCompilerContext(SourceSketch, InMessageLog, InCompilerOptions)
{

}

FBP_Compiler_Template::~FBP_Compiler_Template()
{
}

void FBP_Compiler_Template::SpawnNewClass(const FString& NewClassName)
{
	NewClass = FindObject<UBlueprintGeneratedClass_Template>(Blueprint->GetOutermost(), *NewClassName);
	if (NewClass == NULL)
	{
		// If the class hasn't been found, then spawn a new one
		NewClass = NewObject<UBlueprintGeneratedClass_Template>(Blueprint->GetOutermost(), FName(*NewClassName), RF_Public | RF_Transactional);
	}
	else
	{
		// Already existed, but wasn't linked in the Blueprint yet due to load ordering issues
		NewClass->ClassGeneratedBy = Blueprint;
		FBlueprintCompileReinstancer::Create(NewClass);
	}
}

void FBP_Compiler_Template::CreateClassVariablesFromBlueprint()
{
	Super::CreateClassVariablesFromBlueprint();

	UEditorGraph_Blueprint_Template* EditorGraph_Blueprint = GetGraphBlueprint();

	for (UBP_GraphNode_Template* Node : EditorGraph_Blueprint->DesignerGraph_Template->GetAllNodes())
	{
		if (Node->bIsVariable)
		{
			FProperty* NodeProperty = CreateVariable(Node->GetFName(), FEdGraphPinType(UEdGraphSchema_K2::PC_Object, NAME_None, Node->GetClass(), EPinContainerType::None, false, FEdGraphTerminalType()));
			if (NodeProperty)
			{
				NodeProperty->SetPropertyFlags(CPF_BlueprintVisible);
				NodeProperty->SetPropertyFlags(CPF_BlueprintReadOnly);
				NodeProperty->SetPropertyFlags(CPF_Instanced);
				NodeProperty->SetPropertyFlags(CPF_RepSkip);
				NodeProperty->SetMetaData(TEXT("Category"), TEXT("设计图表引用"));
			}
		}
	}
}

UEditorGraph_Blueprint_Template* FBP_Compiler_Template::GetGraphBlueprint() const
{
	return CastChecked<UEditorGraph_Blueprint_Template>(Blueprint);
}

void FBP_Compiler_Template::FinishCompilingClass(UClass* Class)
{
	UBlueprintGeneratedClass_Template* BlueprintGeneratedClass = CastChecked<UBlueprintGeneratedClass_Template>(Class);

	UEditorGraph_Blueprint_Template* Blueprint_Template = CastChecked<UEditorGraph_Blueprint_Template>(Class->ClassGeneratedBy);
	BlueprintGeneratedClass->DesignerGraph_Template = (UBP_Graph_Template*)StaticDuplicateObject(Blueprint_Template->DesignerGraph_Template, BlueprintGeneratedClass, NAME_None, RF_AllFlags & ~RF_DefaultSubObject);

	if (bIsFullCompile)
	{
		BlueprintGeneratedClass->Bindings.Empty();

		for (int32 Idx = 0; Idx < Blueprint_Template->Bindings.Num(); ++Idx)
		{
			const FDelegateEditorBinding_Template& Binding = Blueprint_Template->Bindings[Idx];
			if (IsBindingValid(Binding, Class, Blueprint_Template, MessageLog))
			{
				if (Binding.DoesBindingTargetExist(Blueprint_Template))
				{
					BlueprintGeneratedClass->Bindings.Add(Binding.ToRuntimeBinding(Blueprint_Template));
					continue;
				}
			}

			Blueprint_Template->Bindings.RemoveAt(Idx--);
		}
	}

	FKismetCompilerContext::FinishCompilingClass(Class);
}

bool FBP_Compiler_Template::IsBindingValid(const FDelegateEditorBinding_Template& Binding, UClass* Class, class UEditorGraph_Blueprint_Template* Blueprint, FCompilerResultsLog& MessageLog)
{
	if (!Binding.Object.IsValid())
	{
		return false;
	}

	if (UFunction* Function = Class->FindFunctionByName(Binding.GetFunctionName(Blueprint), EIncludeSuperFlag::IncludeSuper))
	{
		FDelegateProperty* DelegateProperty = FindFProperty<FDelegateProperty>(Binding.Object->GetClass(), FName(*(Binding.PropertyName.ToString() + TEXT("Delegate"))));

		// Check the signatures to ensure these functions match.
		if (Function->IsSignatureCompatibleWith(DelegateProperty->SignatureFunction, UFunction::GetDefaultIgnoredSignatureCompatibilityFlags() | CPF_ReturnParm))
		{
			return true;
		}
		else
		{
			FText const ErrorFormat = LOCTEXT("BindingFunctionSigDontMatch", "Binding: property '@@' on widget '@@' bound to function '@@', but the sigatnures don't match.  The function must return the same type as the property and have no parameters.");
			MessageLog.Error(*ErrorFormat.ToString(), DelegateProperty, Binding.Object.Get(), Function);
		}
	}
	return false;
}

#undef LOCTEXT_NAMESPACE