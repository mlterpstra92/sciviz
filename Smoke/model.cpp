#include "model.h"

//  Initialize simulation data structures as a function of the grid size 'n'.
//  Although the simulation takes place on a 2D grid, we allocate all data structures as 1D arrays,
//  for compatibility with the FFTW numerical library.

Model::Model (int n)
{
    int i;
    size_t dim;
    DIM      = n;
    dt       = 0.4;
    base_visc= 0.001;
    visc_scale_factor = 1.0f;
    visc     = base_visc*visc_scale_factor;
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

//FFT: Execute the Fast Fourier Transform on the dataset 'vx'.
//     'dirfection' indicates if we do the direct (1) or inverse (-1) Fourier Transform
void Model::FFT(int direction, void* vx)
{
    if (direction==1)
    {
        rfftwnd_one_real_to_complex(plan_rc,(fftw_real*)vx,(fftw_complex*)vx);
    }
    else
    {
        rfftwnd_one_complex_to_real(plan_cr,(fftw_complex*)vx,(fftw_real*)vx);
    }
}

//solve: Solve (compute) one step of the fluid flow simulation
void Model::solve(int n, fftw_real* vx, fftw_real* vy, fftw_real* vx0, fftw_real* vy0, fftw_real visc, fftw_real dt)
{
    fftw_real x, y, x0, y0, f, r, U[2], V[2], s, t, magnitude;
    int i, j, i0, j0, i1, j1;

    for (i=0;i<n*n;i++)
    {
        vx[i] += dt*vx0[i]; vx0[i] = vx[i]; vy[i] += dt*vy0[i]; vy0[i] = vy[i];
    }

    for ( x= 0.5f/n,i=0; i<n; i++, x += 1.0f / n )
    {
        for ( y=0.5f/n,j=0 ; j<n ; j++,y+=1.0f/n )
        {
            x0 = n*(x-dt*vx0[i+n*j])-0.5f;
            y0 = n*(y-dt*vy0[i+n*j])-0.5f;
            i0 = clamp(x0); s = x0-i0;
            i0 = (n+(i0%n))%n;
            i1 = (i0+1)%n;
            j0 = clamp(y0); t = y0-j0;
            j0 = (n+(j0%n))%n;
            j1 = (j0+1)%n;
            vx[i+n*j] = (1-s)*((1-t)*vx0[i0+n*j0]+t*vx0[i0+n*j1])+s*((1-t)*vx0[i1+n*j0]+t*vx0[i1+n*j1]);
            vy[i+n*j] = (1-s)*((1-t)*vy0[i0+n*j0]+t*vy0[i0+n*j1])+s*((1-t)*vy0[i1+n*j0]+t*vy0[i1+n*j1]);
        }
    }

    for(i=0; i<n; i++)
    {
        for(j=0; j<n; j++)
        {
            vx0[i+(n+2)*j] = vx[i+n*j]; vy0[i+(n+2)*j] = vy[i+n*j];
        }
    }

    FFT(1,vx0);
    FFT(1,vy0);

    for (i=0;i<=n;i+=2)
    {
        x = 0.5f*i;
        for (j=0;j<n;j++)
        {
            y = j<=n/2 ? (fftw_real)j : (fftw_real)j-n;
            r = x*x+y*y;
            if ( r==0.0f )
            {
                continue;
            }
            f = (fftw_real)exp(-r*dt*visc);
            U[0] = vx0[i  +(n+2)*j];
            V[0] = vy0[i  +(n+2)*j];
            U[1] = vx0[i+1+(n+2)*j];
            V[1] = vy0[i+1+(n+2)*j];

            vx0[i  +(n+2)*j] = f*((1-x*x/r)*U[0]     -x*y/r *V[0]);
            vx0[i+1+(n+2)*j] = f*((1-x*x/r)*U[1]     -x*y/r *V[1]);
            vy0[i+  (n+2)*j] = f*(  -y*x/r *U[0] + (1-y*y/r)*V[0]);
            vy0[i+1+(n+2)*j] = f*(  -y*x/r *U[1] + (1-y*y/r)*V[1]);
        }
    }

    FFT(-1,vx0);
    FFT(-1,vy0);

    f = 1.0/(n*n);
    for (i=0;i<n;i++)
    {
        for (j=0;j<n;j++)
        {
            vx[i+n*j] = f*vx0[i+(n+2)*j]; 
            vy[i+n*j] = f*vy0[i+(n+2)*j];
            // Calculate the min and max magnitude of all velocities per timestep
            magnitude = sqrt(vx[i+n*j] * vx[i+n*j] + vy[i+n*j] * vy[i+n*j]);
            if (i == 0 && j == 0)
            {
                min_velo = FLT_MAX;
                max_velo = -FLT_MAX;
            }
            else if (magnitude < min_velo)
            {
                min_velo = magnitude;
            }
            else if (magnitude > max_velo)
            {
                max_velo = magnitude;
            }
        }
    }
}

void Model::streamtube_flow()
{
    for (auto streamtube = streamTubes.begin(); streamtube != streamTubes.end(); ++streamtube)
    {
        fftw_real dx, dy;
        Point3d previous;
        Point3d seed = (*streamtube).seed;
        previous = seed;
        (*streamtube).tail.clear();
        auto time_slice = time_slices.end();
        std::advance(time_slice, seed.z);
        for (; time_slice != time_slices.end(); ++time_slice)
        {
            Point3d current;
            // pair of vx, vy of certain time
            fftw_real* vel_x = (*time_slice).first;
            fftw_real* vel_y = (*time_slice).second;
            // Calculate dx and dy using interpolation
            dx = interpolate(vel_x, previous.x, previous.y);
            dy = interpolate(vel_y, previous.x, previous.y);
            // if(dx == FLT_MAX || dy == FLT_MAX)
            //     break;
            // newpoint = point + v(p)
            current.x = previous.x + dx * 10;
            current.y = previous.y + dy * 10;
            current.z = previous.z + 1;
            current.magnitude = dx*dx + dy*dy * 100000;
            current.magnitude = current.magnitude > 20 ? 20 : current.magnitude;
            current.magnitude = current.magnitude < 1 ? 1 : current.magnitude;
            // Insert the newly calculated point
            // TODO: OF TOCH FRONT????????????????
            (*streamtube).tail.push_back(current);
            previous = current;
        }
    }
}

void Model::store_history()
{
    // Store the last 50 timeframes, if queue exceeds 50 frames, pop first before push
    if (time_slices.size() >= 50)
    {
        auto tmppair = time_slices.front();
        free(tmppair.first);
        free(tmppair.second);
        time_slices.pop_front();
    }
    // Copy all current simulation velocities and push them in to the queue
    size_t dim = DIM * 2 * (DIM/2+1);
    copied_vx = (fftw_real*) malloc(dim * sizeof(fftw_real));
    copied_vy = (fftw_real*) malloc(dim * sizeof(fftw_real));
    std::copy(vx, vx + dim, copied_vx);
    std::copy(vy, vy + dim, copied_vy);
    auto pair = std::make_pair(copied_vx, copied_vy);
    time_slices.push_back(pair);
}
//do_one_simulation_step: Do one complete cycle of the simulation:
//      - set_forces:
//      - solve:            read forces from the user
//      - diffuse_matter:   compute a new set of velocities
//      - gluPostRedisplay: draw a new visualization frame
void Model::do_one_simulation_step(const int DIM)
{
    set_forces(DIM);
    solve(DIM, vx, vy, vx0, vy0, visc, dt);
    diffuse_matter(DIM, vx, vy, rho, rho0, dt);
    streamtube_flow();
    store_history();
}


// diffuse_matter: This function diffuses matter that has been placed in the velocity field. It's almost identical to the
// velocity diffusion step in the function above. The input matter densities are in rho0 and the result is written into rho.
void Model::diffuse_matter(int n, fftw_real *vx, fftw_real *vy, fftw_real *rho, fftw_real *rho0, fftw_real dt)
{
    fftw_real x, y, x0, y0, s, t, tmp;
    int i, j, i0, j0, i1, j1;
    for ( x=0.5f/n,i=0 ; i<n ; i++,x+=1.0f/n )
    {
        for ( y=0.5f/n,j=0 ; j<n ; j++,y+=1.0f/n )
        {
            x0 = n*(x-dt*vx[i+n*j])-0.5f;
            y0 = n*(y-dt*vy[i+n*j])-0.5f;
            i0 = clamp(x0);
            s = x0-i0;
            i0 = (n+(i0%n))%n;
            i1 = (i0+1)%n;
            j0 = clamp(y0);
            t = y0-j0;
            j0 = (n+(j0%n))%n;
            j1 = (j0+1)%n;
            tmp = (1-s)*((1-t)*rho0[i0+n*j0]+t*rho0[i0+n*j1])+s*((1-t)*rho0[i1+n*j0]+t*rho0[i1+n*j1]);
            // Calculate min and max rho values per timestep
            if (i == 0 && j == 0)
            {
                min_rho = FLT_MAX;
                max_rho = -FLT_MAX;
            }
            else if (tmp < min_rho)
            {
                min_rho = tmp;
            }
            else if (tmp > max_rho)
            {
                max_rho = tmp;
            }
            rho[i+n*j] = tmp;
        }
    }
}

//set_forces: copy user-controlled forces to the force vectors that are sent to the solver.
//            Also dampen forces and matter density to get a stable simulation.
void Model::set_forces(const int DIM)
{
    int i;
    fftw_real magnitude;
    for (i = 0; i < DIM * DIM; i++)
    {
        rho0[i]  = 0.995 * rho[i];
        fx[i] *= 0.85;
        fy[i] *= 0.85;
        vx0[i]    = fx[i];
        vy0[i]    = fy[i];
        // Calculate the min and max magnitude of all force fields per timestep
        magnitude = sqrt(fx[i] * fx[i] + fy[i] * fy[i]);
        if (i == 0)
        {
            min_force = FLT_MAX;
            max_force = -FLT_MAX;
        }
        else if (magnitude < min_force)
        {
            min_force = magnitude;
        }
        else if (magnitude > max_force)
        {
            max_force = magnitude;
        }
    }
}

// Use interpolation to calculate the value of the dataset v at index coordinates (x, y)
fftw_real Model::interpolate(fftw_real *v, double x, double y)
{
    int x_lower = floor(x);
    int x_upper = ceil(x);
    int y_lower = floor(y);
    int y_upper = ceil(y);

    if (x_lower > DIM || x_lower < 0 ||
        x_upper > DIM || x_upper < 0 ||
        y_lower > DIM || y_lower < 0 ||
        y_upper > DIM || y_upper < 0)
        return 0;

    // The fraction (between 0 and 1) of how far the point is in between the gridpoints
    double alpha_x = x - x_lower;
    double alpha_y = y - y_lower;

    fftw_real upper_left =  v[y_upper * DIM + x_lower];
    fftw_real upper_right = v[y_upper * DIM + x_upper];
    fftw_real lower_left =  v[y_lower * DIM + x_lower];
    fftw_real lower_right = v[y_lower * DIM + x_upper];

    // See http://en.wikipedia.org/wiki/Bilinear_interpolation#mediaviewer/File:Bilinear_interpolation_visualisation.svg
    fftw_real red =     lower_right * alpha_x       * (1 - alpha_y);
    fftw_real green =   lower_left  * (1 - alpha_x) * (1 - alpha_y);
    fftw_real blue =    upper_right * alpha_x       * alpha_y;
    fftw_real yellow =  upper_left  * (1 - alpha_x) * alpha_y;

    fftw_real value = red + green + blue + yellow;
    
    return value;
}
