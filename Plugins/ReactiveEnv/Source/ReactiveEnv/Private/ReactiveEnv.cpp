// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ReactiveEnv.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FReactiveEnvModule"

void FReactiveEnvModule::StartupModule()
{
	AddShaderSourceDirectoryMapping(TEXT("/ReactiveEnv/Private"), FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("/ReactiveEnv/Shaders"), TEXT("/Private")));
	//AddShaderSourceDirectoryMapping(TEXT("/ReactiveEnv/Public"), FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("/ReactiveEnv/Shaders"), TEXT("/Public")));
}

void FReactiveEnvModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FReactiveEnvModule, ReactiveEnv)