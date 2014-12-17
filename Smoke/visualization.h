#ifndef VISUALIZATION_H
#define VISUALIZATION_H
#include <rfftw.h>              //the numerical simulation FFTW library
#include <math.h>               //for various math functions
#include <GL/glut.h>            //the GLUT graphics library
#include "model.h"
#include <string>
#include <iostream>
#include "GL/glui.h"
#define NUM_COLORMAPS 4

using namespace std;

class Visualization {
private:
    void rgbToHSV(float R,float G,float B, float& H, float& S, float& V);
    void hsvToRGB(float& R,float& G,float& B, float H, float S, float V);
public:    
    //--- VISUALIZATION PARAMETERS ---------------------------------------------------------------------
    int color_dir;              //use direction color-coding or not
    int color_map_idx;             //method for scalar coloring
    int frozen;                 //toggles on/off the animation
    float vec_length;           //base length of hedgehogs
    float vec_base_length;
    float vec_scale;            //scale factor
    float height_scale;         //height scale factor
    int drawMatter;
    int drawHedgehogs;
    int numColors;
    int limitColors;
    float saturation;
    float hue;
    int scalar_dataset_idx;
    int vector_dataset_idx;
    int height_dataset_idx;
    int clamping;
    int glyph_location_idx;
    int num_x_glyphs, num_y_glyphs;
    int glyph_shape;
    int drawIsolines;
    int drawHeightplot;
    float isoline_value;
    int useTextures;
    float min_clamp_value, max_clamp_value;
    int multipleIsolines, num_isoline_value;
    float lower_isoline_value, upper_isoline_value;
    unsigned int texture_id[NUM_COLORMAPS];
    enum COLORMAP_TYPE {COLOR_BLACKWHITE = 0, COLOR_RAINBOW, COLOR_BIPOLAR, COLOR_ZEBRA};
    enum DATASET_TYPE {FLUID_DENSITY, FLUID_VELOCITY, FORCE_FIELD, DIVERGENCE_VELOCITY, DIVERGENCE_FORCE};
    enum SAMPLING_TYPE {UNIFORM, RANDOM, JITTER};
    enum GLYPH_TYPE {LINES, ARROWS, TRIANGLES};

    //------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------

    void determineValuesMinMax(Model* model, int dataset_idx, std::vector<fftw_real>& values, fftw_real *min, fftw_real* max);

    Visualization(int a_color_dir, int a_color_map_idx, int a_frozen, float a_vec_length) : color_dir(a_color_dir), color_map_idx(a_color_map_idx), frozen(a_frozen), vec_base_length(a_vec_length), vec_scale(1.0f), height_scale(1.0f), drawMatter(1),drawHedgehogs(0), numColors(256), limitColors(0), saturation(1.0f), hue(1.0f), clamping(0), glyph_shape(LINES), drawIsolines(0), isoline_value(0.06) {
        vec_length = vec_base_length * vec_scale;
    }

    void visualize(Model* model);
    //rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
    void rainbow(float value, float* R, float* G, float* B);

    //diverging: Implements a color palette that diverges
    void bipolar(float value, float* R, float* G, float* B);

    //Intervalling over a color
    void zebra(float value, float* R,float* G,float* B);

    //set_colormap: Sets three different types of colormaps
    void set_colormap(float value, float& R, float& G, float& B);

    void display_text(float x, float y, char* const string);
    // Draw color legend
    void draw_color_legend(float minRho, float maxRho);

    //draw smoke
    void draw_smoke(fftw_real wn, fftw_real hn, int DIM, std::vector<fftw_real> color_map_values, std::vector<fftw_real> height_values, fftw_real min, fftw_real max);

    //draw velocities
    void draw_velocities(fftw_real wn, fftw_real hn, int DIM, fftw_real* direction_x, fftw_real* direction_y);

    void divergence(fftw_real* f_x, fftw_real* f_y, std::vector<fftw_real>& grad, Model* model);

    float calc_angle(float x_dif, float y_dif);

    void draw_arrow(int x_start, int y_start, int x_end, int y_end, float head_width);

    void draw_triangle(int x_start, int y_start, int x_end, int y_end);

    //direction_to_color: Set the current color by mapping a direction vector (x,y), using
    //                    the color mapping method 'method'. If method==1, map the vector direction
    //                    using a rainbow colormap. If method==0, simply use the white color
    void direction_to_color(float x, float y, int method);

    void create_textures();

    float clamp(float x);
    float scale(float x, fftw_real min, fftw_real max);
    double interpolate(double v1, double v2, double iso);
};

#endif