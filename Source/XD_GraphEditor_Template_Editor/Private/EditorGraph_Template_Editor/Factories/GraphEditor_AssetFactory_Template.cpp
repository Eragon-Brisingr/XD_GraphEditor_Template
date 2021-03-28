// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/Factories/GraphEditor_AssetFactory_Template.h"
#include <Kismet2/KismetEditorUtilities.h>

#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"
#include "EditorGraph_Template/Blueprint/BlueprintGeneratedClass_Template.h"
#include "EditorGraph_Template/ObjectBase_Template.h"

UGraphEditor_AssetFactory_Template::UGraphEditor_AssetFactory_Template()
{
	SupportedClass = UEditorGraph_Blueprint_Template::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UGraphEditor_AssetFactory_Template::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UEditorGraph_Blueprint_Template* NewBP = CastChecked<UEditorGraph_Blueprint_Template>(FKismetEditorUtilities::CreateBlueprint(UObjectBase_Template::StaticClass(), InParent, Name, EBlueprintType::BPTYPE_Normal, UEditorGraph_Blueprint_Template::StaticClass(), UBlueprintGeneratedClass_Template::StaticClass(), CallingContext));
	return NewBP;
}

UObject* UGraphEditor_AssetFactory_Template::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}
