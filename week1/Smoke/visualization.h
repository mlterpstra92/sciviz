#ifndef VISUALIZATION_H
#define VISUALIZATION_H
#include <rfftw.h>              //the numerical simulation FFTW library
#include <math.h>               //for various math functions
#include <GL/glut.h>            //the GLUT graphics library
#include "model.h"

class Visualization {
private:
    //--- VISUALIZATION PARAMETERS ---------------------------------------------------------------------
    int color_dir;              //use direction color-coding or not
    int COLOR_BLACKWHITE;       //different types of color mapping: black-and-white, rainbow, banded
    int COLOR_RAINBOW;
    int COLOR_BANDS;
    int COLOR_BIPOLAR;
    int scalar_col;             //method for scalar coloring
    int frozen;                 //toggles on/off the animation
    float vec_scale;            //scaling of hedgehogs

public:    
    //------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------
    Visualization(int a_color_dir, int a_scalar_col, int a_frozen, float a_vec_scale) : color_dir(a_color_dir), COLOR_BLACKWHITE(0), COLOR_RAINBOW(1), COLOR_BANDS(2), COLOR_BIPOLAR(3), scalar_col(a_scalar_col), frozen(a_frozen), vec_scale(a_vec_scale){}
    //rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
    void rainbow(float value, float* R, float* G, float* B);

    //diverging: Implements a color palette that diverges
    void bipolar(float value, float* R, float* G, float* B);

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

    // Returns true when animation is paused, false otherwise
    int isFrozen(){
        return frozen;
    }

    // Sets status to frozen if animation is enabled and vice versa
    void toggleFrozen(){
        frozen = 1 - frozen;
    }

    //Sets hedgehog color to black and white if color is used and vice versa
    void toggleDirectionColor(){
        color_dir = 1 - color_dir;
    }

    //Select next color profile
    void nextColor(){
        scalar_col++;
        if (scalar_col > COLOR_BIPOLAR)
        {
            scalar_col = COLOR_BLACKWHITE;
        }
    }

    // Scale the hedgehog length by a percentage
    // So putting scale = 1.2 makes the hedgehogs 20% larger
    void scaleHedgehogLength(float scale){
        vec_scale *= scale;
    }
};

#endif