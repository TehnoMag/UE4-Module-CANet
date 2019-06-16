// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCANet, Log, All);

#if WITH_EDITOR
// From https://forums.unrealengine.com/development-discussion/c-gameplay-programming/110349-code-snippet-custom-message-in-the-play-in-editor-message-log?p=935128#post935128

#define FFIND(Name) FindFunction(Name)

#define PIE_Warning(Message,Token) FMessageLog("PIE").Warning(FText::FromString(FString::Printf(TEXT("{CANET}:: %s --> "),*Message)))->AddToken(Token)
#define PIE_Error(Message,Token) FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("{CANET}:: %s --> "),*Message)))->AddToken(Token)
#define PIE_Critical(Token) FMessageLog("PIE").Message(EMessageSeverity::CriticalError)->AddToken(Token)
#define PIE_Message(Token) FMessageLog("PIE").Message(EMessageSeverity::Info)->AddToken(Token);


UENUM()
enum class ESeverity : uint8 {
	CriticalError = 0,
	Error = 1,
	Warning = 2,
	Info = 3
};


//CANET_API void LOG_PIE(const bool Debug, const bool Logs, const float Duration, const FColor Color, const FString Message);
CANET_API void LOG_PIE(const bool Debug, const ESeverity Severity, const UObject* Owner, const UFunction* Function, const FString Message = TEXT(""));
CANET_API void LOG_PIE(const ESeverity Severity, const UObject* Owner, const UFunction* Function, const FString Message = TEXT(""));
CANET_API void LOG_PIE(const UObject* Owner, const UFunction* Function, const FString Message = TEXT(""));
CANET_API void LOG_PIE(const UFunction* Function, const UObject* Owner);
CANET_API void LOG_PIE(const UFunction* Function);
#endif