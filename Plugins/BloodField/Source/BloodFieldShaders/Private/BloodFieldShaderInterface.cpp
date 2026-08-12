#include "BloodFieldShaderInterface.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "BloodFieldCS.h"

void FBloodFieldShaderInterface::Dispatch(FTextureRenderTargetResource* TargetResource, const FIntVector& InResolution, const FVector3f& InScale, const FVector3f& InOrigin, const FVector3f& InLocation, float InRadius)
{
	ENQUEUE_RENDER_COMMAND(DispatchBloodField)(
		[TargetResource, InResolution, InScale, InOrigin, InLocation, InRadius]
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

			AddBloodFieldPass(GraphBuilder, ShaderMap, InResolution, InScale, InOrigin, InLocation, InRadius, VolumeTexture);

			GraphBuilder.Execute();
		});
}

void FBloodFieldShaderInterface::AddBloodFieldPass(FRDGBuilder& GraphBuilder, const FGlobalShaderMap* InShaderMap, const FIntVector& InResolution, const FVector3f& InScale, const FVector3f& InOrigin, const FVector3f& InLocation, float InRadius, FRDGTextureRef VolumeTexture)
{
	if (!ensure(IsInRenderingThread())) return;

	RDG_EVENT_SCOPE(GraphBuilder, "BloodField");

	TShaderMapRef<FBloodFieldCS> ComputeShader(InShaderMap);

	FBloodFieldCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBloodFieldCS::FParameters>();

	PassParameters->Resolution = InResolution;
	PassParameters->Scale = InScale;
	PassParameters->Origin = InOrigin;
	PassParameters->Location = InLocation;
	//const float MaxVoxelSize = FMath::Max3(InScale.X / InResolution.X, InScale.Y / InResolution.Y, InScale.Z / InResolution.Z);
	//PassParameters->Radius = FMath::Max(InRadius, MaxVoxelSize);
	PassParameters->Radius = InRadius;
	PassParameters->OutVolume = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(VolumeTexture));

	const FIntVector GroupCount(
		FMath::DivideAndRoundUp(InResolution.X, int32(4)),
		FMath::DivideAndRoundUp(InResolution.Y, int32(4)),
		FMath::DivideAndRoundUp(InResolution.Z, int32(4))
	);

	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("BloodField"), ERDGPassFlags::Compute | ERDGPassFlags::NeverCull, ComputeShader, PassParameters, GroupCount);
}