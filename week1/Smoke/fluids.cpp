// Usage: Drag with the mouse to add smoke to the fluid. This will also move a "rotor" that disturbs
//        the velocity field at the mouse location. Press the indicated keys to change options
//--------------------------------------------------------------------------------------------------
#include <iostream>
#include <GL/glui.h>
#include <chrono>
#include <thread>
#include "fluids.h"
#include "model.h"              //Simulation part of the application
#include "visualization.h"      //Visualization part of the application

const int DIM = 50;             //size of simulation grid
Model model(DIM);
Visualization vis(0, 0, 0, 1000.0f);

int window = -1; // Window ID for GLUT/GLUI

void printStart()
{
    std::cout << "Fluid Flow Simulation and Visualization" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Click and drag the mouse to steer the flow!" << std::endl;
    return;
}

//display: Handle window redrawing events. Simply delegates to visualize().
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    vis.visualize(&model);
    glFlush();
    glutSwapBuffers();
}

//reshape: Handle window resizing (reshaping) events
void reshape(int w, int h)
{
    int tx, ty, tw, th;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
    glViewport(tx, ty, tw, th);
    gluOrtho2D(0.0, (GLdouble)tw, 0.0, (GLdouble)th);
    model.winWidth = tw;
    model.winHeight = th;
}

// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void drag(int mx, int my)
{
    mx /= 0.8;
    int xi, yi, X, Y;
    double  dx, dy, len;
    static int lmx=0,lmy=0;             //remembers last mouse location

    // Compute the array index that corresponds to the cursor location
    xi = (int)model.clamp((double)(DIM + 1) * ((double)mx / (double)model.winWidth));
    yi = (int)model.clamp((double)(DIM + 1) * ((double)(model.winHeight - my) / (double)model.winHeight));

    X = xi;
    Y = yi;

    if (X > (DIM - 1))
    {
        X = DIM - 1;
    }
    if (Y > (DIM - 1))
    {
        Y = DIM - 1;
    }
    if (X < 0)
    {
        X = 0; 
    }
    if (Y < 0)
    {
        Y = 0;
    }

    // Add force at the cursor location
    my = model.winHeight - my;
    dx = mx - lmx;
    dy = my - lmy;
    len = sqrt(dx * dx + dy * dy);
    if (len != 0.0)
    {
        dx *= 0.1 / len;
        dy *= 0.1 / len;
    }
    model.fx[Y * DIM + X] += dx;
    model.fy[Y * DIM + X] += dy;
    model.rho[Y * DIM + X] = 10.0f;
    lmx = mx;
    lmy = my;
}

