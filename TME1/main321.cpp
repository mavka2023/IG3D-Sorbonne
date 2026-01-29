#include "TPraytracing.h"

int main() {
    Camera* cam = new Camera(Point3(0, 0, 0), Point3(0, 0, -5), Vector3(0, 1, 0), 90.0f, 90.0f, 1.0f);
    Scene scene(cam);

    scene.objects.push_back(new Sphere(Point3(-2, 0, -5), 1.0f, new UniformTexture(Color(1,0,0), Color(1,1,1), 10.0f, 0.1f)));
    scene.objects.push_back(new Sphere(Point3(0, 0, -5), 1.0f, new UniformTexture(Color(0,1,0), Color(1,1,1), 10.0f, 0.1f)));
    scene.objects.push_back(new Sphere(Point3(2, 0, -5), 1.0f, new UniformTexture(Color(0,0,1), Color(1,1,1), 10.0f, 0.1f)));

    Light* light = new PointLight(Color(1,1,1), Point3(2, 5, 0));
    scene.lights.push_back(light);

    Image img(800, 600);
    int samples = 64; 

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Color finalCol = getPixelColorAA(x, y, img.width, img.height, scene, samples);
            img.setPixel(x, y, finalCol);
    }
}
    img.save_in_ppm("output314_.ppm");
    return 0;
}