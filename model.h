//#ifndef __MODEL_H__
//#define __MODEL_H__
//
//#include <vector>
//#include "geometry.h"
//
//class Model {
//    std::vector<Vec3f> verts = {};    // array of vertices
//    std::vector<Vec2f> texcoords = {}; //array of texture coordinates
//    std::vector<int> facet_vrt = {}; // per-triangle index in the above array
//    std::vector<int> facet_tex = {}; //per-triangle texture index
//    std::vector<Vec3f> normals;      // array of normals
//    std::vector<int> facet_nrm;      // per-triangle normal indices
//public:
//    Model(const std::string filename);
//    int nverts() const; // number of vertices
//    int ntexcoords() const; //number of texcoords
//    int nfaces() const; // number of triangles
//    int nnormals() const;
//    Vec3f vert(const int i) const;                          // 0 <= i < nverts()
//    Vec3f vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
//    Vec2f uv(int i) const;
//    Vec2f uv(int iface, int nthuv) const;
//    Vec3f normal(int i) const;                        // 0 <= i < nnormals()
//    Vec3f normal(int iface, int nthnorm) const;       // 0 <= iface < nfaces(), 0 <= nthnorm < 3
//};
//
//#endif //__MODEL_H__#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i> > faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    void load_texture(std::string filename, const char* suffix, TGAImage& img);
public:
    Model(const char* filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f normal(int iface, int nthvert);
    Vec3f normal(Vec2f uv);
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
    TGAColor diffuse(Vec2f uv);
    float specular(Vec2f uv);
    std::vector<int> face(int idx);
};
#endif //__MODEL_H__