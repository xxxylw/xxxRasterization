#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model* model = NULL;
float* shadowbuffer = NULL;

const int width = 800;
const int height = 800;

const Vec3f eye(-500, 200, 500);
const Vec3f center(0, 0, 0);
const Vec3f up(0, 1, 0);
const float eye_fov = 45.0f;
const float aspect_ratio = width / static_cast<float>(height);
float zNear = 0.1f;
float zFar = 1000.0f;

//blinn-phone
float Ka = 0.1f;
float Kd = 0.7f;
float Ks = 0.2f;
float shininess = 128.0f;
Vec3f light_pos(200, 500, 500);
Vec3f light_color(1.0f, 1.0f, 1.0f);

// Compute TBN for one triangle
void computeTangentBasis(
	const Vec3f& P0, const Vec3f& P1, const Vec3f& P2,
	const Vec2f& uv0, const Vec2f& uv1, const Vec2f& uv2,
	Vec3f& T, Vec3f& B)
{
	Vec3f dP1 = P1 - P0;
	Vec3f dP2 = P2 - P0;
	float du1 = uv1.x - uv0.x;
	float dv1 = uv1.y - uv0.y;
	float du2 = uv2.x - uv0.x;
	float dv2 = uv2.y - uv0.y;
	float r = 1.0f / (du1 * dv2 - du2 * dv1);
	T = (dP1 * dv2 - dP2 * dv1) * r;
	B = (dP2 * du1 - dP1 * du2) * r;
}

struct mYShader :public IShader {
	Vec3f varying_N[3];
	Vec3f varying_P[3];
	Vec3f varying_T[3];
	Vec3f varying_B[3];
	Vec2f varying_uv[3];

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec3f P = model->vert(iface, nthvert);
		Vec3f N = model->normal(iface, nthvert).normalize(); //problems
		Vec2f uv = model->uv(iface, nthvert);

		varying_P[nthvert] = P;
		varying_N[nthvert] = N;
		varying_uv[nthvert] = uv;

		Vec4f gl_Vertex = embed<4>(P);
		Vec4f clip = Projection * ModelView * gl_Vertex;
		return Viewport * (clip / clip[3]);
	}

	virtual bool fragment(Vec3f bar, TGAColor& color) {
		Vec3f P = varying_P[0] * bar.x + varying_P[1] * bar.y + varying_P[2] * bar.z;
		Vec3f N = (varying_N[0] * bar.x + varying_N[1] * bar.y + varying_N[2] * bar.z).normalize();
		Vec2f uv = varying_uv[0] * bar.x + varying_uv[1] * bar.y + varying_uv[2] * bar.z;
		Vec3f T = (varying_T[0] * bar.x + varying_T[1] * bar.y + varying_T[2] * bar.z);
		Vec3f B = (varying_B[0] * bar.x + varying_B[1] * bar.y + varying_B[2] * bar.z);

		T = (T - N * (N * T)).normalize();
		B = cross(N, T).normalize();

		Vec3f n_ts = model->normal(uv);
		mat<3, 3, float> TBN;
		TBN.set_col(0, T);
		TBN.set_col(1, B);
		TBN.set_col(2, N);
		Vec3f n_world = (TBN * n_ts).normalize();

		TGAColor diffc = model->diffuse(uv);

		float ks_tex = model->specular(uv) / 255.0f;
		const float Ks_global = 0.2f;

		Vec3f L = (light_pos - P).normalize();
		Vec3f V = (eye - P).normalize();
		Vec3f H = (L + V).normalize();
		float diff = std::max(0.f, n_world * L);
		float spec = std::pow(std::max(0.f, n_world * H), shininess);

		Vec3f ambient = Ka * light_color;
		Vec3f diffuse = Kd * diff * Vec3f(diffc.bgra[2] / 255.f, diffc.bgra[1] / 255.f, diffc.bgra[0] / 255.f);
		Vec3f specular = Ks_global * ks_tex * spec * light_color;
		Vec3f final_color = ambient + diffuse + specular;

		color = TGAColor(
			(unsigned char)(final_color.x * 255),
			(unsigned char)(final_color.y * 255),
			(unsigned char)(final_color.z * 255),
			255
		);
		//color = diffc;

		return false;
	}
};

//struct DepthShader : public IShader {
//	mat<3, 3, float> varying_tri;
//
//	DepthShader() : varying_tri() {}
//
//	virtual Vec4f vertex(int iface, int nthvert) {
//		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
//		Vec4f clip = Projection * ModelView * gl_Vertex;
//		gl_Vertex = Viewport * (clip / clip[3]);
//		varying_tri.set_col(nthvert, proj<3>(gl_Vertex));
//		return gl_Vertex;
//	}
//
//	virtual Vec4f fragment(Vec3f bar, TGAColor& color) {
//		Vec3f p = varying_tri * bar;
//		float d = 
//		return false;
//	}
//};

int main(int argc, char** argv) {
	model = (argc == 2 ? new Model(argv[1]) : new Model("obj/formula1/Substance_SpecGloss/Right_ones/Formula_1_mesh.obj"));

	TGAImage framebuffer(width, height, TGAImage::RGB);
	std::vector<float> depth_buf(width * height, std::numeric_limits<float>::max());
	//TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
	//for (int x = 0; x < width;x++) {
	//	for (int y = 0; y < height; y++) {
	//		zbuffer.set(x, y, TGAColor(255));
	//	}
	//}

	lookat(eye, center, up);
	viewport(height, width);

	float minD = +FLT_MAX, maxD = -FLT_MAX;
	for (int v = 0; v < model->nverts(); v++) {
		Vec3f P = model->vert(v);
		Vec4f P_cam = ModelView * embed<4>(P, 1.0f);
		minD = std::min(minD, -P_cam[2]);
		maxD = std::max(maxD, -P_cam[2]);
	}
	zNear = std::max(0.01f, minD * 0.8f);
	zFar = maxD * 1.2f;

	projection(eye_fov, aspect_ratio, zNear, zFar);



	mYShader shader;
	for (int i = 0; i < model->nfaces(); i++) {
		Vec3f P0 = model->vert(i, 0), P1 = model->vert(i, 1), P2 = model->vert(i, 2);
		Vec2f uv0 = model->uv(i, 0), uv1 = model->uv(i, 1), uv2 = model->uv(i, 2);

		Vec3f t0, b0, t1, b1, t2, b2;
		computeTangentBasis(P0, P1, P2, uv0, uv1, uv2, t0, b0);
		computeTangentBasis(P1, P2, P0, uv1, uv2, uv0, t1, b1);
		computeTangentBasis(P2, P0, P1, uv2, uv0, uv1, t2, b2);
		shader.varying_T[0] = t0; shader.varying_B[0] = b0;
		shader.varying_T[1] = t1; shader.varying_B[1] = b1;
		shader.varying_T[2] = t2; shader.varying_B[2] = b2;

		Vec4f screen_coords[3];

		for (int j = 0; j < 3;j++) {
			screen_coords[j] = shader.vertex(i, j);
		}

		triangle(screen_coords, shader, framebuffer, depth_buf);
	}

	framebuffer.flip_vertically();
	//zbuffer.flip_vertically();
	framebuffer.write_tga_file("framebuffer.tga");
	delete model;
	return 0;
}