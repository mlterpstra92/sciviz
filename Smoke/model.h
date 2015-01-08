#ifndef SIMULATION_H
#define SIMULATION_H
#include <rfftw.h>              //the numerical simulation FFTW library
#include <math.h>               //for various math functions
#include <cfloat>
#include <queue>
#include <list>
#include <iostream>

using namespace std;

typedef struct point3d {
    double x, y, z;
} Point3d;

typedef struct streamtube {
    Point3d seed;
    std::list<Point3d> tail;
} streamTube;

class Model {
public:
    Model (int);

    //--- SIMULATION PARAMETERS ------------------------------------------------------------------------
    int DIM;
    double dt;            //simulation time step
    float visc, base_visc, visc_scale_factor;          //fluid viscosity
    int winWidth, winHeight;          //size of the graphics window, in pixels
    std::deque<std::pair<fftw_real*, fftw_real*>> time_slices; // Time slices
    fftw_real *vx, *vy;             //(vx,vy)   = velocity field at the current moment
    fftw_real *vx0, *vy0;           //(vx0,vy0) = velocity field at the previous moment
    fftw_real *fx, *fy;             //(fx,fy)   = user-controlled simulation forces, steered with the mouse
    fftw_real *rho, *rho0;          //smoke density at the current (rho) and previous (rho0) moment
    fftw_real *copied_vx, *copied_vy, *copied_fx, *copied_fy; //pointer for copied values to store in queue
    fftw_real min_rho, max_rho;     // Min and max values of the 2d rho matrix
    fftw_real min_velo, max_velo;   // Min and max magnitudes of the velocities
    fftw_real min_force, max_force; // Min and max magnitudes of the forces
    fftw_real min_div, max_div; // Min and max magnitudes of the forces
    rfftwnd_plan plan_rc, plan_cr;  //simulation domain discretization
    std::list<streamTube> streamTubes;

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

    void streamtube_flow();
    void store_history();
    //do_one_simulation_step: Do one complete cycle of the simulation:
    //      - set_forces:
    //      - solve:            read forces from the user
    //      - diffuse_matter:   compute a new set of velocities
    //      - gluPostRedisplay: draw a new visualization frame
    void do_one_simulation_step(const int DIM);

    // Use interpolation to calculate the value of the vector field v at index coordinates (x, y)
    fftw_real interpolate(fftw_real *v, double x, double y);

};
#endif
