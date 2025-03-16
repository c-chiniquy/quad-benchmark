dxc.exe -WX -Qstrip_debug -Qstrip_reflect -T vs_6_6 -E VSMain -Vn "g_VS_RawRect" -Fh "../src/shaders/VS_RawRect.h" "VS_VertexPulledRect.hlsl" -D RAW_BUFFER
dxc.exe -WX -Qstrip_debug -Qstrip_reflect -T vs_6_6 -E VSMain -Vn "g_VS_StructuredRect" -Fh "../src/shaders/VS_StructuredRect.h" "VS_VertexPulledRect.hlsl" -D STRUCTURED_BUFFER
dxc.exe -WX -Qstrip_debug -Qstrip_reflect -T vs_6_6 -E VSMain -Vn "g_VS_InstancedRect" -Fh "../src/shaders/VS_InstancedRect.h" "VS_InstancedRect.hlsl"
dxc.exe -WX -Qstrip_debug -Qstrip_reflect -T ps_6_6 -E PSMain -Vn "g_PS_Color" -Fh "../src/shaders/PS_Color.h" "PS_Color.hlsl"
dxc.exe -WX -Qstrip_debug -Qstrip_reflect -T vs_6_6 -E VSMain -Vn "g_VS_Triangles" -Fh "../src/shaders/VS_Triangles.h" "VS_Triangles.hlsl"

pause