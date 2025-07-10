#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"

#define MY_PI 3.1415
Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader() {}

const float EPSILON = 1e-5f;

void viewport(int height, int width) {
    Viewport = Matrix::identity();
    Viewport[0][0] = width / 2.0;
    Viewport[0][3] = width / 2.0;
    Viewport[1][1] = height / 2.0;
    Viewport[1][3] = height / 2.0;
    Viewport[2][2] = 255.0 / 2;
    Viewport[2][3] = 255.0 / 2;
    Viewport[3][3] = 1.0;
}

void projection(float eye_fov, float aspect_ratio, float zNear, float zFar) {
    //Projection = Matrix::identity();
    //Matrix M_persp_ortho = Matrix::identity();
    //M_persp_ortho[0][0] = zNear, M_persp_ortho[1][1] = zNear, M_persp_ortho[2][2] = zNear + zFar, M_persp_ortho[2][3] = -zNear * zFar,
    //    M_persp_ortho[3][2] = 1, M_persp_ortho[3][3] = 0;

    //Matrix M_ortho = Matrix::identity();
    //Matrix M_trans = Matrix::identity();
    //float alpha = 0.5 * eye_fov * MY_PI / 180.0;
    //float y_top = tan(alpha) * (-zNear);
    //float y_bottom = -y_top;
    //float x_right = aspect_ratio * y_top;
    //float x_left = -x_right;

    //M_trans[0][3] = -(x_right + x_left), M_trans[1][3] = -(y_top + y_bottom), M_trans[2][3] = -(zNear + zFar);
    //M_ortho[0][0] = 2 / (x_right - x_left), M_ortho[1][1] = 2 / (y_top - y_bottom), M_ortho[2][2] = 2 / (zNear - zFar);

    //M_ortho = M_ortho * M_trans;

    //Projection = M_ortho * M_persp_ortho;
    float alpha = 0.5 * eye_fov * MY_PI / 180.0;
    float y_top = tan(alpha) * zNear;
    float y_bottom = -y_top;
    float x_right = aspect_ratio * y_top;
    float x_left = -x_right;

    Projection = Matrix::identity();
    Projection[0][0] = (2.0f * zNear) / (x_right - x_left);
    Projection[1][1] = (2.0f * zNear) / (y_top - y_bottom);
    Projection[2][0] = (x_right + x_left) / (x_right - x_left);
    Projection[2][1] = (y_top + y_bottom) / (y_top - y_bottom);
    Projection[2][2] = -(zFar + zNear) / (zFar - zNear);
    Projection[2][3] = -2.0f * zFar * zNear / (zFar - zNear);
    Projection[3][2] = -1.0f;
}

void lookat(Vec3f eye, Vec3f target, Vec3f up) {
    Vec3f f = (target - eye).normalize();
    Vec3f s = cross(f, up).normalize();
    Vec3f u = cross(s, f).normalize();

    Matrix T = Matrix::identity();
    Matrix R = Matrix::identity();
    T[0][3] = -eye.x;
    T[1][3] = -eye.y;
    T[2][3] = -eye.z;

    R[0][0] = s.x, R[0][1] = s.y, R[0][2] = s.z;
    R[1][0] = u.x, R[1][1] = u.y, R[1][2] = u.z;
    R[2][0] = -f.x, R[2][1] = -f.y, R[2][2] = -f.z;

    ModelView = R * T;
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

float signed_area(const Vec2f& A, const Vec2f& B, const Vec2f& C) {
    return .5 * ((B.y - A.y) * (B.x + A.x) + (C.y - B.y) * (C.x + B.x) + (A.y - C.y) * (A.x + C.x));
}
//extern float signed_area(const Vec2f& A, const Vec2f& B, const Vec2f& C);

void triangle(Vec4f* pts, IShader& shader, TGAImage& framebuffer, std::vector<float>& depth_buf) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    bboxmin.x = std::max(0.0f, bboxmin.x);
    bboxmin.y = std::max(0.0f, bboxmin.y);
    bboxmax.x = std::min(width - 1.0f, bboxmax.x);
    bboxmax.y = std::min(height - 1.0f, bboxmax.y);

    Vec2i P;
    TGAColor color;

    Vec2f A2f = proj<2>(pts[0] / pts[0][3]);
    Vec2f B2f = proj<2>(pts[1] / pts[1][3]);
    Vec2f C2f = proj<2>(pts[2] / pts[2][3]);
    float triangle_area = signed_area(A2f, B2f, C2f);
    if (triangle_area <= 0) return;
    Vec3f c;

    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec2f P2f(P.x + 0.5f, P.y + 0.5f);
            c.x = signed_area(P2f, B2f, C2f) / triangle_area;
            c.y = signed_area(P2f, C2f, A2f) / triangle_area;
            c.z = signed_area(P2f, A2f, B2f) / triangle_area;

            if (c.x < 0 || c.y < 0 || c.z < 0) continue;

            float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            int frag_depth = z / w;
            if (depth_buf[P.x + P.y * framebuffer.get_width()]<frag_depth) continue;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                depth_buf[P.x + P.y * framebuffer.get_width()] = frag_depth;
                framebuffer.set(P.x, P.y, color);
            }
        }
    }
}