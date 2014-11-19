#ifndef FLUIDS_H
#define FLUIDS_H
int direction_coloring, drawMatter, drawHedgehogs, scalarColoring, animate;
float timeStep, hedgehogScale, viscosityScale;
enum {ANIMATE_ID, SCALAR_COLORING_ID, DRAW_HEDGEHOGS_ID, DRAW_MATTER_ID, DIRECTION_COLOR_ID, NEXT_COLOR_ID, TIMESTEP_SPINNER_ID, HEDGEHOG_SPINNER_ID, VISCOSITY_SPINNER_ID};
void visualize();
#endif