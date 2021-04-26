#include <stdio.h>
#include "math.h"

#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"

//PINGO
#include "pingo/math/vec2.h"
#include "pingo/render/mesh.h"
#include "pingo/example/teapot.h"
#include "pingo/example/cube.h"
#include "pingo/example/pingo_mesh.h"
#include "pingo/backend/ttgobackend.h"
#include "pingo/render/renderer.h"
#include "pingo/render/texture.h"
#include "pingo/render/sprite.h"
#include "pingo/render/scene.h"
#include "pingo/render/object.h"
#include "pingo/math/mat3.h"

#define GPIO_BTN_L    0
#define GPIO_BTN_R    35

void app_main(void)
{
    Vec2i size = {  240, 135 };

    TTGOBackend mB;
    ttgoBackendInit(&mB, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &mB );
    rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
    renderer.clear = false;

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object cube1;
    cube1.mesh = &mesh_cube;
    sceneAddRenderable(&s, object_as_renderable(&cube1));
    cube1.material = 0;

    Object cube2;
    cube2.mesh = &mesh_cube;
    sceneAddRenderable(&s, object_as_renderable(&cube2));

    Object cube3;
    cube3.mesh = &mesh_cube;
    sceneAddRenderable(&s, object_as_renderable(&cube3));

    Object cube4;
    cube4.mesh = &mesh_cube;
    sceneAddRenderable(&s, object_as_renderable(&cube4));

    //TEXTURE FOR CUBE 2
    Texture tex;
    texture_init(&tex, (Vec2i){8,8}, malloc(8*8*sizeof(Pixel)));

    for (int i = 0; i < 8; i++)
        for (int y = 0; y < 8; y++)
            ((uint8_t *)tex.frameBuffer)[i * 8 + y ] = (i + y) % 2 == 0 ? 0xFF : 0x00;

    Material m;
    m.texture = &tex;
    cube1.material = &m;
    cube2.material = &m;
    cube3.material = &m;
    cube4.material = &m;

    Object tea;
    tea.mesh = &pingo_mesh;
    sceneAddRenderable(&s, object_as_renderable(&tea));
    tea.material = 0;

    float phi = 0;
    float phi2 = 0;
    Mat4 t;

    uint32_t frame = 0;
    uint32_t time0 = esp_timer_get_time();

    uint32_t frameRenderTotal = 0;
    while (1) {
        time0 = (uint32_t)esp_timer_get_time();

        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 16.0,(float)size.x / (float)size.y, 40.0);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,0,-9});
        Mat4 rotateDown = mat4RotateX(0.30); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //CUBE 1 TRANSFORM - Defines position and orientation of the object
        cube1.transform =  mat4RotateY(phi2);
        t = mat4Scale((Vec3f){1,1,1});
        cube1.transform = mat4MultiplyM(&cube1.transform, &t );
        t = mat4Translate((Vec3f){-3,0.0,0});
        cube1.transform = mat4MultiplyM(&cube1.transform, &t );

        //CUBE 2 TRANSFORM - Defines position and orientation of the object
        cube2.transform =  mat4Translate((Vec3f){3,0.0,0});
        t = mat4Scale((Vec3f){1,1,1});
        cube2.transform = mat4MultiplyM(&cube2.transform, &t );

        //CUBE 2 TRANSFORM - Defines position and orientation of the object
        cube3.transform =  mat4Translate((Vec3f){0,0,-3});
        t = mat4Scale((Vec3f){1,1,1});
        cube3.transform = mat4MultiplyM(&cube3.transform, &t );

        //CUBE 2 TRANSFORM - Defines position and orientation of the object
        cube4.transform =  mat4Translate((Vec3f){0,0,3});
        t = mat4Scale((Vec3f){1,1,1});
        cube4.transform = mat4MultiplyM(&cube4.transform, &t );

        //TEA TRANSFORM - Defines position and orientation of the object
        tea.transform = mat4RotateZ(3.142128);
        t =mat4RotateY(phi2);
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Scale((Vec3f){0.02,0.02,0.02});
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Translate((Vec3f){0,1,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4RotateZ(3.1421);
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4RotateY(3.14217/4);
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        // Rotate the whole scene a little...
        phi += 0.00 - (float)gpio_get_level(0) * 0.02;

        s.transform = mat4RotateY(phi);

        // Render the scene
        printf("F:%d | ", frame++);
        printf ( "PreRender %dus | ", (uint32_t)esp_timer_get_time()-time0 );

        time0 = esp_timer_get_time();

        rendererRender(&renderer);

        uint32_t FrameMs = esp_timer_get_time()-time0;
        frameRenderTotal += FrameMs;

        printf ( "Render %dms | ", FrameMs / 1000 );

        //time0 = esp_timer_get_time();

        //mB.backend.afterRender(&renderer, &mB);

        //uint32_t afterms = esp_timer_get_time()-time0;
        //frameRenderTotal += afterms;

        //printf ("AfterRender %dms |", afterms/1000 );
        printf ("Average %dms  ", (frameRenderTotal / frame) / 1000 );
        printf ("\n");
    }
}
