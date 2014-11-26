#ifndef VISUALIZATION_H
#define VISUALIZATION_H
#include <rfftw.h>              //the numerical simulation FFTW library
#include <math.h>               //for various math functions
#include <GL/glut.h>            //the GLUT graphics library
#include "model.h"
#include <string>
#include "GL/glui.h"

class Visualization {
private:
    void rgbToHSV(float* R,float* G,float* B, float* H, float* S, float* V);
    void hsvToRGB(float* R,float* G,float* B, float* H, float* S, float* V);
public:    
    //--- VISUALIZATION PARAMETERS ---------------------------------------------------------------------
    int color_dir;              //use direction color-coding or not
    int color_map_idx;             //method for scalar coloring
    int frozen;                 //toggles on/off the animation
    float vec_length;           //base length of hedgehogs
    float vec_base_length;
    float vec_scale;            //scale factor
    int drawMatter;
    int drawHedgehogs;
    int numColors;
    int limitColors;
    float saturation;
    float hue;
    int clamping;
    int glyph_type;
    enum{GLYPH_HEDGEHOGS, GLYPH_ARROWS};
    enum{COLOR_BLACKWHITE, COLOR_RAINBOW, COLOR_BIPOLAR};

    //------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------
    Visualization(int a_color_dir, int a_color_map_idx, int a_frozen, float a_vec_length) : color_dir(a_color_dir), color_map_idx(a_color_map_idx), frozen(a_frozen), vec_base_length(a_vec_length), vec_scale(1.0f), drawMatter(1),drawHedgehogs(0), limitColors(0), saturation(1.0f), hue(1.0f), clamping(1), glyph_type(GLYPH_ARROWS){
        vec_length = vec_base_length * vec_scale;
    }

    void visualize(Model* model);
    //rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
    void rainbow(float value, float* R, float* G, float* B);

    //diverging: Implements a color palette that diverges
    void bipolar(float value, float* R, float* G, float* B);

    //set_colormap: Sets three different types of colormaps
    void set_colormap(float vy);

    void display_text(float x, float y, char* const string);
    // Draw color legend
    void draw_color_legend(float minRho, float maxRho);

    //draw smoke
    void draw_smoke(fftw_real wn, fftw_real hn, Model* model);

    //draw velocities
    void draw_velocities(fftw_real wn, fftw_real hn, Model* model);

    void draw_arrow(int x_start, int y_start, int x_end, int y_end, float head_width);

    //direction_to_color: Set the current color by mapping a direction vector (x,y), using
    //                    the color mapping method 'method'. If method==1, map the vector direction
    //                    using a rainbow colormap. If method==0, simply use the white color
    void direction_to_color(float x, float y, int method);

    //Select next color profile
    void nextColor(){
        color_map_idx++;
        if (color_map_idx > COLOR_BIPOLAR)
        {
            color_map_idx = COLOR_BLACKWHITE;
        }
    }
    float clamp(float x);
    float scale(float x, fftw_real min, fftw_real max);
};

#endif