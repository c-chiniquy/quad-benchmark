
#include "Common.hlsl"

ConstantBuffer<PushConstants> pushConstants : register(b0);

struct VertexInput
{
	float2 position : POSITION;
	float width : WIDTH;
	float height : HEIGHT;
	float4 color : COLOR;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PixelInput VSMain(VertexInput input, uint vertexID : SV_VertexID)
{
	PixelInput output;

	float2 quad[4] = { float2(0, 0), float2(input.width, 0), float2(0, input.height), float2(input.width, input.height) };
	float2 cornerPos = float2(input.position + quad[vertexID]);
	float2 screenPos = ConvertToScreenSpaceCoords(cornerPos, pushConstants.screenSize);

	output.position = float4(screenPos, 0.0f, 1.0f);
	output.color = input.color;

	return output;
}