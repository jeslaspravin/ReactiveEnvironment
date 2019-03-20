#include "ReactiveCompute.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ReactiveEnvActor.h"

void FReactiveCompute::computeInternal()
{
	check(IsInRenderingThread());

	if (bIsUnloading)
	{
		safeReleaseResources();
		return;
	}

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FReactiveComputeShader> computeShader(GetGlobalShaderMap(featureLevel));
	RHICmdList.SetComputeShader(computeShader->GetComputeShader());

	setTextures(RHICmdList, computeShader);
	updateDynamicParameters(RHICmdList, computeShader);

	uint32 numOfGroups = ownerReactiveActor->reactiveRTResolution / THREAD_GROUP_SIZE;
	DispatchComputeShader(RHICmdList, computeShader->GetShader(), numOfGroups, numOfGroups, 1);

	computeShader->unbindBuffers(RHICmdList);
	if (bSnapshot)
	{
		bSnapshot = false;
		snapshot(RHICmdList);
	}
	
	FResolveParams resolveParams;
	RHICmdList.CopyToResolveTarget(heightFieldTextures[heightShiftIndex][heightFieldTextureIndex].GetReference(),
		ownerReactiveActor->waterHeightFieldTexture->Resource->TextureRHI, resolveParams);
	RHICmdList.CopyToResolveTarget(heightFieldNormalTexture.GetReference(),ownerReactiveActor->waterHeightFieldNormalTexture->Resource->TextureRHI
		, resolveParams);
	RHICmdList.CopyToResolveTarget(snowWriteOutTexture.GetReference(), ownerReactiveActor->snowMudTexture->Resource->TextureRHI,resolveParams);

	executionCounter++;// Incrementing counter

	bIsComputing = false;
}

void FReactiveCompute::safeReleaseResources()
{
	snowMudSceneCaptureCurrent.SafeRelease();
	snowMudSceneCaptureSRV.SafeRelease();
	waterLandSceneCaptureCurrent.SafeRelease();
	waterLandSceneCaptureSRV.SafeRelease();
	snowResolvedTextureCurrent.SafeRelease();
	snowResolvedTextureSRV.SafeRelease();
	impactsCapturedCurrent.SafeRelease();
	impactsCapturedSRV.SafeRelease();

	for (int row = 0; row < 2; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			heightFieldTextures[row][col]->Release();

			heightFieldResources[row][col].release();
		}
	}

	heightFieldNormalTexture->Release();
	heightFieldNormalUAV->Release();

	snowWriteOutTexture->Release();
	snowWriteOutUAV->Release();
}

FReactiveCompute::FReactiveCompute(AReactiveEnvActor* ownerActor, ERHIFeatureLevel::Type shaderFeatureLevel)
{
	bIsUnloading = false;
	bIsComputing = false;
	bSnapshot = false;
	executionCounter = 0;
	featureLevel = shaderFeatureLevel;

	ownerReactiveActor = ownerActor;

	check(ownerActor);

	snowResolvedTextureCurrent = ownerActor->snowMudTexture->Resource->TextureRHI->GetTexture2D();
	snowResolvedTextureSRV = RHICreateShaderResourceView(snowResolvedTextureCurrent, 0);
	snowMudSceneCaptureCurrent = ownerActor->snowMudCaptureRT->Resource->TextureRHI->GetTexture2D();
	snowMudSceneCaptureSRV= RHICreateShaderResourceView(snowMudSceneCaptureCurrent, 0);
	waterLandSceneCaptureCurrent= ownerActor->landWaterCaptureRT->Resource->TextureRHI->GetTexture2D();;
	waterLandSceneCaptureSRV = RHICreateShaderResourceView(waterLandSceneCaptureCurrent, 0);
	if(ownerActor->impactsCaptureRT->Resource && ownerActor->impactsCaptureRT->Resource->TextureRHI)
	{
		impactsCapturedCurrent = ownerActor->impactsCaptureRT->Resource->TextureRHI->GetTexture2D();
		impactsCapturedSRV = RHICreateShaderResourceView(impactsCapturedCurrent, 0);
	}
	else
	{
		impactsCapturedCurrent = nullptr;
	}

	FRHIResourceCreateInfo createInfo;
	for (int row = 0; row < 2; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			heightFieldTextures[row][col] = RHICreateTexture2D(ownerActor->reactiveRTResolution, ownerActor->reactiveRTResolution, 
				 EPixelFormat::PF_R16F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, createInfo);
			heightFieldResources[row][col] = FHeightFieldResources(RHICreateShaderResourceView(heightFieldTextures[row][col], 0),
				RHICreateUnorderedAccessView(heightFieldTextures[row][col]));
		}
	}

	heightFieldNormalTexture = RHICreateTexture2D(ownerActor->reactiveRTResolution, ownerActor->reactiveRTResolution,
		EPixelFormat::PF_FloatRGBA, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, createInfo);
	heightFieldNormalUAV = RHICreateUnorderedAccessView(heightFieldNormalTexture);

	snowWriteOutTexture = RHICreateTexture2D(ownerActor->reactiveRTResolution, ownerActor->reactiveRTResolution,
		EPixelFormat::PF_G8, 1, 1, TexCreate_UAV, createInfo);
	snowWriteOutUAV = RHICreateUnorderedAccessView(snowWriteOutTexture);

	dynamicParameters = FReactiveDynamicParameters();
}

