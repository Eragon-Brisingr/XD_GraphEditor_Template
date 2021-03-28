// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Template/ObjectBase_Template.h"
#include "EditorGraph_Template/Blueprint/BlueprintGeneratedClass_Template.h"

UBP_Graph_Template* UObjectBase_Template::GetGraph_Template() const
{
	UBlueprintGeneratedClass_Template* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass_Template>(GetClass());
	return BlueprintGeneratedClass->DesignerGraph_Template;
}
