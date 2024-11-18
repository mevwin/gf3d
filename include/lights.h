#ifndef __LIGHTS_H_
#define __LIGHTS_H_

#include "gf3d_mesh.h"
#include "gf3d_materials.h"

#define LIGHTS_MAX 16

typedef struct {

    GFC_Vector4D    lightPos;       // position
    GFC_Vector4D    lightDir;       // direction
    GFC_Vector4D    lightColor;     // color
    float           angle;
    float           brightness;
    float           falloff;
    float           inUse;         // if non zero, the light should be used
    /*
    ambientCoeff    how strong the ambient light is (generic light/side effect of all lights/simulates the scattering of irl lights)
    attenuation     how fast it falls off
    angle           if non, zero, spot light (work in radians)
    brightness

    additive colors: rby
    subtractive colors: cmyb

    emission lights: emitting light/color without actually emitting a light
   
    diffuse light: the color emitted
        - light should be additive, not subtractive
    
    specular: how shiny it is
    
    if light.position.w == 0.0
        directional light
    else
        point light:
            - surfaceToLight
            - distanceToLight
            - if spotlight
                    - angle is non-zero
                    - cone restrictions

    linear color:
        return ambient + attentuation*(diffuse + specular);
        final color is the summation of all calcs

    transparency pass:
        - first work with solid
        - do a second pass to draw transparent objects

        three.js
        shader toy
    */
}Light;

typedef struct {
    MeshUBO         mesh;
    MaterialUBO     material;
    Light           light[LIGHTS_MAX];
}LightUBO;

void lights_init();
void lights_close();
void light_free(Light* light);
Light* light_new();



#endif