void do_one_step(void)
{
    if (!vis.frozen)
    {
        model.do_one_simulation_step(DIM);
        // Window has to be set explicitly, otherwise
        // the redisplay might be sent to the GLUI window
        // in stead of the GLUT window.
        glutSetWindow(window);
        glutPostRedisplay();
    }
    else
    {
        // Sleep, otherwise we use too much CPU.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// some controls generate a callback when they are changed
void glui_callback(int control)
{
    switch(control)
    {
        case HEDGEHOG_SPINNER_ID:
            vis.vec_length = vis.vec_base_length * vis.vec_scale;
            break;

        case VISCOSITY_SPINNER_ID:
            model.visc = model.base_visc * model.visc_scale_factor;

        case MIN_CLAMP_ID:
        case MAX_CLAMP_ID:
            minClamp->set_float_limits(0.0f, maxClamp->get_float_val());
            maxClamp->set_float_limits(minClamp->get_float_val(), 100.0f);
            break;

        default:
            break;
    }
    GLUI_Master.sync_live_all();
    glutPostRedisplay();
}

void create_GUI()
{
    // Function to create all glui GUI parts
    GLUI *glui = GLUI_Master.create_glui_subwindow(window, GLUI_SUBWINDOW_RIGHT);
    glui->set_main_gfx_window(window);
    GLUI_Rollout* generalRollout = glui->add_rollout("General", true);
    GLUI_Listbox *scalar_list = new GLUI_Listbox(generalRollout, "Scalar dataset", &(vis.scalar_dataset_idx), DATASET_ID, glui_callback);
    scalar_list->add_item(0, "Rho");
    scalar_list->add_item(1, "||Fluid velocity||");
    scalar_list->add_item(2, "||Force field||");

    GLUI_Listbox *vector_list = new GLUI_Listbox(generalRollout, "Vector dataset", &(vis.vector_dataset_idx), DATASET_ID, glui_callback);
    vector_list->add_item(1, "Fluid velocity");
    vector_list->add_item(2, "Force field");


    // Add several checkboxes
    new GLUI_Checkbox(generalRollout, "Frozen", &(vis.frozen), ANIMATE_ID, glui_callback);

    // Add spinners
    GLUI_Spinner* timestep_spinner = new GLUI_Spinner(generalRollout, "Timestep", GLUI_SPINNER_FLOAT, &(model.dt), TIMESTEP_SPINNER_ID, glui_callback);
    timestep_spinner->set_float_limits(0.0f, 1.0f);

    GLUI_Spinner* viscosity_spinner = new GLUI_Spinner(generalRollout, "Viscosity multiplier", GLUI_SPINNER_FLOAT, &(model.visc_scale_factor), VISCOSITY_SPINNER_ID, glui_callback);
    viscosity_spinner->set_float_limits(-1.0f, 100.0f);
    // Radio button for Scale / Clamp
    GLUI_Panel* scale_clamp_panel = new GLUI_Panel(generalRollout, "Dataset manipulation");
    GLUI_RadioGroup* scale_clamp = glui->add_radiogroup_to_panel(scale_clamp_panel, &(vis.clamping), SCALE_CLAMP_ID, glui_callback);
    glui->add_radiobutton_to_group( scale_clamp, "Scale");
    glui->add_radiobutton_to_group( scale_clamp, "Clamp");

    vis.min_clamp_value = 0.0f;
    vis.max_clamp_value = 1.0f;
    minClamp = new GLUI_Spinner(generalRollout, "Min clamp", GLUI_SPINNER_FLOAT, &(vis.min_clamp_value), MIN_CLAMP_ID, glui_callback);
    maxClamp = new GLUI_Spinner(generalRollout, "Max clamp", GLUI_SPINNER_FLOAT, &(vis.max_clamp_value), MAX_CLAMP_ID, glui_callback);
    minClamp->set_float_limits(0.0f, maxClamp->get_float_val());
    maxClamp->set_float_limits(minClamp->get_float_val(), 100.0f);
    maxClamp->set_speed(0.1);


    // SMOKE ROLLOUT
    GLUI_Rollout* smokeRollout = glui->add_rollout("Color map", false); 
    new GLUI_Checkbox(smokeRollout, "Draw matter", &(vis.drawMatter), DRAW_MATTER_ID, glui_callback);
  

    // Add listbox with color maps
    GLUI_Listbox *color_map_list = new GLUI_Listbox(smokeRollout, "Color map", &(vis.color_map_idx), COLOR_MAP_ID, glui_callback);
    color_map_list->add_item(0, "Black/White");
    color_map_list->add_item(1, "Rainbow");
    color_map_list->add_item(2, "Bipolar");

    new GLUI_Checkbox(smokeRollout, "Limit colors", &(vis.limitColors), LIMIT_COLORS_ID, glui_callback);
    GLUI_Spinner* numColors_spinner = new GLUI_Spinner(smokeRollout, "Number of colors", GLUI_SPINNER_INT, &(vis.numColors), NUM_COLOR_SPINNER_ID, glui_callback);
    numColors_spinner->set_int_limits(2, 256);

    GLUI_Spinner* hue_spinner = new GLUI_Spinner(smokeRollout, "Hue", GLUI_SPINNER_FLOAT, &(vis.hue), HUE_SPINNER_ID, glui_callback);
    hue_spinner->set_float_limits(0.0f, 1.0f);

    GLUI_Spinner* saturation_spinner = new GLUI_Spinner(smokeRollout, "Saturation", GLUI_SPINNER_FLOAT, &(vis.saturation), SATURATION_SPINNER_ID, glui_callback);
    saturation_spinner->set_float_limits(0.0f, 1.0f);

    GLUI_Rollout* glyphRollout = glui->add_rollout("Vectors", false);
    new GLUI_Checkbox(glyphRollout, "Draw glyphs", &(vis.drawHedgehogs), DRAW_HEDGEHOGS_ID, glui_callback);
    new GLUI_Checkbox(glyphRollout, "Direction coloring", &(vis.color_dir), DIRECTION_COLOR_ID, glui_callback);
    GLUI_Spinner* hedgehog_spinner = new GLUI_Spinner(glyphRollout, "Vector scale factor", GLUI_SPINNER_FLOAT, &(vis.vec_scale), HEDGEHOG_SPINNER_ID, glui_callback);
    hedgehog_spinner->set_float_limits(0.0f, 10.0f);

    GLUI_Listbox *glyph_location_list = new GLUI_Listbox(glyphRollout, "Glyph location", &(vis.glyph_location_idx), GLYPH_LOCATION_ID, glui_callback);
    glyph_location_list->add_item(0, "Uniform");
    glyph_location_list->add_item(1, "Random");
    glyph_location_list->add_item(2, "Jitter");

    vis.num_x_glyphs = model.DIM;
    vis.num_y_glyphs = model.DIM;
    GLUI_Spinner* xsamples = new GLUI_Spinner(glyphRollout, "X samples", GLUI_SPINNER_INT, &(vis.num_x_glyphs), X_GLYPH_SPINNER, glui_callback);
    xsamples->set_int_limits(0, 200);
    GLUI_Spinner* ysamples = new GLUI_Spinner(glyphRollout, "Y samples", GLUI_SPINNER_INT, &(vis.num_y_glyphs), Y_GLYPH_SPINNER, glui_callback);
    ysamples->set_int_limits(0, 200);

    GLUI_Listbox *glyph_shape_list = new GLUI_Listbox(glyphRollout, "Glyph shape", &(vis.glyph_shape), GLYPH_SHAPE_ID, glui_callback);
    glyph_shape_list->add_item(0, "Lines");
    glyph_shape_list->add_item(1, "Arrows");
    glyph_shape_list->add_item(2, "Triangles");

}


//main: The main program
int main(int argc, char **argv)
{   
    printStart();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1200,768);

    window = glutCreateWindow("Real-time smoke simulation and visualization");
    glutDisplayFunc(display);
    GLUI_Master.set_glutReshapeFunc(reshape);
    GLUI_Master.set_glutIdleFunc(do_one_step);
    glutMotionFunc(drag);
    create_GUI();

    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glLineWidth(2);

    glutMainLoop();         //calls do_one_simulation_step, keyboard, display, drag, reshape
    return 0;
}
