#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FReactiveConstantParameters, )
UNIFORM_MEMBER(int, resolution)
UNIFORM_MEMBER(float, heightFieldTexelSize)
UNIFORM_MEMBER(float, snowMudTexelSize)
UNIFORM_MEMBER(float, reactiveSize)
END_UNIFORM_BUFFER_STRUCT(FReactiveConstantParameters)

BEGIN_UNIFORM_BUFFER_STRUCT(FReactiveDynamicParameters, )
UNIFORM_MEMBER(float, travelSpeed)
UNIFORM_MEMBER(float, damping)
UNIFORM_MEMBER(float, impactStrength)
UNIFORM_MEMBER(float, waveStrength)
UNIFORM_MEMBER(float, heightFieldScale)
UNIFORM_MEMBER(FVector, distanceOffset)
UNIFORM_MEMBER(int, heightField2FrameRes)
UNIFORM_MEMBER(int, heightField1FrameRes)
UNIFORM_MEMBER(int, heightFieldOutRes)
UNIFORM_MEMBER(int, offsetHeightField0FrameRes)
UNIFORM_MEMBER(int, offsetHeightField1FrameRes)
UNIFORM_MEMBER(int, heightFieldNormalRes)
UNIFORM_MEMBER(int, waterLandSceneCaptureRes)
UNIFORM_MEMBER(int, snowMudSceneCaptureRes)
UNIFORM_MEMBER(int, impactCapturesRes)
UNIFORM_MEMBER(int, snowResolvedTextureRes)
UNIFORM_MEMBER(int, snowMudWriteOutRes)
END_UNIFORM_BUFFER_STRUCT(FReactiveDynamicParameters)

typedef TUniformBufferRef<FReactiveConstantParameters> FReactiveConstantParamUBRef;
typedef TUniformBufferRef<FReactiveDynamicParameters> FReactiveDynamicParamUBRef;

#define THREAD_GROUP_SIZE 32

struct FNecessarySurfaces
{
public:
	FShaderResourceViewRHIRef/*Texture2D<float>*/ heightField2Frame;
	FShaderResourceViewRHIRef/*Texture2D<float>*/ heightField1Frame;

	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ heightFieldOut;//OP

	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ offsetHeightField0Frame;
	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ offsetHeightField1Frame;

	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ heightFieldNormal;//OP From HeightFieldOut

	FShaderResourceViewRHIRef/*Texture2D<float>*/ waterLandSceneCapture;
	FShaderResourceViewRHIRef/*Texture2D<float>*/ snowMudSceneCapture;
	FShaderResourceViewRHIRef/*Texture2D<float>*/ impactCaptures;

	FShaderResourceViewRHIRef/*Texture2D<float>*/ snowResolvedTexture;
	FUnorderedAccessViewRHIRef/*RWTexture2D<float>*/ snowMudWriteOut;// OP
};


class FReactiveComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FReactiveComputeShader, Global);

public:

	FReactiveComputeShader() {}

	explicit FReactiveComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5); }

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);

		Ar << heightField2Frame;
		Ar << heightField1Frame;

		Ar << heightFieldOut;//OP

		Ar << offsetHeightField0Frame;
		Ar << offsetHeightField1Frame;

		Ar << heightFieldNormal;//OP From HeightFieldOut

		Ar << waterLandSceneCapture;
		Ar << snowMudSceneCapture;
		Ar << impactCaptures;

		Ar << snowResolvedTexture;
		Ar << snowMudWriteOut;// OP

		return bShaderHasOutdatedParams;
	}


	void setSurfaces(FRHICommandList& RHICmdList, FNecessarySurfaces& surfaces);
	void setUniformBuffers(FRHICommandList& RHICmdList,FReactiveDynamicParameters& VariableParameters,
		FReactiveConstantParameters& constanctParameter);
	void unbindBuffers(FRHICommandList& RHICmdList);
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);

private:

	FShaderResourceParameter/*Texture2D<float>*/ heightField2Frame;
	FShaderResourceParameter/*Texture2D<float>*/ heightField1Frame;

	FShaderResourceParameter/*RWTexture2D<float>*/ heightFieldOut;//OP

	FShaderResourceParameter/*RWTexture2D<float>*/ offsetHeightField0Frame;
	FShaderResourceParameter/*RWTexture2D<float>*/ offsetHeightField1Frame;

	FShaderResourceParameter/*RWTexture2D<float>*/ heightFieldNormal;//OP From HeightFieldOut

	FShaderResourceParameter/*Texture2D<float>*/ waterLandSceneCapture;
	FShaderResourceParameter/*Texture2D<float>*/ snowMudSceneCapture;
	FShaderResourceParameter/*Texture2D<float>*/ impactCaptures;

	FShaderResourceParameter/*Texture2D<float>*/ snowResolvedTexture;
	FShaderResourceParameter/*RWTexture2D<float>*/ snowMudWriteOut;// OP
};