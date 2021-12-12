#pragma once

# include <Siv3D.hpp>
# include "utils.h"

// シャドウマッピングで影をつける
// ベースはチュートリアル37.1「3Dモデルを描く」（下記のURL）
//   https://zenn.dev/reputeless/books/siv3d-documentation/viewer/tutorial-3d-2#37.1-3d-%E3%83%A2%E3%83%87%E3%83%AB%E3%82%92%E6%8F%8F%E3%81%8F

const int shadowMapSize = 1024;	// シャドウマップの解像度
const double shadowMapScreenSize = 50;
// NearClipとFarClipの意味が逆になっている？
const double shadowMapNearClip = 1000;
const double shadowMapFarClip = 0.1;
const double sunDistance = 50;
// シャドウマップの値の大小が逆になっている？
const double shadowMapDefaultValue = 0.0;	// 何もない場所のシャドウマップの値

const uint32 shadowMapTextureSlot = 1;	// シェーダでのテクスチャ番号
const uint32 vsShadowMappingCBSlot = 4;	// VSShadowMappingの定数バッファ番号

// 描画時にシェーダに渡す情報
struct VSShadowMapping
{
	Mat4x4 sunCameraMatrix;
};

class ShadowMapping : public SceneManager<String>::Scene
{
public:
	ShadowMapping(const InitData& initData)
		: IScene{ initData }
		, backgroundColor(ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve())
		, groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) }
		, groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB }
		, blacksmithModel{ U"example/obj/blacksmith.obj" }
		, millModel{ U"example/obj/mill.obj" }
		, treeModel{ U"example/obj/tree.obj" }
		, pineModel{ U"example/obj/pine.obj" }
		, siv3dkunModel{ U"example/obj/siv3d-kun.obj" }
		, renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }
		, shadowMapTexture{ shadowMapSize, shadowMapSize, TextureFormat::R32_Float, HasDepth::Yes }	// シャドウマップは1チャネルの浮動小数点テクスチャ
		, shadowMapGenerationVS(HLSL{ U"shadow_map_generation.hlsl", U"VS" })
		, shadowMapGenerationPS(HLSL{ U"shadow_map_generation.hlsl", U"PS" })
		, shadowMappingVS(HLSL{ U"shadow_mapping.hlsl", U"VS" })
		, shadowMappingPS(HLSL{ U"shadow_mapping.hlsl", U"PS" })
		, camera{ Graphics3D::GetRenderTargetSize(), 40_deg, Vec3{ 0, 3, -16 } }
	{
		// モデルに付随するテクスチャをアセット管理に登録
		Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(siv3dkunModel, TextureDesc::MippedSRGB);
	}

	void update() override
	{
		camera.update(4.0);

		// 太陽を回す
		Graphics3D::SetSunDirection(Quaternion::RotateY(0.05 * Scene::DeltaTime()) * Graphics3D::GetSunDirection());

		sunPosition = sunDistance * Graphics3D::GetSunDirection();	// 光源の位置
		worldToSunMatrix = DirectX::XMMatrixLookAtLH(ToDXVec(sunPosition), ToDXVec(Vec3::Zero()), ToDXVec(Vec3::UnitY()));
		sunProjMatrix = DirectX::XMMatrixOrthographicLH(shadowMapScreenSize, shadowMapScreenSize, shadowMapNearClip, shadowMapFarClip);
		sunCameraMatrix = worldToSunMatrix * sunProjMatrix;

		cbVSShadowMapping->sunCameraMatrix = sunCameraMatrix;
	}

	void draw() const override
	{
		// シャドウマップの作成
		{
			shadowMapTexture.clear(ColorF{ shadowMapDefaultValue });	// 何もない場所の深度を設定

			const ScopedRenderTarget3D target{ shadowMapTexture };	// 出力先をシャドウマップに設定
			const ScopedCustomShader3D shader{ shadowMapGenerationVS, shadowMapGenerationPS };	// シェーダを変更

			Graphics3D::SetCameraTransform(sunCameraMatrix, sunPosition);

			drawModels();
		}

		// 画面に映るシーン自体の描画
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };
			const ScopedCustomShader3D shader{ shadowMappingVS, shadowMappingPS };
			// シャドウマップの範囲外が影にならないようにする
			const ScopedRenderStates3D::SamplerStateInfo samplerStateInfo{
				.shaderStage = ShaderStage::Pixel,
				.slot = shadowMapTextureSlot,
				.state = SamplerState{
					TextureAddressMode::Border, TextureFilter::Nearest, 1, 0.0f, Float4{ shadowMapDefaultValue, 0.0f, 0.0f, 0.0f  }
				}
			};
			const ScopedRenderStates3D states{ samplerStateInfo };

			Graphics3D::SetCameraTransform(camera);

			Graphics3D::SetPSTexture(shadowMapTextureSlot, shadowMapTexture);
			Graphics3D::SetVSConstantBuffer(vsShadowMappingCBSlot, cbVSShadowMapping);

			drawModels();
		}

		// [RenderTexture を 2D シーンに描画]
		{
			Graphics3D::Flush();
			renderTexture.resolve();
			Shader::LinearToScreen(renderTexture);
		}
	}

private:
	const ColorF backgroundColor;
	const Mesh groundPlane;
	const Texture groundTexture;
	const Model blacksmithModel, millModel, treeModel, pineModel, siv3dkunModel;
	const MSRenderTexture renderTexture;
	const RenderTexture shadowMapTexture;	// シャドウマップ
	const VertexShader shadowMapGenerationVS;	// シャドウマップ生成用の頂点シェーダ
	const PixelShader shadowMapGenerationPS;	// シャドウマップ生成用のピクセルシェーダ
	const VertexShader shadowMappingVS;	// 影を付けて描画するための頂点シェーダ
	const PixelShader shadowMappingPS;	// 影を付けて描画するためのピクセルシェーダ

	ConstantBuffer<VSShadowMapping> cbVSShadowMapping;	// 描画時に渡す定数バッファ

	DebugCamera3D camera;

	Vec3 sunPosition;
	Mat4x4 worldToSunMatrix;
	Mat4x4 sunProjMatrix;
	Mat4x4 sunCameraMatrix;

	// 各3Dモデルを描画する
	void drawModels() const
	{
		// 地面の描画
		groundPlane.draw(groundTexture);

		// 球の描画
		Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

		// 鍛冶屋の描画
		blacksmithModel.draw(Vec3{ 8, 0, 4 });

		// 風車の描画
		millModel.draw(Vec3{ -8, 0, 4 });

		// 木の描画
		{
			const ScopedRenderStates3D renderStates{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			treeModel.draw(Vec3{ 16, 0, 4 });
			pineModel.draw(Vec3{ 16, 0, 0 });
		}

		// Siv3D くんの描画
		siv3dkunModel.draw(Vec3{ 2, 0, -2 }, Quaternion::RotateY(180_deg));
	}
};
