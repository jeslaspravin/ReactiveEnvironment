#pragma once

#include "IReactiveEnvEditor.h"
#include "IAssetTypeActions.h"
#include "HAL/IConsoleManager.h"




class FReactiveEnvEditor : public IReactiveEnvEditor
{

private:

	TArray<TSharedPtr<IAssetTypeActions>> registeredAssetTypeActions;

	TArray<IConsoleCommand*> reactiveEnvConsoleCmds;

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	void onDumpRTs();
};