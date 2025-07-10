//#include <fstream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <iostream>
//#include <algorithm>
//#include <stdexcept>
//#include "model.h"
//
//Model::Model(const std::string filename) {
//    std::ifstream in(filename);
//    if (!in) {
//        throw std::runtime_error("Failed to open OBJ file: " + filename);
//    }
//
//    std::string line;
//    int line_no = 0;
//    while (std::getline(in, line)) {
//        ++line_no;
//        std::istringstream iss(line);
//        std::string prefix;
//        if (!(iss >> prefix)) continue; // empty or whitespace
//
//        if (prefix == "v") {
//            Vec3f v;
//            if (!(iss >> v[0] >> v[1] >> v[2])) {
//                std::cerr << "Warning: malformed vertex at line " << line_no << std::endl;
//                continue;
//            }
//            verts.push_back(v);
//        }
//        else if (prefix == "vt") {
//            Vec2f uv;
//            if (!(iss >> uv[0] >> uv[1])) {
//                std::cerr << "Warning: malformed texture coord at line " << line_no << std::endl;
//                continue;
//            }
//            texcoords.push_back(uv);
//        }
//        else if (prefix == "vn") {
//            Vec3f n;
//            if (!(iss >> n[0] >> n[1] >> n[2])) {
//                std::cerr << "Warning: malformed normal at line " << line_no << std::endl;
//                continue;
//            }
//            normals.push_back(n);
//        }
//        else if (prefix == "f") {
//            std::vector<int> fv, ft, fn;
//            std::string token;
//            while (iss >> token) {
//                std::replace(token.begin(), token.end(), '/', ' ');
//                std::istringstream fs(token);
//                int vi = -1, ti = -1, ni = -1;
//                fs >> vi;
//                if (fs.peek() == ' ') fs >> ti;
//                if (fs.peek() == ' ') fs >> ni;
//                if (vi < 1 || vi >(int)verts.size()) {
//                    throw std::runtime_error("Face vertex index out of range at line " + std::to_string(line_no));
//                }
//                fv.push_back(vi - 1);
//                if (ti != -1) {
//                    if (ti < 1 || ti >(int)texcoords.size()) {
//                        throw std::runtime_error("Face texture index out of range at line " + std::to_string(line_no));
//                    }
//                    ft.push_back(ti - 1);
//                }
//                if (ni != -1) {
//                    if (ni < 1 || ni >(int)normals.size()) {
//                        throw std::runtime_error("Face normal index out of range at line " + std::to_string(line_no));
//                    }
//                    fn.push_back(ni - 1);
//                }
//            }
//            if (fv.size() != 3) {
//                throw std::runtime_error("Non-triangulated face at line " + std::to_string(line_no));
//            }
//            facet_vrt.insert(facet_vrt.end(), fv.begin(), fv.end());
//            if (!ft.empty()) {
//                facet_tex.insert(facet_tex.end(), ft.begin(), ft.end());
//            }
//            if (!fn.empty()) {
//                facet_nrm.insert(facet_nrm.end(), fn.begin(), fn.end());
//            }
//        }
//        // else ignore other prefixes
//    }
//
//    if (facet_nrm.size() > 0 && facet_nrm.size() != facet_vrt.size())
//        throw std::runtime_error("Mismatch between face vertex and normal indices count");
//
//    if (facet_tex.size() > 0 && facet_tex.size() != facet_vrt.size()) {
//        throw std::runtime_error("Mismatch between face vertex and texture indices count");
//    }
//
//    std::cerr << "Loaded OBJ: " << verts.size() << " vertices, "
//        << texcoords.size() << " texcoords, "
//        << nfaces() << " faces." << std::endl;
//}
//
//int Model::nverts() const { return static_cast<int>(verts.size()); }
//int Model::ntexcoords() const { return static_cast<int>(texcoords.size()); }
//int Model::nfaces() const { return static_cast<int>(facet_vrt.size() / 3); }
//
//Vec3f Model::vert(int i) const {
//    if (i < 0 || i >= nverts()) throw std::out_of_range("Vertex index out of range");
//    return verts[i];
//}
//
//Vec3f Model::vert(int iface, int nthvert) const {
//    int idx = facet_vrt.at(iface * 3 + nthvert);
//    return vert(idx);
//}
//
//Vec2f Model::uv(int i) const {
//    if (i < 0 || i >= ntexcoords()) throw std::out_of_range("Texcoord index out of range");
//    return texcoords[i];
//}
//
//Vec2f Model::uv(int iface, int nthuv) const {
//    int idx = facet_tex.at(iface * 3 + nthuv);
//    return uv(idx);
//}
//
//int Model::nnormals() const {
//    return static_cast<int>(normals.size());
//}
//
//Vec3f Model::normal(int i) const {
//    if (i < 0 || i >= nnormals())
//        throw std::out_of_range("Normal index out of range");
//    return normals[i];
//}
//
//Vec3f Model::normal(int iface, int nthnorm) const {
//    int idx = facet_nrm.at(iface * 3 + nthnorm);
//    return normal(idx);
//}

#include <iostream>
#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0;i < 3;i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0;i < 3;i++) iss >> n[i];
            norms_.push_back(n);
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0;i < 2;i++) iss >> uv[i];
            uv_.push_back(uv);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i = 0; i < 3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);
}

Model::~Model() {}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    std::vector<int> face;
    for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
    return face;
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert][0]];
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2f uvf) {
    Vec2i uv(uvf[0] * (diffusemap_.get_width()-1), uvf[1] * (diffusemap_.get_height()-1));
    return diffusemap_.get(uv[0], uv[1]);
}

Vec3f Model::normal(Vec2f uvf) {
    Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
    TGAColor c = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (int i = 0; i < 3; i++)
        res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
    return res;
}

Vec2f Model::uv(int iface, int nthvert) {
    return uv_[faces_[iface][nthvert][1]];
}

float Model::specular(Vec2f uvf) {
    Vec2i uv(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height());
    return specularmap_.get(uv[0], uv[1])[0] / 1.f;
}

Vec3f Model::normal(int iface, int nthvert) {
    int idx = faces_[iface][nthvert][2];
    return norms_[idx].normalize();
}