
#include "Common.hlsl"

ConstantBuffer<PushConstants> pushConstants : register(b0);

struct VertexInput
{
	float2 position : POSITION;
	float4 color : COLOR;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PixelInput VSMain(VertexInput input)
{
	PixelInput output;

	float2 screenPos = ConvertToScreenSpaceCoords(input.position, pushConstants.screenSize);

	output.position = float4(screenPos, 0.0f, 1.0f);
	output.color = input.color;

	return output;
}
