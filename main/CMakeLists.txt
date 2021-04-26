idf_component_register(

    SRCS
    "esp3d.c"

    "pingo/math/mat3.c"
    "pingo/math/mat4.c"
    "pingo/math/vec2.c"
    "pingo/math/vec3.c"
    "pingo/math/vec4.c"
    "pingo/render/depth.c"
    "pingo/render/material.c"
    "pingo/render/mesh.c"
    "pingo/render/object.c"
    "pingo/render/pixel.c"
    "pingo/render/rasterizer.c"
    "pingo/render/renderable.c"
    "pingo/render/renderer.c"
    "pingo/render/scene.c"
    "pingo/render/sprite.c"
    "pingo/render/texture.c"
    "pingo/example/cube.c"
    "pingo/example/memorybackend.c"
    "pingo/example/teapot.c"
    "pingo/example/pingo_mesh.c"


    "pingo/backend/ttgobackend.c"

    "pingo/backend/tft_espi/tft.c"
    "pingo/backend/tft_espi/tftspi.c"
    "pingo/backend/tft_espi/spi_master_lobo.c"

    INCLUDE_DIRS
    "."
    "pingo/math"
    "pingo/render"
    "pingo/example"
    "pingo/backend"
    "pingo/backend/tft_espi"
    )

