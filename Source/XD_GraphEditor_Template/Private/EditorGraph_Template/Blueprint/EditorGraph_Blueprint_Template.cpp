// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Blueprint_Template.h"
#include "BP_Graph_Template.h"

UEditorGraph_Blueprint_Template::UEditorGraph_Blueprint_Template()
{
	DesignerGraph_Template = CreateDefaultSubobject<UBP_Graph_Template>(GET_MEMBER_NAME_CHECKED(UEditorGraph_Blueprint_Template, DesignerGraph_Template));
}
