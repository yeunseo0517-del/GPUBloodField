// Copyright Epic Games, Inc. All Rights Reserved.

#include "BloodFieldShaders.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FBloodFieldShadersModule"

void FBloodFieldShadersModule::StartupModule()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BloodField"));

	check(Plugin.IsValid());

	const FString ShaderDirectory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));

	AddShaderSourceDirectoryMapping(TEXT("/Plugin/BloodField"),
		ShaderDirectory);
}

void FBloodFieldShadersModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBloodFieldShadersModule, BloodFieldShaders)