// Copyright (c) 2018 Jhonny Hueller
#include "GraphEditor_AssetFactory_Template.h"
#include "EditorGraph_Blueprint_Template.h"
#include "BlueprintGeneratedClass_Template.h"
#include "KismetEditorUtilities.h"

UGraphEditor_AssetFactory_Template::UGraphEditor_AssetFactory_Template()
{
	SupportedClass = UEditorGraph_Blueprint_Template::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UGraphEditor_AssetFactory_Template::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UEditorGraph_Blueprint_Template* NewBP = CastChecked<UEditorGraph_Blueprint_Template>(FKismetEditorUtilities::CreateBlueprint(UObject::StaticClass(), InParent, Name, EBlueprintType::BPTYPE_Normal, UEditorGraph_Blueprint_Template::StaticClass(), UBlueprintGeneratedClass_Template::StaticClass(), CallingContext));
	return NewBP;
}

UObject* UGraphEditor_AssetFactory_Template::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}