FReactiveCompute::~FReactiveCompute()
{
	bIsUnloading = true;
}

void FReactiveCompute::saveSnapshotFrame()
{
	bSnapshot = true;
}

void FReactiveCompute::computeReactive()
{
	if (bIsUnloading || bIsComputing)
	{
		return;
	}

	bIsComputing = true;

	heightFieldTextureIndex = executionCounter % 3;
	heightShiftIndex = executionCounter % 2;
	
	if (heightShiftIndex == 0 && heightFieldTextureIndex == 0)
		executionCounter = 0;

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(FReactiveComputeRun, FReactiveCompute*, ReactiveCompute, this,
		{
			ReactiveCompute->computeInternal();
		});
}


void FReactiveCompute::snapshot(FRHICommandListImmediate& RHICmdList)
{
	// TODO: Save out buffers/textures
}

void FReactiveCompute::updateDynamicParameters(FRHICommandListImmediate& RHICmdList, TShaderMapRef<FReactiveComputeShader>& shader)
{
	dynamicParameters.damping = ownerReactiveActor->waveDamping;
	dynamicParameters.travelSpeed = ownerReactiveActor->getWaveTravelSpeed(); 
	dynamicParameters.impactStrength = ownerReactiveActor->impactStrengthMultiplier;
	dynamicParameters.waveStrength = ownerReactiveActor->waveStrengthMultiplier;
	dynamicParameters.heightFieldScale = ownerReactiveActor->heightFieldNormalScale;

	// Gives Distance offset in texture coordinate space
	dynamicParameters.distanceOffset = ownerReactiveActor->getDisplacementOffset()/ownerReactiveActor->reactiveDistance;

#if WITH_EDITOR
//	if(dynamicParameters.distanceOffset.Size2D()>0.0005)
//		UE_LOG(LogTemp, Log, TEXT("Distance offset %s"), *dynamicParameters.distanceOffset.ToString());
#endif


	uint8 previousPreviousFrameIndex = heightFieldTextureIndex - 2 >= 0 ? heightFieldTextureIndex - 2 : 3 + (heightFieldTextureIndex - 2);
	uint8 previousFrameIndex = (previousPreviousFrameIndex + 1) % 3;
	uint8 previousShiftIndex = (heightShiftIndex + 1) % 2;

	dynamicParameters.heightField1FrameRes = heightFieldTextures[heightShiftIndex][previousFrameIndex]->GetSizeX();
	dynamicParameters.heightField2FrameRes = heightFieldTextures[heightShiftIndex][previousPreviousFrameIndex]->GetSizeX();
	dynamicParameters.offsetHeightField0FrameRes = heightFieldTextures[previousShiftIndex][heightFieldTextureIndex]->GetSizeX();
	dynamicParameters.offsetHeightField1FrameRes = heightFieldTextures[previousShiftIndex][previousFrameIndex]->GetSizeX();

	dynamicParameters.heightFieldNormalRes = heightFieldNormalTexture->GetSizeX();
	dynamicParameters.heightFieldOutRes = heightFieldTextures[heightShiftIndex][heightFieldTextureIndex]->GetSizeX();
	dynamicParameters.snowMudWriteOutRes = snowWriteOutTexture->GetSizeX();

	dynamicParameters.snowMudSceneCaptureRes = snowMudSceneCaptureCurrent->GetSizeX();
	dynamicParameters.waterLandSceneCaptureRes = waterLandSceneCaptureCurrent->GetSizeX();
	dynamicParameters.impactCapturesRes = impactsCapturedCurrent->GetSizeX();
	dynamicParameters.snowResolvedTextureRes = snowResolvedTextureCurrent->GetSizeX();

	constantParams.heightFieldTexelSize = 1 / (float)ownerReactiveActor->reactiveRTResolution;
	constantParams.reactiveSize = ownerReactiveActor->reactiveDistance;
	constantParams.resolution = ownerReactiveActor->reactiveRTResolution;
	constantParams.snowMudTexelSize = 1 / (float)snowWriteOutTexture->GetSizeX();

	shader->setUniformBuffers(RHICmdList, dynamicParameters,constantParams);
}

