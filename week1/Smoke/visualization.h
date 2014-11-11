#ifndef VISUALIZATION_H
#define VISUALIZATION_H

class Visualization {
public:
    //--- VISUALIZATION PARAMETERS ---------------------------------------------------------------------
    int   color_dir             = 0;    //use direction color-coding or not
    float vec_scale             = 1000; //scaling of hedgehogs
    const int COLOR_BLACKWHITE  = 0;    //different types of color mapping: black-and-white, rainbow, banded
    const int COLOR_RAINBOW     = 1;
    const int COLOR_BANDS       = 2;
    int   scalar_col            = 0;    //method for scalar coloring
    int   frozen                = 0;    //toggles on/off the animation

    //------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------
    //rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
    void rainbow(float value, float* R, float* G, float* B);

    //set_colormap: Sets three different types of colormaps
    void set_colormap(float vy);

    //draw smoke
    void draw_smoke(fftw_real wn, fftw_real hn, Model* model);

    //draw velocities
    void draw_velocities(fftw_real wn, fftw_real hn, Model* model);

    //direction_to_color: Set the current color by mapping a direction vector (x,y), using
    //                    the color mapping method 'method'. If method==1, map the vector direction
    //                    using a rainbow colormap. If method==0, simply use the white color
    void direction_to_color(float x, float y, int method);
};

#endif