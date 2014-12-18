// Usage: Drag with the mouse to add smoke to the fluid. This will also move a "rotor" that disturbs
//        the velocity field at the mouse location. Press the indicated keys to change options
//--------------------------------------------------------------------------------------------------
#include <iostream>
#include <GL/glui.h>
#include <chrono>
#include <thread>
#include <sstream>
#include "fluids.h"
#include "model.h"              //Simulation part of the application
#include "visualization.h"      //Visualization part of the application

const int DIM = 50;             //size of simulation grid
Model model(DIM);
Visualization vis(0, vis.COLOR_RAINBOW, 0, 1000.0f);

int window = -1; // Window ID for GLUT/GLUI

void printStart()
{
    std::cout << "Fluid Flow Simulation and Visualization" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Click and drag the mouse to steer the flow!" << std::endl;
    return;
}

void calcFPS(int theTimeInterval = 1000, std::string theWindowTitle = "NONE")
{
    // Static values which only get initialised the first time the function runs
    static int t0Value       = glutGet(GLUT_ELAPSED_TIME); // Set the initial time to now
    static int    fpsFrameCount = 0;             // Set the initial FPS frame count to 0
    static double fps           = 0.0;           // Set the initial FPS value to 0.0

    // Get the current time in seconds since the program started (non-static, so executed every time)
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    // Calculate and display the FPS every specified time interval
    if ((currentTime - t0Value) > theTimeInterval)
    {
        // Calculate the FPS as the number of frames divided by the interval in seconds
        fps = ((double)(fpsFrameCount * 1000.0)) / (currentTime - t0Value);
        char buf[6];
        snprintf(buf, 6, "%5.2f", fps);
        std::string fpsStr = buf;

        // Append the FPS value to the window title details
        theWindowTitle += " | FPS: " + fpsStr;

        // Convert the new window title to a c_str and set it
        const char* pszConstString = theWindowTitle.c_str();
        glutSetWindowTitle(pszConstString);

        // Reset the FPS frame counter and set the initial time to be now
        fpsFrameCount = 0;
        t0Value = glutGet(GLUT_ELAPSED_TIME);
    }
    else // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
        fpsFrameCount++;
}

const float depth = 1000.0f;
const float dist = 0.5f * depth;
const float offX = 0.0f;
const float offY = 0.0f;

