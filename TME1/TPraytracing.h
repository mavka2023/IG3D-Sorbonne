#ifndef TPRAYTRACING_H
#define TPRAYTRACING_H

#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <limits>
#include <string>
#include <algorithm>

// ===== Color =====
class Color {
public:
    float r, g, b;
    Color() : r(0.f), g(0.f), b(0.f) {}
    Color(float r_, float g_, float b_) : r(r_), g(g_), b(b_) {}
};

std::ostream& operator<<(std::ostream& out, const Color& c);

// ===== Vector3 =====
class Vector3 {
public:
    float x, y, z;
    Vector3() : x(0.f), y(0.f), z(0.f) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator-(const Vector3& other) const;
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator*(float scalar) const;
    float operator*(const Vector3& other) const;
    static Vector3 cross(const Vector3& a, const Vector3& b);
    Vector3 normalize() const;
};

std::ostream& operator<<(std::ostream& out, const Vector3& v);

// ===== Point3 =====
class Point3 {
public:
    float x, y, z;
    Point3() : x(0.f), y(0.f), z(0.f) {}
    Point3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Vector3 operator-(const Point3& other) const;
    Point3 operator+(const Vector3& vec) const;
};

std::ostream& operator<<(std::ostream& out, const Point3& p);

// ===== Ray =====
class Ray {
public: 
    Point3 point;
    Vector3 direction;
    Ray(const Point3& o, const Vector3& d) : point(o), direction(d) {}
};

// ===== Image =====
class Image {
public:
    int width, height;
    std::vector<Color> pixels;
    Image(int w, int h) : width(w), height(h), pixels(w * h) {}
    void setPixel(int x, int y, const Color &c);
    void save_in_ppm(const std::string &filename) const;
};

// ===== Light =====
class Light {
public:
    Color intensity;
    Point3 position;
    Light(const Color& inten, const Point3& pos) : intensity(inten), position(pos) {}
    virtual ~Light() = default;
};

class PointLight : public Light {
public:
    PointLight(const Color& inten, const Point3& pos) : Light(inten, pos) {}
};

// ===== Camera =====
class Camera {
public:
    Point3 center;
    Point3 point; 
    Vector3 up;
    float alpha, beta, zmin;
    Camera(const Point3& c, const Point3& p, const Vector3& u, float a, float b, float z)
        : center(c), point(p), up(u), alpha(a), beta(b), zmin(z) {}

    Ray generateRay(float x, float y, int width, int height) const;
};

// ===== Texture_Material =====
struct MaterialProperties {
    Color kd;
    Color ks;
    float shininess; 
    float ka;
};

class TextureMaterial {
public:
    virtual ~TextureMaterial() = default;
    virtual MaterialProperties getProperties(const Point3& hitPoint) const = 0;
};

class UniformTexture : public TextureMaterial {
private:
    MaterialProperties props;
public:
    UniformTexture(Color diffuse, Color specular, float shiny, float ka);
    MaterialProperties getProperties(const Point3& hitPoint) const override;
};

//=====Object =====
class Object {
public:
    TextureMaterial* material; 
    Object(TextureMaterial* mat) : material(mat) {}
    virtual ~Object() { delete material; }
    virtual float intersect(const Ray& ray) const = 0;
    virtual Vector3 normal(const Point3& hitPoint) const = 0;
    virtual MaterialProperties determingMaterial(const Point3& hitPoint) const = 0;
};

class Sphere : public Object {
private:
    Point3 center;
    float radius;    
public:
    Sphere(const Point3& c, float r, TextureMaterial* mat);
    float intersect(const Ray& ray) const override;
    Vector3 normal(const Point3& hitPoint) const override;
    MaterialProperties determingMaterial(const Point3& hitPoint) const override;
};

// ===== Scene =====
class Scene {
public:
    std::vector<Object*> objects;
    std::vector<Light*> lights;
    Camera* camera;
    Scene(Camera* cam) : camera(cam) {}
    ~Scene();
};

Color ray_color(const Ray& r, Color background, Color foreground);

Color trace(const Ray& ray, const Scene& scene);

Color getPixelColorAA(int x, int y, int width, int height, const Scene& scene, int samples);

#endif