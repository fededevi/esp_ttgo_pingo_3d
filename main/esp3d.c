

#include <stdio.h>

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
#include "pingo/example/memorybackend.h"
#include "pingo/render/renderer.h"
#include "pingo/render/texture.h"
#include "pingo/render/sprite.h"
#include "pingo/render/scene.h"
#include "pingo/render/object.h"
#include "pingo/math/mat3.h"

#include "tft_espi/tft.h"

#define BACKGROUND_COLOR    0
#define GRID_COLOR          0xFFFF

Pixel frameBuffer[DEFAULT_TFT_DISPLAY_WIDTH][DEFAULT_TFT_DISPLAY_HEIGHT];
uint16_t copyBuffer[DEFAULT_TFT_DISPLAY_HEIGHT][DEFAULT_TFT_DISPLAY_WIDTH];

#define SPI_BUS TFT_VSPI_HOST

spi_lobo_device_handle_t spi;

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t e = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (e>>8) | (e<<8);
}

void app_main(void)
{
    tft_disp_type = DISP_TYPE_ST7789V;

    max_rdclock = 8000000;

    printf("TFT_PinsInit");
    TFT_PinsInit();

    spi_lobo_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,				// set SPI MISO pin
        .mosi_io_num=PIN_NUM_MOSI,				// set SPI MOSI pin
        .sclk_io_num=PIN_NUM_CLK,				// set SPI CLK pin
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz = DEFAULT_TFT_DISPLAY_HEIGHT * DEFAULT_TFT_DISPLAY_WIDTH * 2 + 8,
    };

    spi_lobo_device_interface_config_t devcfg={
        .clock_speed_hz=8000000,                // Initial clock out at 8 MHz
        .mode=0,                                // SPI mode 0
        .spics_io_num=-1,                       // we will use external CS pin
        .spics_ext_io_num=PIN_NUM_CS,           // external CS pin
        .flags=LB_SPI_DEVICE_HALFDUPLEX,        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    vTaskDelay(500 / portTICK_RATE_MS);
    printf("\r\n==============================\r\n");
    printf("TFT display DEMO, LoBo 11/2017\r\n");
    printf("==============================\r\n");
    printf("Pins used: miso=%d, mosi=%d, sck=%d, cs=%d\r\n", PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS);

    esp_err_t ret=spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
    assert(ret==ESP_OK);
    printf("SPI: display device added to spi bus (%d)\r\n", SPI_BUS);
    disp_spi = spi;

    ret = spi_lobo_device_select(spi, 1);
    assert(ret==ESP_OK);
    ret = spi_lobo_device_deselect(spi);
    assert(ret==ESP_OK);

    printf("SPI: attached display device, speed=%u\r\n", spi_lobo_get_speed(spi));
    printf("SPI: bus uses native pins: %s\r\n", spi_lobo_uses_native_pins(spi) ? "true" : "false");

    printf("SPI: display init...\r\n");
    TFT_display_init();
    printf("OK\r\n");

    max_rdclock = find_rd_speed();
    printf("SPI: Max rd speed = %u\r\n", max_rdclock);

    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
    printf("SPI: Changed speed to %u\r\n", spi_lobo_get_speed(spi));

    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(PORTRAIT_FLIP);
    TFT_invertDisplay(1);

    int count = 0;

    Vec2i size = {  DEFAULT_TFT_DISPLAY_HEIGHT, DEFAULT_TFT_DISPLAY_WIDTH};

    MemoryBackend mB;
    memoryBackendInit(&mB, &frameBuffer, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &mB );

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
        renderer.camera_projection = mat4Perspective( 2, 16.0,(float)size.x / (float)size.y, 50.0);

        //VIEW MATRIX
        Mat4 v = mat4Translate((Vec3f) { 0,0,-5});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );


        //TEA TRANSFORM
        tea.transform = mat4RotateZ(PI);
        t =mat4RotateY(phi2);
        tea.transform = mat4MultiplyM(&tea.transform, &t );
        t = mat4Translate((Vec3f){0,-0.5,0});
        tea.transform = mat4MultiplyM(&tea.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi += 0.01);

        rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
        wait_trans_finish(1);
        rendererRender(&renderer);

        int xOff = 52;
        int yOff = 40;
        int xSize = 135;
        int ySize = 240;

        for (int i = 0; i < DEFAULT_TFT_DISPLAY_HEIGHT; i++) {
            for (int j = 0; j < DEFAULT_TFT_DISPLAY_WIDTH; j++) {
                uint16_t v = (frameBuffer[j][i].g) ;
                copyBuffer[i][j] = color565(v,v,v);
            }
        }

        count++;
        disp_select();
        send_data2(xOff, yOff, xSize+xOff-1, yOff+ySize, xSize*ySize-1, &copyBuffer[0][0]);
        disp_deselect();

        vTaskDelay(1/ portTICK_PERIOD_MS);
    }
}
