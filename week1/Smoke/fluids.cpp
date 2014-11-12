// Usage: Drag with the mouse to add smoke to the fluid. This will also move a "rotor" that disturbs
//        the velocity field at the mouse location. Press the indicated keys to change options
//--------------------------------------------------------------------------------------------------

#include <stdio.h>              //for printing the help text
#include <GL/glut.h>            //the GLUT graphics library
#include <GL/glui.h>
#include "fluids.h"
#include "model.h"              //Simulation part of the application
#include "visualization.h"      //Visualization part of the application

const int DIM = 50;             //size of simulation grid
Model model(DIM);
Visualization vis(0, 0, 0, 1000);
int draw_smoke = 0;    //draw the smoke or not
int draw_vecs  = 1;    //draw the vector field or not

int window = -1; // Window ID for GLUT/GLUI

void printStart()
{
    printf("Fluid Flow Simulation and Visualization\n");
    printf("=======================================\n");
    printf("Click and drag the mouse to steer the flow!\n");
    printf("T/t:   increase/decrease simulation timestep\n");
    printf("S/s:   increase/decrease hedgehog scaling\n");
    printf("c:     toggle direction coloring on/off\n");
    printf("V/v:   increase decrease fluid viscosity\n");
    printf("x:     toggle drawing matter on/off\n");
    printf("y:     toggle drawing hedgehogs on/off\n");
    printf("m:     toggle thru scalar coloring\n");
    printf("a:     toggle the animation on/off\n");
    printf("q:     quit\n\n");
    return;
}

//display: Handle window redrawing events. Simply delegates to visualize().
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    visualize();
    glFlush();
    glutSwapBuffers();
}

//reshape: Handle window resizing (reshaping) events
void reshape(int w, int h)
{
    GLUI_Master.auto_set_viewport();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
    model.winWidth = w;
    model.winHeight = h;
}

//keyboard: Handle key presses
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 't':
            model.dt -= 0.001;
            break;
        case 'T':
            model.dt += 0.001;
            break;
        case 'c':
            vis.toggleDirectionColor();
            break;
        case 'S':
            vis.scaleHedgehogLength(1.2);
            break;
        case 's':
            vis.scaleHedgehogLength(0.8);
            break;
        case 'V':
            model.visc *= 5;
            break;
        case 'v':
            model.visc *= 0.2;
            break;
        case 'x':
            draw_smoke = 1 - draw_smoke;
            if (draw_smoke==0)
                draw_vecs = 1;

            break;
        case 'y':
            draw_vecs = 1 - draw_vecs;
            if (draw_vecs==0)
                draw_smoke = 1;

            break;
        case 'm':
            vis.nextColor();
            break;
        case 'a':
            vis.toggleFrozen();
            break;
        case 'q':
            exit(0);
    }
}



// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void drag(int mx, int my)
{
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
    if (!vis.isFrozen())
    {
        model.do_one_simulation_step(DIM);
        // Window has to be set explicitly, otherwise
        // the redisplay might be sent to the GLUI window
        // in stead of the GLUT window.
        glutSetWindow(window);
        glutPostRedisplay();
    }
}

//visualize: This is the main visualization function
void visualize()
{
    fftw_real  wn = (fftw_real)model.winWidth / (fftw_real)(DIM + 1);   // Grid cell width
    fftw_real  hn = (fftw_real)model.winHeight / (fftw_real)(DIM + 1);  // Grid cell height

    if (draw_smoke)
    {
        vis.draw_smoke(wn, hn, &model);
    }

    if (draw_vecs)
    {
        vis.draw_velocities(wn, hn, &model);
    }
}

//main: The main program
int main(int argc, char **argv)
{   
    printStart();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(500,500);

    window = glutCreateWindow("Real-time smoke simulation and visualization");
    glutDisplayFunc(display);
    GLUI_Master.set_glutReshapeFunc(reshape);
    GLUI_Master.set_glutIdleFunc(do_one_step);
    GLUI_Master.set_glutKeyboardFunc(keyboard);

    glutMotionFunc(drag);

    // Make the GLUT window a subwindow of the GLUI window.
    GLUI *glui = GLUI_Master.create_glui_subwindow(window, GLUI_SUBWINDOW_RIGHT);
    glui->set_main_gfx_window(window);

    // Add a test checkbox. To see that it works.
    int test;
    glui->add_checkbox("Test", &test);

    glutMainLoop();         //calls do_one_simulation_step, keyboard, display, drag, reshape
    return 0;
}
