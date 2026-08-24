#include "BloodFieldShaderInterface.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "BloodFieldCS.h"
#include "BloodFieldShaderTypes.h"

void FBloodFieldShaderInterface::Dispatch(FTextureRenderTargetResource* TargetResource, const FBloodFieldPassInput& Input)
{
	ENQUEUE_RENDER_COMMAND(DispatchBloodField)(
		[TargetResource, Input]
		(FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

			FRDGTextureRef VolumeTexture = TargetResource->GetRenderTargetTexture(GraphBuilder);

			if (!VolumeTexture)
			{
				UE_LOG(LogTemp, Warning, TEXT("Fail to Get Render Target Texture during Dispatch"));
				return;
			}

			AddBloodFieldPass(GraphBuilder, VolumeTexture, ShaderMap, Input);

			GraphBuilder.Execute();
		});
}

void FBloodFieldShaderInterface::AddBloodFieldPass(FRDGBuilder& GraphBuilder, FRDGTextureRef VolumeTexture, const FGlobalShaderMap* InShaderMap, const FBloodFieldPassInput& Input)
{
	if (!ensure(IsInRenderingThread()) || Input.SplatsData.IsEmpty()) return;

	RDG_EVENT_SCOPE(GraphBuilder, "BloodField");

	FRDGBufferRef SplatBuffer = CreateStructuredBuffer(
		GraphBuilder, TEXT("BloodField.SplatData"), Input.SplatsData
	);
	FRDGBufferRef GroupBuffer = CreateStructuredBuffer(
		GraphBuilder, TEXT("BloodField.GroupData"), Input.GroupData
	);

	FRDGTextureRef ResolveTexture = GraphBuilder.CreateTexture(FRDGTextureDesc::Create3D(
		Input.Resolution, PF_R32_UINT, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV
	), TEXT("BloodField.ResolveTexture"));
	FRDGTextureUAVRef ResolveUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(ResolveTexture));

	//=======
	// Pass 1
	//=======
	TShaderMapRef<FBloodSelectCS> BloodSelectCS(InShaderMap);

	FBloodSelectCS::FParameters* SelectParameters = GraphBuilder.AllocParameters<FBloodSelectCS::FParameters>();

	SelectParameters->Resolution = Input.Resolution;
	SelectParameters->Scale = Input.Scale;
	SelectParameters->Origin = Input.Origin;
	SelectParameters->Radius = Input.Radius;
	SelectParameters->SplatCount = Input.SplatsData.Num();
	FVector3f VoxelSize = Input.Scale / FVector3f(Input.Resolution);
	FIntVector VoxelRadius(
		FMath::CeilToInt(Input.Radius / VoxelSize.X),
		FMath::CeilToInt(Input.Radius / VoxelSize.Y),
		FMath::CeilToInt(Input.Radius / VoxelSize.Z)
	);
	SelectParameters->VoxelRadius = VoxelRadius;
	FIntVector Side = VoxelRadius * 2 + FIntVector(1, 1, 1);
	SelectParameters->Side = Side;
	SelectParameters->VoxelsPerSplat = Side.X * Side.Y * Side.Z;
	SelectParameters->SplatBuffer = GraphBuilder.CreateSRV(SplatBuffer);

	SelectParameters->ResolveTextureUAV = ResolveUAV;
	AddClearUAVPass(GraphBuilder, ResolveUAV, 0u);

	const uint32 TotalThread = SelectParameters->SplatCount * SelectParameters->VoxelsPerSplat;
	const FIntVector SelectGroupCount(FMath::DivideAndRoundUp(TotalThread, 64u), 1, 1);

	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Select"), ERDGPassFlags::Compute | ERDGPassFlags::NeverCull, BloodSelectCS, SelectParameters, SelectGroupCount);

	//=======
	// Pass 2
	//=======
	TShaderMapRef<FBloodFieldResolveCS> BloodFieldResolveCS(InShaderMap);

	FBloodFieldResolveCS::FParameters* ResolveParameters = GraphBuilder.AllocParameters<FBloodFieldResolveCS::FParameters>();
	ResolveParameters->Resolution = Input.Resolution;
	ResolveParameters->Scale = Input.Scale;
	ResolveParameters->Origin = Input.Origin;
	ResolveParameters->Radius = Input.Radius;
	ResolveParameters->PatternWorldSize = Input.PatternWorldSize;
	ResolveParameters->SplatBuffer = GraphBuilder.CreateSRV(SplatBuffer);
	ResolveParameters->GroupBuffer = GraphBuilder.CreateSRV(GroupBuffer);
	ResolveParameters->OutVolume = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(VolumeTexture));
	ResolveParameters->ResolveTextureSRV = GraphBuilder.CreateSRV(ResolveTexture);
	ResolveParameters->PatternTexture = Input.BloodTextures;
	ResolveParameters->PatternSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FRDGTextureUAVRef VolumeUAV = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(VolumeTexture));

	const FIntVector ResolveGroupCount(
		FMath::DivideAndRoundUp(Input.Resolution.X, 4),
		FMath::DivideAndRoundUp(Input.Resolution.Y, 4),
		FMath::DivideAndRoundUp(Input.Resolution.Z, 4)
	);

	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Resolve"), ERDGPassFlags::Compute | ERDGPassFlags::NeverCull, BloodFieldResolveCS, ResolveParameters, ResolveGroupCount);
}