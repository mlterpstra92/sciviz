// Usage: Drag with the mouse to add smoke to the fluid. This will also move a "rotor" that disturbs
//        the velocity field at the mouse location. Press the indicated keys to change options
//--------------------------------------------------------------------------------------------------
#include <iostream>
#include <GL/glui.h>
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
}

// some controls generate a callback when they are changed
void glui_callback(int control)
{
    switch(control)
    {
        case ANIMATE_ID:
            break;

        case DRAW_HEDGEHOGS_ID:
            if (vis.drawHedgehogs == 0)
                vis.drawMatter = 1;
            break;

        case DRAW_MATTER_ID:
            if (vis.drawMatter == 0)
                vis.drawHedgehogs = 1;
            break;

        case DIRECTION_COLOR_ID:
            break;

        case NEXT_COLOR_ID:
            vis.nextColor();
            break;

        case TIMESTEP_SPINNER_ID:
            
            break;
        case HEDGEHOG_SPINNER_ID:
            vis.vec_length = vis.vec_base_length * vis.vec_scale;

            break;

        case VISCOSITY_SPINNER_ID:
            model.visc = model.base_visc * model.visc_scale_factor;

        case NUM_COLOR_SPINNER_ID:
            break;

        case LIMIT_COLORS_ID:
            break;
        default:
            break;
    }
    GLUI_Master.sync_live_all();
    glutPostRedisplay();
}

//main: The main program
int main(int argc, char **argv)
{   
    printStart();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800,500);

    window = glutCreateWindow("Real-time smoke simulation and visualization");
    glutDisplayFunc(display);
    GLUI_Master.set_glutReshapeFunc(reshape);
    GLUI_Master.set_glutIdleFunc(do_one_step);
    glutMotionFunc(drag);

    // Make the GLUT window a subwindow of the GLUI window.
    GLUI *glui = GLUI_Master.create_glui_subwindow(window, GLUI_SUBWINDOW_RIGHT);
    glui->set_main_gfx_window(window);

    // Add a test checkbox. To see that it works.
    /*timeStep = 0.4f;
    viscosityScale = 1.0f;
    hedgehogScale = 1.0f;*/

    glui->add_checkbox("Direction coloring", &(vis.color_dir), DIRECTION_COLOR_ID, glui_callback);
    glui->add_checkbox("Draw matter", &(vis.drawMatter), DRAW_MATTER_ID, glui_callback);
    glui->add_checkbox("Draw hedgehogs", &(vis.drawHedgehogs), DRAW_HEDGEHOGS_ID, glui_callback);
    glui->add_checkbox("Frozen", &(vis.frozen), ANIMATE_ID, glui_callback);
    glui->add_button("Next color", NEXT_COLOR_ID, glui_callback);

    GLUI_Spinner* timestep_spinner = glui->add_spinner("Timestep", GLUI_SPINNER_FLOAT, &(model.dt), TIMESTEP_SPINNER_ID, glui_callback);
    timestep_spinner->set_float_limits(0.0f, 1.0f);

    GLUI_Spinner* hedgehog_spinner = glui->add_spinner("Hedgehog scale multiplier", GLUI_SPINNER_FLOAT, &(vis.vec_scale), HEDGEHOG_SPINNER_ID, glui_callback);
    hedgehog_spinner->set_float_limits(0.0f, 10.0f);

    GLUI_Spinner* viscosity_spinner = glui->add_spinner("Viscosity multiplier", GLUI_SPINNER_FLOAT, &(model.visc_scale_factor), VISCOSITY_SPINNER_ID, glui_callback);
    viscosity_spinner->set_float_limits(-1.0f, 100.0f);

    glui->add_checkbox("Limit colors", &(vis.limitColors), LIMIT_COLORS_ID, glui_callback);
    GLUI_Spinner* numColors_spinner = glui->add_spinner("Number of colors", GLUI_SPINNER_INT, &(vis.numColors), NUM_COLOR_SPINNER_ID, glui_callback);
    numColors_spinner->set_int_limits(2, 256);

    GLUI_Spinner* hue_spinner = glui->add_spinner("Hue", GLUI_SPINNER_FLOAT, &(vis.hue), HUE_SPINNER_ID, glui_callback);
    hue_spinner->set_float_limits(0.0f, 1.0f);

    GLUI_Spinner* saturation_spinner = glui->add_spinner("Saturation", GLUI_SPINNER_FLOAT, &(vis.saturation), SATURATION_SPINNER_ID, glui_callback);
    saturation_spinner->set_float_limits(0.0f, 1.0f);

    glutMainLoop();         //calls do_one_simulation_step, keyboard, display, drag, reshape
    return 0;
}