//display: Handle window redrawing events. Simply delegates to visualize().
void display(void)
{
    int tx, ty, tw, th;
    GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 0.0, dist + depth, 
              0.0, 0.0, 0.0, 
              0.0, 1.0, 0.0);
    // Translate the GL origin to the simulation middle
    // glTranslatef(0.5 * tw + offX, 0.5 * th + offY, 0);
    // Now rotate along the axis w.r.t. this origin
    glTranslatef(0.0f, 0.0f, dist - depth);
    glRotatef(-vis.x_rot, 1.0f, 0.0f, 0.0f);
    glRotatef(-vis.y_rot, 0.0f, 1.0f, 0.0f);
    glRotatef(-vis.z_rot, 0.0f, 0.0f, 1.0f);
    // Translate to the middle of the simulation coordinates.
    glTranslatef(-0.5 * tw + offX, -0.5 * th + offY, 0.0f);

    vis.visualize(&model);
    glFlush();
    calcFPS(1000, "Real-time smoke simulation and visualization");
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
    //gluOrtho2D(0.0, (GLdouble)tw, 0.0, (GLdouble)th);
    //glFrustum(0.0, (GLdouble)tw, 0.0, (GLdouble)th, -100, 100);
    gluPerspective(25.0f / vis.zoom, (GLdouble)tw / (GLdouble)th, 1.0f, 2500.0f);
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
    int oldNum = vis.numColors;
    switch(control)
    {
        case HEDGEHOG_SPINNER_ID:
            vis.vec_length = vis.vec_base_length * vis.vec_scale;
            break;

        case VISCOSITY_SPINNER_ID:
            model.visc = model.base_visc * model.visc_scale_factor;
            break;

        case MIN_CLAMP_ID:
        case MAX_CLAMP_ID:
            minClamp->set_float_limits(0.0f, maxClamp->get_float_val());
            maxClamp->set_float_limits(minClamp->get_float_val(), 100.0f);
            break;

        case HUE_SPINNER_ID:
        case SATURATION_SPINNER_ID:
            if(vis.useTextures)
                vis.create_textures();
            break;

        case LOWER_ISOLINES_VALUE_ID:
        case UPPER_ISOLINES_VALUE_ID:
            lower_iso_spinner->set_float_limits(0.0f, upper_iso_spinner->get_float_val());
            upper_iso_spinner->set_float_limits(lower_iso_spinner->get_float_val(), 5.0f);
            break;

        case DRAW_ISOLINES_ID:
            vis.multipleIsolines = 0;
            break;

        case LIMIT_COLORS_ID:
        case NUM_COLOR_SPINNER_ID:
            oldNum = vis.numColors;
            if(!vis.limitColors)
                vis.numColors = 256;
            vis.create_textures();
            vis.numColors = oldNum;  
            break;

        case ZOOM_ID:
            reshape(model.winWidth, model.winHeight);
        default:
            // Do no special actions
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
    scalar_list->add_item(3, "div Velocity");
    scalar_list->add_item(4, "div Force");

    GLUI_Listbox *vector_list = new GLUI_Listbox(generalRollout, "Vector dataset", &(vis.vector_dataset_idx), DATASET_ID, glui_callback);
    vector_list->add_item(1, "Fluid velocity");
    vector_list->add_item(2, "Force field");


    // Add several checkboxes
    new GLUI_Checkbox(generalRollout, "Frozen", &(vis.frozen), ANIMATE_ID, glui_callback);
    new GLUI_Checkbox(generalRollout, "Textures", &(vis.useTextures), TEXTURE_ID, glui_callback);

    // Add spinners

    GLUI_Spinner* viscosity_spinner = new GLUI_Spinner(generalRollout, "Viscosity multiplier", GLUI_SPINNER_FLOAT, &(model.visc_scale_factor), VISCOSITY_SPINNER_ID, glui_callback);
    viscosity_spinner->set_float_limits(-1.0f, 100.0f);
    // Radio button for Scale / Clamp
    GLUI_Panel* scale_clamp_panel = new GLUI_Panel(generalRollout, "Dataset manipulation");
    GLUI_RadioGroup* scale_clamp = glui->add_radiogroup_to_panel(scale_clamp_panel, &(vis.clamping), SCALE_CLAMP_ID, glui_callback);
    glui->add_radiobutton_to_group( scale_clamp, "Scale");
    glui->add_radiobutton_to_group( scale_clamp, "Clamp");

    minClamp = new GLUI_Spinner(generalRollout, "Min clamp", GLUI_SPINNER_FLOAT, &(vis.min_clamp_value), MIN_CLAMP_ID, glui_callback);
    maxClamp = new GLUI_Spinner(generalRollout, "Max clamp", GLUI_SPINNER_FLOAT, &(vis.max_clamp_value), MAX_CLAMP_ID, glui_callback);
    minClamp->set_float_limits(0.0f, maxClamp->get_float_val());
    maxClamp->set_float_limits(minClamp->get_float_val(), 100.0f);
    maxClamp->set_speed(0.1);

    // x rotation spinner
    GLUI_Spinner* x_rot_spinner = new GLUI_Spinner(generalRollout, "x rotation", GLUI_SPINNER_FLOAT, &(vis.x_rot), X_ROT_ID, glui_callback);
    x_rot_spinner->set_float_limits(0.0f, 360.0f);

    // y rotation spinner
    GLUI_Spinner* y_rot_spinner = new GLUI_Spinner(generalRollout, "y rotation", GLUI_SPINNER_FLOAT, &(vis.y_rot), Y_ROT_ID, glui_callback);
    y_rot_spinner->set_float_limits(0.0f, 360.0f);

    // z rotation spinner
    GLUI_Spinner* z_rot_spinner = new GLUI_Spinner(generalRollout, "z rotation", GLUI_SPINNER_FLOAT, &(vis.z_rot), Z_ROT_ID, glui_callback);
    z_rot_spinner->set_float_limits(0.0f, 360.0f);

    // z rotation spinner
    GLUI_Spinner* zoom_spinner = new GLUI_Spinner(generalRollout, "zoom", GLUI_SPINNER_FLOAT, &(vis.zoom), ZOOM_ID, glui_callback);
    zoom_spinner->set_float_limits(0.2f, 10.0f);


    // COLOR MAP ROLLOUT
    GLUI_Rollout* smokeRollout = glui->add_rollout("Color map", false); 
    new GLUI_Checkbox(smokeRollout, "Draw matter", &(vis.drawMatter), DRAW_MATTER_ID, glui_callback);
  

    // Add listbox with color maps
    GLUI_Listbox *color_map_list = new GLUI_Listbox(smokeRollout, "Color map", &(vis.color_map_idx), COLOR_MAP_ID, glui_callback);
    color_map_list->add_item(0, "Black/White");
    color_map_list->add_item(1, "Rainbow");
    color_map_list->add_item(2, "Bipolar");
    color_map_list->add_item(3, "Zebra");

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

    GLUI_Rollout *iso_ops_rollout = glui->add_rollout("Isolines", false);
    new GLUI_Checkbox(iso_ops_rollout, "Isolines", &(vis.drawIsolines), DRAW_ISOLINES_ID, glui_callback);
    GLUI_Spinner* iso_spinner = new GLUI_Spinner(iso_ops_rollout, "Isoline value", GLUI_SPINNER_FLOAT, &(vis.isoline_value), ISOLINES_VALUE_ID, glui_callback);
    iso_spinner->set_float_limits(0, 5);
    new GLUI_Checkbox(iso_ops_rollout, "Multiple isolines", &(vis.multipleIsolines), MULTIPLE_ISOLINES_ID, glui_callback);
    lower_iso_spinner = new GLUI_Spinner(iso_ops_rollout, "Lower Isoline limit", GLUI_SPINNER_FLOAT, &(vis.lower_isoline_value), LOWER_ISOLINES_VALUE_ID, glui_callback);
    lower_iso_spinner->set_float_limits(0, 5);
    upper_iso_spinner = new GLUI_Spinner(iso_ops_rollout, "Upper Isoline limit", GLUI_SPINNER_FLOAT, &(vis.upper_isoline_value), UPPER_ISOLINES_VALUE_ID, glui_callback);
    upper_iso_spinner->set_float_limits(0, 5);
    GLUI_Spinner* num_lines_spinner = new GLUI_Spinner(iso_ops_rollout, "No. of isolines", GLUI_SPINNER_INT, &(vis.num_isoline_value), NUM_ISOLINES_VALUE_ID, glui_callback);
    num_lines_spinner->set_int_limits(1, 20);

    GLUI_Rollout *heightplot_rollout = glui->add_rollout("Height plot", false);
    new GLUI_Checkbox(heightplot_rollout, "Height plot", &(vis.drawHeightplot), DRAW_ISOLINES_ID, glui_callback);
    GLUI_Listbox *height_scalar_list = new GLUI_Listbox(heightplot_rollout, "Height dataset", &(vis.height_dataset_idx), DATASET_ID, glui_callback);
    height_scalar_list->add_item(0, "Rho");
    height_scalar_list->add_item(1, "||Fluid velocity||");
    height_scalar_list->add_item(2, "||Force field||");
    height_scalar_list->add_item(3, "div Velocity");
    height_scalar_list->add_item(4, "div Force");

    GLUI_Spinner* height_spinner = new GLUI_Spinner(heightplot_rollout, "Height scale factor", GLUI_SPINNER_FLOAT, &(vis.height_scale), HEIGHT_SPINNER_ID, glui_callback);
    height_spinner->set_float_limits(0.0f, 500.0f);
    // Radio button for Scale / Clamp for height plot
    GLUI_Panel* height_scale_clamp_panel = new GLUI_Panel(heightplot_rollout, "Height dataset manipulation");
    GLUI_RadioGroup* height_scale_clamp = glui->add_radiogroup_to_panel(height_scale_clamp_panel, &(vis.heightClamping), HEIGHT_SCALE_CLAMP_ID, glui_callback);
    glui->add_radiobutton_to_group( height_scale_clamp, "Scale");
    glui->add_radiobutton_to_group( height_scale_clamp, "Clamp");
    minClamp = new GLUI_Spinner(heightplot_rollout, "Min clamp", GLUI_SPINNER_FLOAT, &(vis.min_height_clamp_value), MIN_HEIGHT_CLAMP_ID, glui_callback);
    maxClamp = new GLUI_Spinner(heightplot_rollout, "Max clamp", GLUI_SPINNER_FLOAT, &(vis.max_height_clamp_value), MAX_HEIGHT_CLAMP_ID, glui_callback);

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

    glEnable(GL_DEPTH_TEST);
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glLineWidth(2);
    vis.create_textures();

    glutMainLoop();         //calls do_one_simulation_step, keyboard, display, drag, reshape
    free(model.rho0);
    free(model.rho);
    free(model.fy);
    free(model.fx);
    free(model.vy0);
    free(model.vx0);
    free(model.vy);
    free(model.vx);

    return 0;
}
