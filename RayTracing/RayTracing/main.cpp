#include <DxLib.h>
#include<math.h>
#include"Geometry.h"
#include "main.h"
const int screen_width = 640;
const int screen_height = 480;



//ヒントになると思って、色々と関数を用意しておりますが
//別にこの関数を使わなければいけないわけでも、これに沿わなければいけないわけでも
//ありません。レイトレーシングができていれば構いません。

///レイ(光線)と球体の当たり判定
///@param ray (視点からスクリーンピクセルへのベクトル)
///@param sphere 球
///@param distance 交点までの距離
///@hint レイは正規化しといたほうが使いやすいだろう
bool IsHitRayAndObject(const Position3& eye, const Vector3& ray, const Sphere& sp, float &distance) {
	//レイが正規化済みである前提で…
	//
	//視点から球体中心へのベクトル(視線)を作ります
	//
	auto C = sp.pos - eye;
	//中心ベクトルと視線の内積をとります＝＞ベクトル長　=　射影
	auto R = Dot(C, ray);

	auto vec = ray * R;

	auto Vec = C - vec;

	if (Vec.Magnitude() < sp.radius)
	{
		// 当たっていたらdistanceを返す
		auto Magni = sqrt(sp.radius * sp.radius - Vec.Magnitude() * Vec.Magnitude());

		auto power = vec.Magnitude() - Magni;

		distance = power;
		return true;
	}
	else
	{
		return false;
	}



	return false;
}

///レイトレーシング
///@param eye 視点座標
///@param sphere 球オブジェクト(そのうち複数にする)
float Clamp(float val, float minVal = 0.0f, float maxVal = 1.0f)
{
	return max(min(val, maxVal), minVal);
}

// inVector 入射ベクトル
// nVector　当たった面の法線ベクトル（正規化済み）
// return 反射ベクトル
Vector3 ReflectVector(const Vector3& inVector, const Vector3& nVector)
{
	return inVector - nVector * 2 * (inVector * nVector);
}

struct Col
{
	unsigned char r, g, b;
	Col() :r(0), g(0), b(0) {};
	Col(unsigned char inr, unsigned char ing, unsigned char inb) :r(inr), g(ing), b(inb) {}
	void operator*=(float scale)
	{
		r = Clamp(r * scale, 0, 255);
		g = Clamp(g * scale, 0, 255);
		b = Clamp(b * scale, 0, 255);
	}

	void operator+=(const Col & incol)
	{
		r = Clamp(r + incol.r, 0, 255);
		g = Clamp(g + incol.g, 0, 255);
		b = Clamp(b + incol.b, 0, 255);
	}

	Col operator+(const Col & incol)
	{
		Col ret(r, g, b);
		ret.r = Clamp(ret.r + incol.r, 0, 255);
		ret.g = Clamp(ret.g + incol.g, 0, 255);
		ret.b = Clamp(ret.b + incol.b, 0, 255);
		return ret;
	}

	Col Max(const Col& col) {
		Col ret(r, g, b);
		ret.r = max(col.r, ret.r);
		ret.g = max(col.g, ret.g);
		ret.b = max(col.b, ret.b);
		return ret;
	}

	unsigned int getcol() const
	{
		return DxLib::GetColor(r, g, b);
	}
};

Col CheckColor(const Vector3 & intersept)
{
	bool flag = false;
	if ((((int)(intersept.x / 50) + (int)(intersept.z / 50)) % 2) == 0)
	{
		flag = true;
		//DrawPixel(x, y, GetColor(255, 255, 255));
		//return Col(255, 255, 255);
	}
	else
	{
		flag = false;
		//DrawPixel(x, y, GetColor(0, 0, 0));
		//return Col(0, 0, 0);
	}
	if (intersept.x <= 0)
	{
		flag = !flag;
	}
	if (intersept.z <= 0)
	{
		flag = !flag;
	}
	if (flag)
	{
		return Col(255, 255, 255);
	}
	else
	{
		return Col(200, 200, 255);
	}
	//return Col(0, 0, 0);
}

