#include "TPraytracing.h"

int main() {
    Camera* cam = new Camera(Point3(0, 0, 0), Point3(0, 0, -5), Vector3(0, 1, 0), 60.0f, 60.0f, 1.0f);
    Scene scene(cam);

    TextureMaterial* redMat   = new UniformTexture(Color(0.7f, 0, 0), Color(0.3f, 0.3f, 0.3f), 100.0f, 0.05f);
    TextureMaterial* greenMat = new UniformTexture(Color(0, 0.7f, 0), Color(0.3f, 0.3f, 0.3f), 100.0f, 0.05f);
    TextureMaterial* blueMat  = new UniformTexture(Color(0, 0, 0.7f), Color(0.3f, 0.3f, 0.3f), 100.0f, 0.05f);
    TextureMaterial* floorMat = new UniformTexture(Color(0.2f, 0.2f, 0.2f), Color(0.1f, 0.1f, 0.1f), 10.0f, 0.05f);

    scene.objects.push_back(new Sphere(Point3(-2.2, 0, -6), 1.0f, redMat));
    scene.objects.push_back(new Sphere(Point3(0, 0, -6), 1.0f, greenMat));
    scene.objects.push_back(new Sphere(Point3(2.2, 0, -6), 1.0f, blueMat));
    scene.objects.push_back(new Sphere(Point3(0, -1001.0, -6), 1000.0f, floorMat));

    scene.lights.push_back(new PointLight(Color(1.0f, 1.0f, 1.0f), Point3(5, 5, 0)));
    scene.lights.push_back(new PointLight(Color(0.5f, 0.5f, 0.5f), Point3(-5, 3, 2)));

    Image img(800, 600);
    int samples = 4; 

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Color finalCol = getPixelColorAA(x, y, img.width, img.height, scene, samples, 5);
            img.setPixel(x, y, finalCol);
        }
    }

    img.save_in_ppm("output321.ppm");
    return 0;
}