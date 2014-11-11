#ifndef SIMULATION_H
#define SIMULATION_H
#include <rfftw.h>              //the numerical simulation FFTW library
#include <math.h>               //for various math functions

class Model {
public:
    Model (int);

    //--- SIMULATION PARAMETERS ------------------------------------------------------------------------
    double dt;            //simulation time step
    float visc;          //fluid viscosity
    int winWidth, winHeight;          //size of the graphics window, in pixels
    fftw_real *vx, *vy;             //(vx,vy)   = velocity field at the current moment
    fftw_real *vx0, *vy0;           //(vx0,vy0) = velocity field at the previous moment
    fftw_real *fx, *fy;             //(fx,fy)   = user-controlled simulation forces, steered with the mouse
    fftw_real *rho, *rho0;          //smoke density at the current (rho) and previous (rho0) moment
    rfftwnd_plan plan_rc, plan_cr;  //simulation domain discretization

    //------ SIMULATION CODE STARTS HERE -----------------------------------------------------------------

    //FFT: Execute the Fast Fourier Transform on the dataset 'vx'.
    //     'direction' indicates if we do the direct (1) or inverse (-1) Fourier Transform
    void FFT(int direction, void* vx);

    //solve: Solve (compute) one step of the fluid flow simulation
    void solve(int n, fftw_real* vx, fftw_real* vy, fftw_real* vx0, fftw_real* vy0, fftw_real visc, fftw_real dt);

    int clamp(float x)
    {
        return ((x)>=0.0?((int)(x)):(-((int)(1-(x)))));
    }

    float max(float x, float y)
    {
        return x < y ? x : y;
    }

    // diffuse_matter: This function diffuses matter that has been placed in the velocity field. It's almost identical to the
    // velocity diffusion step in the function above. The input matter densities are in rho0 and the result is written into rho.
    void diffuse_matter(int n, fftw_real *vx, fftw_real *vy, fftw_real *rho, fftw_real *rho0, fftw_real dt);

    //set_forces: copy user-controlled forces to the force vectors that are sent to the solver.
    //            Also dampen forces and matter density to get a stable simulation.
    void set_forces(const int DIM);

    //do_one_simulation_step: Do one complete cycle of the simulation:
    //      - set_forces:
    //      - solve:            read forces from the user
    //      - diffuse_matter:   compute a new set of velocities
    //      - gluPostRedisplay: draw a new visualization frame
    void do_one_simulation_step(const int DIM);

};
//  Initialize simulation data structures as a function of the grid size 'n'.
//  Although the simulation takes place on a 2D grid, we allocate all data structures as 1D arrays,
//  for compatibility with the FFTW numerical library.

Model::Model (int n)
{
    int i;
    size_t dim;
    dt       = 0.4;
    visc     = 0.001;
    dim      = n * 2*(n/2+1)*sizeof(fftw_real);        //Allocate data structures
    vx       = (fftw_real*) malloc(dim);
    vy       = (fftw_real*) malloc(dim);
    vx0      = (fftw_real*) malloc(dim);
    vy0      = (fftw_real*) malloc(dim);
    dim      = n * n * sizeof(fftw_real);
    fx       = (fftw_real*) malloc(dim);
    fy       = (fftw_real*) malloc(dim);
    rho      = (fftw_real*) malloc(dim);
    rho0     = (fftw_real*) malloc(dim);
    plan_rc  = rfftw2d_create_plan(n, n, FFTW_REAL_TO_COMPLEX, FFTW_IN_PLACE);
    plan_cr  = rfftw2d_create_plan(n, n, FFTW_COMPLEX_TO_REAL, FFTW_IN_PLACE);

    for (i = 0; i < n * n; i++)                      //Initialize data structures to 0
    {
        vx[i] = vy[i] = vx0[i] = vy0[i] = fx[i] = fy[i] = rho[i] = rho0[i] = 0.0f;
    }
}

#endif