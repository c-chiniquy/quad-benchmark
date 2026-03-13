dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_RawRect" -Fh "../src/shaders/VS_RawRect.h" "VS_VertexPulledRect.hlsl" -D RAW_BUFFER -Qstrip_reflect -Wno-ignored-attributes
dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_StructuredRect" -Fh "../src/shaders/VS_StructuredRect.h" "VS_VertexPulledRect.hlsl" -D STRUCTURED_BUFFER -Qstrip_reflect -Wno-ignored-attributes
dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_InstancedRect" -Fh "../src/shaders/VS_InstancedRect.h" "VS_InstancedRect.hlsl" -Qstrip_reflect -Wno-ignored-attributes
dxc.exe -WX -Qstrip_debug -T ps_6_6 -E PSMain -Vn "g_PS_Color" -Fh "../src/shaders/PS_Color.h" "PS_Color.hlsl" -Qstrip_reflect -Wno-ignored-attributes
dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_Triangles" -Fh "../src/shaders/VS_Triangles.h" "VS_Triangles.hlsl" -Qstrip_reflect -Wno-ignored-attributes

dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_RawRect" -Fh "../src/shaders/VS_RawRect_SPIRV.h" "VS_VertexPulledRect.hlsl" -D RAW_BUFFER -D VULKAN -spirv -fspv-target-env=vulkan1.3 -fvk-use-dx-layout -fvk-bind-resource-heap 0 0 -fvk-bind-sampler-heap 1 0
dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_StructuredRect" -Fh "../src/shaders/VS_StructuredRect_SPIRV.h" "VS_VertexPulledRect.hlsl" -D STRUCTURED_BUFFER -D VULKAN -spirv -fspv-target-env=vulkan1.3 -fvk-use-dx-layout -fvk-bind-resource-heap 0 0 -fvk-bind-sampler-heap 1 0
dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_InstancedRect" -Fh "../src/shaders/VS_InstancedRect_SPIRV.h" "VS_InstancedRect.hlsl" -D VULKAN -spirv -fspv-target-env=vulkan1.3 -fvk-use-dx-layout -fvk-bind-resource-heap 0 0 -fvk-bind-sampler-heap 1 0
dxc.exe -WX -Qstrip_debug -T ps_6_6 -E PSMain -Vn "g_PS_Color" -Fh "../src/shaders/PS_Color_SPIRV.h" "PS_Color.hlsl" -D VULKAN -spirv -fspv-target-env=vulkan1.3 -fvk-use-dx-layout -fvk-bind-resource-heap 0 0 -fvk-bind-sampler-heap 1 0
dxc.exe -WX -Qstrip_debug -T vs_6_6 -E VSMain -Vn "g_VS_Triangles" -Fh "../src/shaders/VS_Triangles_SPIRV.h" "VS_Triangles.hlsl" -D VULKAN -spirv -fspv-target-env=vulkan1.3 -fvk-use-dx-layout -fvk-bind-resource-heap 0 0 -fvk-bind-sampler-heap 1 0

pause