#ifndef FLUIDS_H
#define FLUIDS_H
GLUI_Spinner* minClamp, *maxClamp, *lower_iso_spinner, *upper_iso_spinner;
enum {
	  ANIMATE_ID, 
	  DRAW_HEDGEHOGS_ID, 
	  DRAW_MATTER_ID, 
	  DIRECTION_COLOR_ID, 
	  NEXT_COLOR_ID, 
	  TIMESTEP_SPINNER_ID, 
	  HEDGEHOG_SPINNER_ID, 
	  VISCOSITY_SPINNER_ID, 
	  NUM_COLOR_SPINNER_ID, 
	  LIMIT_COLORS_ID,
	  SATURATION_SPINNER_ID, 
	  HUE_SPINNER_ID,
	  SCALE_CLAMP_ID,
	  COLOR_MAP_ID,
	  DATASET_ID,
	  GLYPH_LOCATION_ID,
	  X_GLYPH_SPINNER,
	  Y_GLYPH_SPINNER,
	  GLYPH_SHAPE_ID,
	  MIN_CLAMP_ID,
	  MAX_CLAMP_ID,
	  TEXTURE_ID,
	  DRAW_ISOLINES_ID,
	  ISOLINES_VALUE_ID,
	  MULTIPLE_ISOLINES_ID,
	  LOWER_ISOLINES_VALUE_ID,
	  UPPER_ISOLINES_VALUE_ID,
	  NUM_ISOLINES_VALUE_ID,
	  HEIGHT_SPINNER_ID
};

#endif