
struct PushConstants
{
	uint rawOrStructuredBufferIndex;
	float2 screenSize;
};

float4 ConvertToFloat4(uint color32)
{
	return float4(
		(float)(color32 & 0xFF) / 255.0f,
		(float)((color32 >> 8) & 0xFF) / 255.0f,
		(float)((color32 >> 16) & 0xFF) / 255.0f,
		(float)((color32 >> 24) & 0xFF) / 255.0f);
}

float2 ConvertToScreenSpaceCoords(float2 pos, float2 screenSize)
{
	float2 screenPos;
	screenPos.x = (pos.x / screenSize.x) * 2.0 - 1.0;
	screenPos.y = 1.0 - (pos.y / screenSize.y) * 2.0;
	return screenPos;
}
