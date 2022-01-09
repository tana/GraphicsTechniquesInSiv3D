# GraphicsTechniquesInSiv3D
Implementation of various 3D graphics techniques in Siv3D.

## Implemented techniques
| Name | Code |
| ---- | ---- |
| Shadow Mapping <br> <img width="300" alt="screenshot of shadow mapping" src="screenshot/shadow_mapping.png"> | Header: [ShadowMapping.h](GraphicsTechniquesInSiv3D/ShadowMapping.h) <br> Source: [ShadowMapping.cpp](GraphicsTechniquesInSiv3D/ShadowMapping.cpp) <br> Shadow map generation shader: [shadow_map_generation.hlsl](GraphicsTechniquesInSiv3D/App/shadow_map_generation.hlsl) <br> Main shader: [shadow_mapping.hlsl](GraphicsTechniquesInSiv3D/App/shadow_mapping.hlsl) |
| Soft shadow using Variance Shadow Map <br> <img width="300" alt="screenshot of variance shadow map" src="screenshot/variance_shadow_map.png"> | Header: [VarianceShadowMap.h](GraphicsTechniquesInSiv3D/VarianceShadowMap.h) <br> Source: [VarianceShadowMap.cpp](GraphicsTechniquesInSiv3D/VarianceShadowMap.cpp) <br> Shadow map generation shader: [variance_shadow_map_generation.hlsl](GraphicsTechniquesInSiv3D/App/variance_shadow_map_generation.hlsl) <br> Main shader: [variance_shadow_mapping.hlsl](GraphicsTechniquesInSiv3D/App/variance_shadow_mapping.hlsl) |
