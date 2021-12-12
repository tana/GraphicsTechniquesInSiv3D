#pragma once

# include <Siv3D.hpp>
# include "utils.h"

// 分散シャドウマップ（Variance Shadow Map）によるソフトシャドウ
// ベースはチュートリアル37.1「3Dモデルを描く」（下記のURL）
//   https://zenn.dev/reputeless/books/siv3d-documentation/viewer/tutorial-3d-2#37.1-3d-%E3%83%A2%E3%83%87%E3%83%AB%E3%82%92%E6%8F%8F%E3%81%8F

// 描画時にシェーダに渡す情報
struct VSShadowMapping
{
	Mat4x4 sunCameraMatrix;
};

class VarianceShadowMap : public SceneManager<String>::Scene
{
public:
	VarianceShadowMap(const InitData& initData);

	void update() override;

	void draw() const override;

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
	void drawModels() const;
};
