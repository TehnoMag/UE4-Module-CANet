// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

#include "CANet.Definations.h"
#include "ModuleManager.h"

DEFINE_LOG_CATEGORY(LogCANet);
IMPLEMENT_MODULE(FDefaultModuleImpl, CANet);

#if WITH_EDITOR
// From https://forums.unrealengine.com/development-discussion/c-gameplay-programming/110349-code-snippet-custom-message-in-the-play-in-editor-message-log?p=935128#post935128

#include "MessageLog.h"
#include "TokenizedMessage.h"


#define LOCTEXT_NAMESPACE "CANET"


void LOG_PIE(const bool Debug, const ESeverity Severity, const UObject* Owner, const UFunction* Function, const FString Message) {
	if (!Debug) { return; }
	if (!Function) { return; }
	if (!Owner) { return; }
	//
#if WITH_EDITOR
	FFormatNamedArguments ARG;
	FFormatArgumentValue AFunction = Function->GetDisplayNameText();
	FFormatArgumentValue AInfo = FText::FromString(Function->GetFullName());
	FFormatArgumentValue APackage = FText::FromString(Owner->GetFullName());
	//
	ARG.Add(TEXT("Function"), AFunction);
	ARG.Add(TEXT("Package"), APackage);
	ARG.Add(TEXT("Details"), AInfo);
	//
	const auto Token = FTextToken::Create(FText::Format(LOCTEXT("LOG_PIE", "{Function}: [{Details} at ({Package})]"), ARG));
	//
	switch (Severity) {
	case ESeverity::Info:
		PIE_Message(Token); break;
	case ESeverity::Warning:
		PIE_Warning(Message, Token); break;
	case ESeverity::Error:
		PIE_Error(Message, Token); break;
	case ESeverity::CriticalError:
		PIE_Critical(Token); break;
	default: break;
	}
#endif
}

CANET_API void LOG_PIE(const ESeverity Severity, const UObject* Owner, const UFunction* Function, const FString Message) {
	LOG_PIE(true, Severity, Owner, Function, Message);
}

CANET_API void LOG_PIE(const UObject* Owner, const UFunction* Function, const FString Message) {
	LOG_PIE(ESeverity::Warning, Owner, Function, Message);
}

CANET_API void LOG_PIE(const UFunction* Function, const UObject* Owner) {
	LOG_PIE(Owner, Function, FString(TEXT("Something went wrong!")));
}

CANET_API void LOG_PIE(const UFunction* Function) {
	LOG_PIE(ESeverity::Info, Function, Function, TEXT(""));
}


#undef LOCTEXT_NAMESPACE
#undef PIE_Message
#undef PIE_Warning
#undef PIE_Critical
#undef PIE_Error
#undef FFIND
#endif