void RayTracing(const Position3& eye, const Sphere& sphere, const Plane& plane) {

	DrawBox(0, 0, screen_width, screen_height, GetColor(100,150,245), true);
	auto distancepl = Dot(eye, plane.normal) - plane.offset;

	for (int y = 0; y < screen_height; ++y) {//スクリーン縦方向
		for (int x = 0; x < screen_width; ++x) {//スクリーン横方向
			//①視点とスクリーン座標から視線ベクトルを作る
			//ここでrayを作る
			Vector3 pos = { static_cast<float>(x - screen_width / 2), static_cast<float>(screen_height / 2 - y) , 0 };
			Vector3 ray = pos - eye;


			//②正規化をする
			ray.Normalize();
			//③IsHitRay関数がTrueだったら塗りつぶす
			//※塗りつぶしはDrawPixel関数を使う。

		/*	float brightness = min(max(normal*toLight,0) + ambient, 1.0f);
*/
			float distance = 0.0f;
			auto Light = Vector3(1, -1, -1);
			Light.Normalize();
			if (IsHitRayAndObject(eye, ray, sphere, distance))
			{
				auto N = (ray * distance - (sphere.pos - eye)).Normalized();		

				// 反射ベクトル
				auto rLight = ReflectVector(Light, N);

				// pow関数 pow(元の数,乗数)
				float spec = pow(Clamp(Dot(rLight, -ray.Normalized())), 15.0f);

				auto dot = Dot(N, -Light);
				dot = max(dot + spec, 0.25f);

				Col difCol(255, 0, 220);
				Col speCol(255, 255, 255);
				Col ambCol(32, 32, 32);

				difCol *= dot;
				speCol *= spec;

				Col col = difCol + speCol;
				col = col.Max(ambCol);

				auto tmpPos = ray * distance + eye;//反射レイの起点

				auto tmpray = ReflectVector(ray, N);//反射

				//auto raypla = Dot(tmpPos, plane.normal);

				auto t = (Dot(tmpPos, plane.normal) - plane.offset) / Dot(-tmpray, plane.normal);

				//auto Tpos = tmpPos + tmpray * t;


				if (Dot(-tmpray, plane.normal) > 0)//当たっているかどうか)
				{
					auto aa = (tmpPos.y - plane.offset) / 100.0f;
					//交点座標を求める
					auto Tpos = tmpPos + tmpray * t;
					auto col = CheckColor(Tpos);
					DrawPixel(x, y, col.getcol());
					//DrawPixel(x, y, GetColor(255 * aa, 255 * aa, 255 * aa));
					continue;
				}

				//Color *= Clamp(dot);
				DrawPixel(x, y, GetColor(col.r, col.g, col.b));
			}
			else
			{
				if (Dot(-ray, plane.normal) > 0)
				{
					/*Plane pla(Position3(0, 1, 0), 5);
					auto a = Dot(eye, pla.normal) - pla.offset;
					auto b = Dot(-ray, pla.normal);
					auto t = a / b;
					auto as = eye + ray * t;

					if ((((int)(as.x) + (int)(as.z)) % 2) == 0)
					{
						DrawPixel(x, y, GetColor(255, 255, 255));
					}*/
					auto rayplane = Dot(-ray, plane.normal);
					auto t = distancepl / rayplane;

					auto intersept = eye + ray * t;

					auto as = CheckColor(intersept);
					// 影の処理
					//　今の色を半分にする。
					// 交点interseptからLightの逆ベクトルと球体が交点を持つかどうかを判別
					if (IsHitRayAndObject(intersept, -Light, sphere, distance))
					{
						as *= 0.5f;
						//DrawPixel(x, y, as.getcol());
						//colのカラー値を半分にする。
					}

					DrawPixel(x, y, as.getcol());
				}
			}
		}
	}

}

void DrawPixelWithFloat(int x, int y, float r, float g, float b)
{
	DrawPixel(x, y, GetColor(r * 0xff, g * 0xff, b * 0xff));
}

int main() {
	ChangeWindowMode(true);
	SetGraphMode(screen_width, screen_height, 32);
	SetMainWindowText(_T("1816045_田口ひかる"));
	DxLib_Init();
	//DrawBox(0, 0, screen_width, screen_height, GetColor(255,255,255), true);
	//RayTracing(Vector3(0, 0, 300), Sphere(100, Position3(0, 0, -100)), Plane(Vector3(0, 1, 0), -100));

	Sphere sphere = Sphere(100, Position3(0, 0, -100));

	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		ClsDrawScreen();

		if (CheckHitKey(KEY_INPUT_LEFT))
		{
			sphere.pos.x -= 5;
		}
		if (CheckHitKey(KEY_INPUT_RIGHT))
		{
			sphere.pos.x += 5;
		}
		if (CheckHitKey(KEY_INPUT_UP))
		{
			sphere.pos.y += 5;
		}
		if (CheckHitKey(KEY_INPUT_DOWN))
		{
			sphere.pos.y -= 5;
		}

		RayTracing(Vector3(0, 0, 300), sphere, Plane(Vector3(0, 1, 0), -100));

		ScreenFlip();
	}



	//WaitKey();

}


