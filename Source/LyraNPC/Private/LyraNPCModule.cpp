// Copyright LyraNPC Framework. All Rights Reserved.

#include "LyraNPCModule.h"

#define LOCTEXT_NAMESPACE "FLyraNPCModule"

DEFINE_LOG_CATEGORY(LogLyraNPC);

void FLyraNPCModule::StartupModule()
{
	UE_LOG(LogLyraNPC, Log, TEXT("LyraNPC Module Started"));
}

void FLyraNPCModule::ShutdownModule()
{
	UE_LOG(LogLyraNPC, Log, TEXT("LyraNPC Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLyraNPCModule, LyraNPC)
