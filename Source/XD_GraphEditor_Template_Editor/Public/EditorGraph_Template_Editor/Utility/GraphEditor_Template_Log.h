// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
DECLARE_LOG_CATEGORY_EXTERN(Log_GraphEditor_Template, Verbose, All);
#define GraphEditor_Template_Log(Message,...) UE_LOG(Log_GraphEditor_Template, Log, TEXT(Message), ##__VA_ARGS__)
#define GraphEditor_Template_Display_Log(Message,...) UE_LOG(Log_GraphEditor_Template, Display, TEXT(Message), ##__VA_ARGS__)
#define GraphEditor_Template_Warning_Log(Message,...) UE_LOG(Log_GraphEditor_Template, Warning, TEXT(Message), ##__VA_ARGS__)
#define GraphEditor_Template_Error_Log(Message,...) UE_LOG(Log_GraphEditor_Template, Error, TEXT(Message), ##__VA_ARGS__)


