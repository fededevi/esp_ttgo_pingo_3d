#include <stdio.h>
#include "math.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

//PINGO
#include "pingo/math/vec2.h"
#include "pingo/render/mesh.h"
#include "pingo/example/teapot.h"
#include "pingo/example/cube.h"
#include "pingo/backend/ttgobackend.h"
#include "pingo/render/renderer.h"
#include "pingo/render/texture.h"
#include "pingo/render/sprite.h"
#include "pingo/render/scene.h"
#include "pingo/render/object.h"
#include "pingo/math/mat3.h"


void app_main(void)
{
    Vec2i size = {  240, 135 };

    TTGOBackend mB;
    ttgoBackendInit(&mB, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &mB );
    rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);


    Object tea;
    tea.mesh = &mesh_teapot;
    sceneAddRenderable(&s, object_as_renderable(&tea));
    tea.material = 0;

    float phi = 0;
    float phi2 = 0;
    Mat4 t;

    while (1) {
        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 2, 16.0,(float)size.x / (float)size.y, 50.0);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0,-5});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM - Defines position and orientation of the object
        tea.transform = mat4RotateZ(M_PI);
        t =mat4RotateY(phi2);
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Translate((Vec3f){0,-0.5,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        // Rotate the whole scene a little...
        s.transform = mat4RotateY(phi += 0.05);

        // Render the scene
        rendererRender(&renderer);

        //vTaskDelay(1/ portTICK_PERIOD_MS);
    }
}
