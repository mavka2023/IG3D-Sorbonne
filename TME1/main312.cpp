#include "TPraytracing.h"

int main() {
    Camera* cam = new Camera(Point3(0, 0, 0), Point3(0, 0, -5), Vector3(0, 1, 0), 90.0f, 90.0f, 1.0f);
    Scene scene(cam);

    scene.objects.push_back(new Sphere(Point3(-2, 0, -5), 1.0f, new UniformTexture(Color(1,0,0), Color(1,1,1), 32.0f, 0.1f)));
    scene.objects.push_back(new Sphere(Point3(0, 0, -5), 1.0f, new UniformTexture(Color(0,1,0), Color(1,1,1), 32.0f, 0.1f)));
    scene.objects.push_back(new Sphere(Point3(2, 0, -5), 1.0f, new UniformTexture(Color(0,0,1), Color(1,1,1), 32.0f, 0.1f)));

    Light* light = new PointLight(Color(1,1,1), Point3(2, 5, 0));
    scene.lights.push_back(light);

    Image img(800, 600);

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Ray ray = scene.camera->generateRay(static_cast<float>(x), static_cast<float>(y), img.width, img.height);

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
                
                Color finalColor(0.0f, 0.0f, 0.0f);
                for (Light* l : scene.lights) {
                    Vector3 L = (l->position - hitPoint).normalize();
                    
                    // Diffuse
                    float dotDiff = std::max(0.0f, N * L);
                    finalColor.r += props.kd.r * l->intensity.r * dotDiff;
                    finalColor.g += props.kd.g * l->intensity.g * dotDiff;
                    finalColor.b += props.kd.b * l->intensity.b * dotDiff;
                }
                img.setPixel(x, y, finalColor);
            } else {
                img.setPixel(x, y, ray_color(ray, Color(1.0f, 1.0f, 1.0f), Color(0.0f, 0.0f, 0.8f)));
            }
        }
    }

    img.save_in_ppm("output312.ppm");
    return 0;
}