#ifndef FLUIDS_H
#define FLUIDS_H
int direction_coloring, drawMatter, drawHedgehogs, scalarColoring, animate;
float timeStep, hedgehogScale, viscosityScale;
enum {ANIMATE_ID, SCALAR_COLORING_ID, DRAW_HEDGEHOGS_ID, DRAW_MATTER_ID, DIRECTION_COLOR_ID, NEXT_COLOR_ID, TIMESTEP_SPINNER_ID, HEDGEHOG_SPINNER_ID, VISCOSITY_SPINNER_ID};
/*int ANIMATE_ID=0;
int SCALAR_COLORING_ID=1;
int DRAW_HEDGEHOGS_ID=2;
int DRAW_MATTER_ID=3;
int DIRECTION_COLOR_ID=4;*/
void visualize();
#endif