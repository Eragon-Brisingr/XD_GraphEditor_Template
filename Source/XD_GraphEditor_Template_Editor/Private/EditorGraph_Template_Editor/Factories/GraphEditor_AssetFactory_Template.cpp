// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/Factories/GraphEditor_AssetFactory_Template.h"

#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"

UGraphEditor_AssetFactory_Template::UGraphEditor_AssetFactory_Template()
{
	SupportedClass = UBP_Graph_Template::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UGraphEditor_AssetFactory_Template::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UBP_Graph_Template>(InParent, InClass, InName, Flags);
}