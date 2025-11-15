// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLyraNPC, Log, All);

class FLyraNPCModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface. Beware of calling this during the shutdown phase.
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FLyraNPCModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FLyraNPCModule>("LyraNPC");
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("LyraNPC");
	}
};
