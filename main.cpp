#include <Novice.h>
#include "Matrix.h"
#include <imgui.h>

const char kWindowTitle[] = "LD2A_11_ヨシダメイ";


const int kWindowWidth = 1280;
const int kWindowHeight = 720;

struct Sphere {
	Vector3 center;
	float radius;
};
struct Line
{
	Vector3 origin;
	Vector3 diff;
};
struct Ray
{
	Vector3 origin;
	Vector3 diff;
};
struct Segment
{
	Vector3 origin;
	Vector3 diff;
};

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;//グリッドの半分の幅
	const uint32_t kSubivision = 10;//分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubivision);
	//左から右に線を引く
	for (uint32_t zIndex = 0; zIndex <= kSubivision; ++zIndex) {
		Vector3 startPos = { kGridHalfWidth,0, kGridEvery * zIndex - kGridHalfWidth };
		Vector3 endPos = { -kGridHalfWidth,0, kGridEvery * zIndex - kGridHalfWidth };
		//スクリーン座標に変換
		Vector3 startNdcVertex = Transform(startPos, viewProjectionMatrix);
		startPos = Transform(startNdcVertex, viewportMatrix);
		Vector3 endNdcVertex = Transform(endPos, viewProjectionMatrix);
		endPos = Transform(endNdcVertex, viewportMatrix);
		//描画
		if (kGridEvery * zIndex - kGridHalfWidth != 0) {
			Novice::DrawLine(
				int(startPos.x), int(startPos.y),
				int(endPos.x), int(endPos.y), 0xAAAAAAFF);
		}
		else {
			Novice::DrawLine(
				int(startPos.x), int(startPos.y),
				int(endPos.x), int(endPos.y), 0x000000FF);
		}

	}
	//奥から手前に線を引く
	for (uint32_t xIndex = 0; xIndex <= kSubivision; ++xIndex) {
		Vector3 startPos = { kGridEvery * xIndex - kGridHalfWidth ,0,kGridHalfWidth };
		Vector3 endPos = { kGridEvery * xIndex - kGridHalfWidth,0,-kGridHalfWidth };
		//スクリーン座標に変換
		Vector3 startNdcVertex = Transform(startPos, viewProjectionMatrix);
		startPos = Transform(startNdcVertex, viewportMatrix);
		Vector3 endNdcVertex = Transform(endPos, viewProjectionMatrix);
		endPos = Transform(endNdcVertex, viewportMatrix);
		//描画
		if (kGridEvery * xIndex - kGridHalfWidth != 0) {
			Novice::DrawLine(
				int(startPos.x), int(startPos.y),
				int(endPos.x), int(endPos.y), 0xAAAAAAFF);
		}
		else {
			Novice::DrawLine(
				int(startPos.x), int(startPos.y),
				int(endPos.x), int(endPos.y), 0x000000FF);
		}
	}


}

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 20;
	const float kLonEvery = 2 * (float)M_PI / (float)kSubdivision;
	const float kLatEvery = (float)M_PI / (float)kSubdivision;
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float  lat = -1 * (float)M_PI / 2 + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;
			Vector3 a, b, c;
			a = { sphere.radius * std::cos(lat) * std::cos(lon), sphere.radius * std::sin(lat),  sphere.radius * std::cos(lat) * std::sin(lon) };
			a = Add(a, sphere.center);
			b = { sphere.radius * std::cos(lat + kLatEvery) * std::cos(lon),sphere.radius * std::sin(lat + kLatEvery),sphere.radius * std::cos(lat + kLatEvery) * std::sin(lon) };
			b = Add(b, sphere.center);
			c = { sphere.radius * std::cos(lat) * std::cos(lon + kLonEvery),sphere.radius * std::sin(lat),sphere.radius * std::cos(lat) * std::sin(lon + kLonEvery) };
			c = Add(c, sphere.center);
			//スクリーン座標変換
			a = Transform(a, viewProjectionMatrix);
			a = Transform(a, viewportMatrix);
			b = Transform(b, viewProjectionMatrix);
			b = Transform(b, viewportMatrix);
			c = Transform(c, viewProjectionMatrix);
			c = Transform(c, viewportMatrix);
			//描画
			Novice::DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
			Novice::DrawLine((int)a.x, (int)a.y, (int)c.x, (int)c.y, color);
		}
	}
}

Vector3 Project(const Vector3& v1, const Vector3& v2) {
	Vector3 v3, v4;
	float product;
	v4 = Normalize(v2);
	product = (v1.x * v4.x) + (v1.y * v4.y) + (v1.z * v4.z);
	v3 = Scaler(product, v4);
	return v3;
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 cp;
	cp = Add(segment.origin, point);
	return cp;
}

float Lengh(const Vector3& v) {
	float length;
	length = pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2);
	return length;
}

//球の衝突判定
bool IsCollision(const Sphere& s1,const Sphere& s2) {
	//二つの球の中心間の距離を求める
	float distance = Lengh(Subtract(s1.center, s2.center));
	//半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		return true;
	}
	else
	{
		return false;
	}
	
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 rotate = {};
	Vector3 translate = { 0,0,0 };
	Vector3 cameraPosition = { 0,1.9f,-6.49f };
	Vector3 cameraRotate = { 0.26f,0,0 };

	Sphere sphereA = { {0,0,0},1};
	Sphere sphereB = { {1,0,1},0.5f};
	uint32_t color = WHITE;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraPosition);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		if (IsCollision(sphereA, sphereB) == true) {
			color = RED;
		}
		else
		{
			color = WHITE;
		}
		

		if (keys[DIK_W]) {
			sphereA.center.z += 0.02f;
		}
		if (keys[DIK_S]) {
			sphereA.center.z -= 0.02f;
		}
		if (keys[DIK_A]) {
			sphereA.center.x -= 0.02f;
		}
		if (keys[DIK_D]) {
			sphereA.center.x += 0.02f;
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(worldViewProjectionMatrix, viewportMatrix);
		DrawSphere(sphereA, worldViewProjectionMatrix, viewportMatrix, color);
		DrawSphere(sphereB, worldViewProjectionMatrix, viewportMatrix, WHITE);

		//デバッグ
		ImGui::InputFloat3("sphereA.center", &sphereA.center.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat("sphereA.radius", &sphereA.radius);
		ImGui::InputFloat3("sphereB.center", &sphereB.center.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat("sphereB.radius", &sphereB.radius);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
