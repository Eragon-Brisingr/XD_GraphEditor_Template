// Fill out your copyright notice in the Description page of Project Settings.

#include "BP_Compiler_Template.h"
#include "EditorGraph_Blueprint_Template.h"
#include "BlueprintGeneratedClass_Template.h"
#include "KismetReinstanceUtilities.h"
#include "BP_Graph_Template.h"

FBP_Compiler_Template::FBP_Compiler_Template(UEditorGraph_Blueprint_Template* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions, TArray<UObject*>* InObjLoaded)
	: FKismetCompilerContext(SourceSketch, InMessageLog, InCompilerOptions, InObjLoaded)
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

void FBP_Compiler_Template::FinishCompilingClass(UClass* Class)
{
	FKismetCompilerContext::FinishCompilingClass(Class);
	UBlueprintGeneratedClass_Template* BlueprintGeneratedClass = CastChecked<UBlueprintGeneratedClass_Template>(Class);

	UEditorGraph_Blueprint_Template* Blueprint_Template = CastChecked<UEditorGraph_Blueprint_Template>(Class->ClassGeneratedBy);
	
	BlueprintGeneratedClass->DesignerGraph_Template = (UBP_Graph_Template*)StaticDuplicateObject(Blueprint_Template->DesignerGraph_Template, BlueprintGeneratedClass);
}