void FReactiveCompute::setTextures(FRHICommandListImmediate& RHICmdList, TShaderMapRef<FReactiveComputeShader>& shader)
{
	FNecessarySurfaces necessarySurfaces;

	uint8 previousPreviousFrameIndex = heightFieldTextureIndex - 2 >= 0 ? heightFieldTextureIndex - 2 : 3 + (heightFieldTextureIndex - 2);
	uint8 previousFrameIndex = (previousPreviousFrameIndex + 1) % 3;
	uint8 previousShiftIndex = (heightShiftIndex + 1) % 2;

	necessarySurfaces.heightField1Frame = heightFieldResources[heightShiftIndex][previousFrameIndex].SRV;
	necessarySurfaces.heightField2Frame = heightFieldResources[heightShiftIndex][previousPreviousFrameIndex].SRV;

	necessarySurfaces.offsetHeightField0Frame = heightFieldResources[previousShiftIndex][heightFieldTextureIndex].UAV;
	necessarySurfaces.offsetHeightField1Frame = heightFieldResources[previousShiftIndex][previousFrameIndex].UAV;

	necessarySurfaces.heightFieldOut = heightFieldResources[heightShiftIndex][heightFieldTextureIndex].UAV;
	necessarySurfaces.heightFieldNormal = heightFieldNormalUAV;
	necessarySurfaces.snowMudWriteOut = snowWriteOutUAV;

	// Ensure that textures are in sync with the actor owner
	if (ownerReactiveActor->snowMudCaptureRT->Resource->TextureRHI != snowMudSceneCaptureCurrent)
	{
		snowMudSceneCaptureCurrent = ownerReactiveActor->snowMudCaptureRT->Resource->TextureRHI->GetTexture2D();
		snowMudSceneCaptureSRV.SafeRelease();
		snowMudSceneCaptureSRV = RHICreateShaderResourceView(snowMudSceneCaptureCurrent, 0);
	}
	
	if (ownerReactiveActor->landWaterCaptureRT->Resource->TextureRHI != waterLandSceneCaptureCurrent)
	{
		waterLandSceneCaptureCurrent = ownerReactiveActor->landWaterCaptureRT->Resource->TextureRHI->GetTexture2D();
		waterLandSceneCaptureSRV.SafeRelease();
		waterLandSceneCaptureSRV = RHICreateShaderResourceView(waterLandSceneCaptureCurrent, 0);
	}

	if (ownerReactiveActor->impactsCaptureRT && ownerReactiveActor->impactsCaptureRT->Resource->TextureRHI != impactsCapturedCurrent)
	{
		impactsCapturedCurrent = ownerReactiveActor->impactsCaptureRT->Resource->TextureRHI->GetTexture2D();
		impactsCapturedSRV.SafeRelease();
		impactsCapturedSRV = RHICreateShaderResourceView(impactsCapturedCurrent, 0);
	}

	if (ownerReactiveActor->snowMudTexture->Resource->TextureRHI != snowResolvedTextureCurrent)
	{
		snowResolvedTextureCurrent = ownerReactiveActor->snowMudTexture->Resource->TextureRHI->GetTexture2D();
		snowResolvedTextureSRV.SafeRelease();
		snowResolvedTextureSRV = RHICreateShaderResourceView(snowResolvedTextureCurrent, 0);
	}

	necessarySurfaces.snowMudSceneCapture = snowMudSceneCaptureSRV;
	necessarySurfaces.waterLandSceneCapture = waterLandSceneCaptureSRV;
	necessarySurfaces.impactCaptures = impactsCapturedSRV;
	necessarySurfaces.snowResolvedTexture = snowResolvedTextureSRV;

	shader->setSurfaces(RHICmdList, necessarySurfaces);
}
