#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

extern const int width;
extern const int height;

void viewport(int height, int width);
void projection(float eye_fov, float aspect_ratio, float zNear, float zFar);
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
	virtual ~IShader();
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, TGAImage& frambuffer, std::vector<float>& depth_buf);

#endif // __OUR_GL_H__
