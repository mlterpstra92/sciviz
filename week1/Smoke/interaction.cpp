#include "interaction.h"

//------ INTERACTION CODE STARTS HERE -----------------------------------------------------------------

//display: Handle window redrawing events. Simply delegates to visualize().
void Interaction::display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    visualize();
    glFlush();
    glutSwapBuffers();
}

//reshape: Handle window resizing (reshaping) events
void Interaction::reshape(int w, int h)
{
    glViewport(0.0f, 0.0f, (GLfloat)w, (GLfloat)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
    winWidth = w;
    winHeight = h;
}

//keyboard: Handle key presses
void Interaction::keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 't':
            dt -= 0.001;
            break;
        case 'T':
            dt += 0.001;
            break;
        case 'c':
            color_dir = 1 - color_dir;
            break;
        case 'S':
            vec_scale *= 1.2;
            break;
        case 's':
            vec_scale *= 0.8;
            break;
        case 'V':
            visc *= 5;
            break;
        case 'v':
            visc *= 0.2;
            break;
        case 'x':
            draw_smoke = 1 - draw_smoke;
            if (draw_smoke==0)
            {
                draw_vecs = 1;
            }
            break;
        case 'y':
            draw_vecs = 1 - draw_vecs;
            if (draw_vecs==0)
            {
                draw_smoke = 1;
            }
            break;
        case 'm':
            scalar_col++;
            if (scalar_col>COLOR_BANDS)
            {
                scalar_col = COLOR_BLACKWHITE;
            }
            break;
        case 'a':
            frozen = 1-frozen;
            break;
        case 'q':
            exit(0);
    }
}



// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void Interaction::drag(int mx, int my)
{
    int xi, yi, X, Y;
    double  dx, dy, len;
    static int lmx=0,lmy=0;             //remembers last mouse location

    // Compute the array index that corresponds to the cursor location
    xi = (int)clamp((double)(DIM + 1) * ((double)mx / (double)winWidth));
    yi = (int)clamp((double)(DIM + 1) * ((double)(winHeight - my) / (double)winHeight));

    X = xi; Y = yi;

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
    my = winHeight - my;
    dx = mx - lmx;
    dy = my - lmy;
    len = sqrt(dx * dx + dy * dy);
    if (len != 0.0)
    {
        dx *= 0.1 / len;
        dy *= 0.1 / len;
    }
    fx[Y * DIM + X] += dx;
    fy[Y * DIM + X] += dy;
    rho[Y * DIM + X] = 10.0f;
    lmx = mx;
    lmy = my;
}