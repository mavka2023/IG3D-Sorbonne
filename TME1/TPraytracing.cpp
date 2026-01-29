#include "TPraytracing.h"

std::ostream& operator<<(std::ostream& out, const Color& c) {
    out << "Color(" << c.r << ", " << c.g << ", " << c.b << ")";
    return out;
}

Vector3 Vector3::operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
Vector3 Vector3::operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
Vector3 Vector3::operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
float Vector3::operator*(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

// Returns the cross product (perpendicular vector) of a and b
Vector3 Vector3::cross(const Vector3& a, const Vector3& b) {
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vector3 Vector3::normalize() const {
    float len = std::sqrt((*this) * (*this));
    return (len > 0) ? Vector3(x / len, y / len, z / len) : *this; 
}

std::ostream& operator<<(std::ostream& out, const Vector3& v) {
    out << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return out;
}

Vector3 Point3::operator-(const Point3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
Point3 Point3::operator+(const Vector3& vec) const { return Point3(x + vec.x, y + vec.y, z + vec.z); }

std::ostream& operator<<(std::ostream& out, const Point3& p) {
    out << "Point3(" << p.x << ", " << p.y << ", " << p.z << ")";
    return out;
}

void Image::setPixel(int x, int y, const Color &c) {
    if (x >= 0 && x < width && y >= 0 && y < height)
        pixels[y * width + x] = c;
}

void Image::save_in_ppm(const std::string &filename) const {
    std::ofstream file(filename);
    if (!file) { std::cerr << "Cannot open file\n"; return; }
    file << "P3\n" << width << " " << height << "\n255\n";
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const Color &c = pixels[y * width + x];
            int r = static_cast<int>(std::max(0.0f, std::min(c.r, 1.0f)) * 255);
            int g = static_cast<int>(std::max(0.0f, std::min(c.g, 1.0f)) * 255);
            int b = static_cast<int>(std::max(0.0f, std::min(c.b, 1.0f)) * 255);
            file << r << " " << g << " " << b << "   ";
        }
        file << "\n";
    }
}

Ray Camera::generateRay(float x, float y, int width, int height) const {
    const float PI = 3.1415926535f;
    Vector3 direction = (point - center).normalize();
    Vector3 right = Vector3::cross(direction, up).normalize();
    Vector3 trueUp = Vector3::cross(right, direction).normalize();
    float radAlpha = alpha * PI / 180.0f;
    float widthAspect = std::tan(radAlpha * 0.5f) * 2.0f * zmin;
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float heightAspect = widthAspect / aspect; 
    float upx = (x + 0.5f) / width - 0.5f;
    float vpx = - (y + 0.5f) / height + 0.5f;
    Vector3 pixelPos = direction * zmin + right * (upx * widthAspect) + trueUp * (vpx * heightAspect);
    return Ray(center, pixelPos.normalize());
}

UniformTexture::UniformTexture(Color diffuse, Color specular, float shiny, float ka) {
    props.kd = diffuse; props.ks = specular; props.shininess = shiny; props.ka = ka;
}

MaterialProperties UniformTexture::getProperties(const Point3& hitPoint) const { return props; }

Sphere::Sphere(const Point3& c, float r, TextureMaterial* mat) : Object(mat), center(c), radius(r) {}
//https://www.tutorialspoint.com/computer_graphics/ray_object_intersection_in_ray_tracing.htm
float Sphere::intersect(const Ray& ray) const {
    Vector3 oc = ray.point - center;
    float a = ray.direction * ray.direction;
    float b = (ray.direction * oc) * 2.0f;
    float c = (oc * oc) - (radius * radius);
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return -1.0f;
    return (-b - std::sqrt(discriminant)) / (2.0f * a);
}
//In three-dimensional space, a surface normal, or simply normal, to a surface at point P
//is a vector perpendicular to the tangent plane of the surface at P
Vector3 Sphere::normal(const Point3& hitPoint) const {
    Vector3 normalVec = hitPoint - center;
    return normalVec.normalize();
}

MaterialProperties Sphere::determingMaterial(const Point3& hitPoint) const {
    return material->getProperties(hitPoint);
}

Scene::~Scene() {
    for(auto obj : objects) delete obj;
    for(auto l : lights) delete l;
    delete camera;
}

Color ray_color(const Ray& r, Color background, Color foreground) {
    Vector3 unit_direction = r.direction.normalize();
    float t = 0.5f * (unit_direction.y + 1.0f);
    return Color(
        (1.0f - t) * background.r + t * foreground.r,
        (1.0f - t) * background.g + t * foreground.g,
        (1.0f - t) * background.b + t * foreground.b
    );
}

Color trace(const Ray& ray, const Scene& scene, int depth) {
    if (depth <= 0) return Color(0, 0, 0);

    Object* closestObject = nullptr;
    float minT = std::numeric_limits<float>::max();
 
    for (Object* obj : scene.objects) {
        float t = obj->intersect(ray);
        if (t > 0.0f && t < minT) {
            minT = t;
            closestObject = obj;
        }
    }

    if (closestObject != nullptr) {
        Point3 hitPoint = ray.point + (ray.direction * minT);
        Vector3 N = closestObject->normal(hitPoint);
        MaterialProperties props = closestObject->determingMaterial(hitPoint);
        Vector3 V = (scene.camera->center - hitPoint).normalize();

        // Ambient
        Color finalColor(props.kd.r * props.ka, props.kd.g * props.ka, props.kd.b * props.ka);

        for (Light* l : scene.lights) {
            Vector3 lightVec = l->position - hitPoint;
            float distanceToLight = std::sqrt(lightVec * lightVec);
            Vector3 L = lightVec.normalize();

            Ray shadowRay(hitPoint + (N * 0.008f), L);
            bool inShadow = false;

            for (Object* obj : scene.objects) {
                float tShadow = obj->intersect(shadowRay);
                if (tShadow > 0.0f && tShadow < distanceToLight) {
                    inShadow = true;
                    break; 
                }
            }

            if (inShadow) continue; 
            float dotDiff = std::max(0.0f, N * L);
            
            // Diffuse
            finalColor.r += props.kd.r * l->intensity.r * dotDiff;
            finalColor.g += props.kd.g * l->intensity.g * dotDiff;
            finalColor.b += props.kd.b * l->intensity.b * dotDiff;

            // Specular
            if (dotDiff > 0) {
                Vector3 R_spec = (N * (N * L) * 2.0f - L).normalize();
                float specFactor = std::pow(std::max(0.0f, V * R_spec), props.shininess);
                finalColor.r += props.ks.r * l->intensity.r * specFactor;
                finalColor.g += props.ks.g * l->intensity.g * specFactor;
                finalColor.b += props.ks.b * l->intensity.b * specFactor;
            }
        }
        if (props.ks.r > 0 || props.ks.g > 0 || props.ks.b > 0) {
        float dotIN = ray.direction * N;
        Vector3 reflectDir = (ray.direction - N * (2.0f * dotIN)).normalize();
        Point3 reflectOrigin = hitPoint + (N * 0.001f);
        Ray reflectedRay(reflectOrigin, reflectDir);

        Color reflectedColor = trace(reflectedRay, scene, depth - 1);
        float reflectionDampen = 0.2f; 

        finalColor.r += props.ks.r * reflectedColor.r * reflectionDampen;
        finalColor.g += props.ks.g * reflectedColor.g * reflectionDampen;
        finalColor.b += props.ks.b * reflectedColor.b * reflectionDampen;
        }
        return finalColor;
    }

    return ray_color(ray, Color(1.0f, 1.0f, 1.0f), Color(0.0f, 0.0f, 0.8f));
}
//This function shoots multiple rays per pixel, samples the colors, averages them, and returns the final anti-aliased pixel color.
Color getPixelColorAA(int x, int y, int width, int height, const Scene& scene, int samples, int maxDepth) {
    Color accumulatedColor(0, 0, 0);
    
    int sqrtSamples = std::sqrt(samples); 
    float step = 1.0f / sqrtSamples;

    for (int i = 0; i < sqrtSamples; ++i) {
        for (int j = 0; j < sqrtSamples; ++j) {
            float offsetX = (i + 0.5f) * step;
            float offsetY = (j + 0.5f) * step;

            Ray ray = scene.camera->generateRay(x + offsetX, y + offsetY, width, height);
            
            Color sampleColor = trace(ray, scene, maxDepth);
            accumulatedColor.r += sampleColor.r;
            accumulatedColor.g += sampleColor.g;
            accumulatedColor.b += sampleColor.b;
        }
    }

    return Color(accumulatedColor.r / samples, accumulatedColor.g / samples, accumulatedColor.b / samples);
}