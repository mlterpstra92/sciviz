#include "visualization.h"
#include "model.h"
#include "GL/glui.h"
#include <iostream>

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)


//visualize: This is the main visualization function
void Visualization::visualize(Model* model)
{
    fftw_real  wn = (fftw_real)model->winWidth / (fftw_real)(model->DIM + 1)*0.8;   // Grid cell width
    fftw_real  hn = (fftw_real)model->winHeight / (fftw_real)(model->DIM + 1);  // Grid cell height
   	int dim = model->DIM * 2 * (model->DIM /2+1);

    if (drawMatter)
    {	
    	fftw_real* values;
    	fftw_real min, max;
    	// Scalar values
    	switch (scalar_dataset_idx)
    	{
    	case FLUID_VELOCITY:
    		// Calculate magnitudes
    		values = (fftw_real*)malloc(dim * sizeof(fftw_real));
    		for (int i = 0; i < dim; i++)
    		{
    			values[i] = ((fftw_real)sqrt(model->vx[i] * model->vx[i] + model->vy[i] * model->vy[i]));
    		}    			
    		min = model->min_velo;
    		max = model->max_velo;

    		break;
    	case FORCE_FIELD:
    		// Calculate magnitudes    
    		values = (fftw_real*)malloc(dim * sizeof(fftw_real));		
	    	for (int i = 0; i < dim; i++)
	    	{
	    		values[i] = ((fftw_real)sqrt(model->fx[i] * model->fx[i] + model->fy[i] * model->fy[i]));
	    	}
	    	min = model->min_force;
	    	max = model->max_force;

    		break;
    	case FLUID_DENSITY:
    	default:
    		values = model->rho;
    		min = model->min_rho;
    		max = model->max_rho;
    	}
        draw_smoke(wn, hn, model->DIM, values, min, max);
        if(!clamping)
        	draw_color_legend(min, max);
        else
        	draw_color_legend(min_clamp_value, max_clamp_value);
    }
    if (drawHedgehogs)
    {
    	// Vector values
    	fftw_real* direction_x;
    	fftw_real* direction_y;
		switch (vector_dataset_idx)
    	{
    	case FORCE_FIELD:
    		direction_x = model->fx;
    		direction_y = model->fy;
    		break;
    	case FLUID_VELOCITY:
    	default:
    		direction_x = model->vx;
    		direction_y = model->vy;
    	}
        draw_velocities(wn, hn, model->DIM, direction_x, direction_y);
    }
}

//rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
void Visualization::rainbow(float value,float* R,float* G,float* B)
{
	const float dx = 0.8;
	value = (6-2*dx)*value+dx;
	*R = fmax(0.0, (3-fabs(value-4)-fabs(value-5))/2);
	*G = fmax(0.0, (4-fabs(value-2)-fabs(value-4))/2);
	*B = fmax(0.0, (3-fabs(value-1)-fabs(value-2))/2);
}

//diverge: Implements a color pallete that diverges
void Visualization::bipolar(float value,float* R,float* G,float* B)
{
	*R = value * fmax(0.0, (3-fabs(value-1)-fabs(value-2))/2);
	*G = 0;
	*B = 1 - (value * fmax(0.0, (3-fabs(value-1)-fabs(value-2))/2));
}

//clamp: Clamp all values between 0 and 1
float Visualization::clamp(float x)
{
    if (x >= max_clamp_value) {
        return 1.0;
    } else if (x < min_clamp_value) {
        return 0.0;
    } else {
        return x;
    }
}

//Scale all values between the overall min and max values
float Visualization::scale(float x, fftw_real min, fftw_real max)
{
    return (x - min) / (max - min);;
}

//set_colormap: Sets three different types of colormaps
void Visualization::set_colormap(float value)
{
	// Create a color band when the limit Colors button is checked.
	if (limitColors == 1)
	{
		value *= numColors - 1;
		value = round(value); // Round value, otherwise only the max gets a different color
		value /= numColors  - 1;
	}
	float R = 0, G = 0, B = 0, H = 0, S = 0, V = 0;
	// Different Color maps
	switch(color_map_idx)
	{
	case COLOR_BLACKWHITE:
		R = G = B = value;
		break;
	case COLOR_RAINBOW:
		rainbow(value, &R, &G, &B);
		break;
	case COLOR_BIPOLAR:
		bipolar(value, &R, &G, &B);
		break;
	}
	// Save calculations when Hue AND Saturation are set to 1
	if (hue != 1.0 || saturation != 1.0)
	{   
		rgbToHSV(&R, &G, &B, &H, &S, &V);
		H *= hue;
		S *= saturation;
		hsvToRGB(&R, &G, &B, &H, &S, &V);
	}
	glColor3f(R,G,B);
}

// calc RGB values of from HSV values
void Visualization::hsvToRGB(float* R,float* G,float* B, float* H, float* S, float* V)
{
	if ((*S) == 0.0)
	{
		*R = *G = *B = *V;
	}
	else
	{
		int Hi = (int)floor((*H) / 60.0);
		float f = ((*H) / 60.0) - Hi; 
		float p = (*V)*(1.0 - (*S));
		float q = (*V)*(1.0 - f*(*S));
		float t = (*V)*(1.0 - ((1.0 - f)*(*S)));

		switch(Hi)
		{
			case 0:
				*R = (*V);
				*G = t;
				*B = p;
				break;
			case 1:
				*R = q;
				*G = (*V);
				*B = p;
				break;
			case 2:
				*R = p;
				*G = (*V);
				*B = t;
				break;
			case 3:
				*R = p;
				*G = q;
				*B = (*V);
				break;
			case 4:
				*R = t;
				*G = p;
				*B = (*V);
				break;
			case 5:
				*R = (*V);
				*G = p;
				*B = q;
				break;
		}
	}
}
 
// calc HSV values of rgbhsvColor from RGB values
void Visualization::rgbToHSV(float* R,float* G,float* B, float* H, float* S, float* V)
{	
	float maxValue=MAX(MAX(*R, *G), *B);
	float minValue=MIN(MIN(*R, *G), *B);

	float delta = maxValue - minValue;

	if (delta == 0.0)
		(*H) = 0.0;
	else {
		if ((*R) == maxValue){
			*H = round(60.0 * ((((*G) - (*B))/delta)));
		}
		else if((*G) == maxValue){
			*H = round(60.0 * ((((*B) - (*R))/delta) + 2));
		}
		else {
			*H = round(60.0 * ((((*R) - (*G))/delta) + 4));
		}

		if((*H) < 0.0)
			*H += 360.0;
		if((*H) >= 360.0)
			*H -= 360.0;
	}

	if (maxValue == 0.0)
		*S = 0.0;
	else
		*S = delta / maxValue;

	*V = maxValue;
}

//direction_to_color: Set the current color by mapping a direction vector (x,y), using
//                    the color mapping method 'method'. If method==1, map the vector direction
//                    using a rainbow colormap. If method==0, simply use the white color
void Visualization::direction_to_color(float x, float y, int method)
{
	float r,g,b,f;
	if (method)
	{
		f = atan2(y,x) / 3.1415927 + 1;
		r = f;
		if(r > 1) r = 2 - r;
		g = f + .66667;
		if(g > 2) g -= 2;
		if(g > 1) g = 2 - g;
		b = f + 2 * .66667;
		if(b > 2) b -= 2;
		if(b > 1) b = 2 - b;
	}
	else
	{ 
		r = g = b = 1; 
	}
	glColor3f(r,g,b);
}

// Display text in OpenGL
void Visualization::display_text(float x, float y, char* const string)
{
	char * ch;
    glRasterPos3f( x, y, 0.0 );

    for( ch = string; *ch; ch++ ) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)*ch );
    }
}

// Draw color legend
void Visualization::draw_color_legend(float min, float max)
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glBegin(GL_QUAD_STRIP);
	if (limitColors == 0)
	{
		numColors = 256;
	}
	float stepSize = (th / float(numColors));
	
	for (int i = 0; i < numColors; ++i)
	{
		set_colormap((float)(i * stepSize + ((float)(i + 1) / numColors) * stepSize) / th);
		glVertex2f(tw * 0.9, i * stepSize);
		glVertex2f(tw, i * stepSize);
		glVertex2f(tw * 0.9, (i + 1) * stepSize);
		glVertex2f(tw, (i + 1) * stepSize);
	}
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	int numbersToDraw = numColors > 10 ? 10 : numColors;
	for (int i = 0; i < numbersToDraw; ++i)
	{
		float value = ((float)i / numbersToDraw) * (max - min) + min;
		char* strVal = NULL;
		if (asprintf(&strVal, "%0.2f", value) == -1)
			cout << "Error printing to allocated sting.";

		display_text(tw * 0.9 - 50, (th / numbersToDraw) * i, strVal);

		// Display line for where the value is on the legend
		glBegin(GL_LINES);
		glVertex2f(tw * 0.9 - 5, (th / numbersToDraw) * i);
		glVertex2f(tw * 0.9, (th / numbersToDraw) * i);
		glEnd();
	}
	char* maxStr = NULL;
	if (asprintf(&maxStr, "%0.2f", max) == -1)
		cout << "Error printing to allocated string.";

	display_text(tw * 0.9 - 50, th - 18, maxStr);
	glBegin(GL_LINES);
	glVertex2f(tw * 0.9 - 5, th);
	glVertex2f(tw * 0.9, th - 1);
	glEnd();
}

// Draw smoke
void Visualization::draw_smoke(fftw_real wn, fftw_real hn, int DIM, fftw_real* values, fftw_real min, fftw_real max)
{
	int i, j;
    fftw_real vy0, vy1, vy2, vy3;
 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (j = 0; j < DIM - 1; j++)            //draw smoke
    {
        for (i = 0; i < DIM - 1; i++)
        {
            double px0 = wn + (fftw_real)i * wn;
            double py0 = hn + (fftw_real)j * hn;
            int idx0 = (j * DIM) + i;

            double px1 = wn + (fftw_real)i * wn;
            double py1 = hn + (fftw_real)(j + 1) * hn;
            int idx1 = ((j + 1) * DIM) + i;

            double px2 = wn + (fftw_real)(i + 1) * wn;
            double py2 = hn + (fftw_real)(j + 1) * hn;
            int idx2 = ((j + 1) * DIM) + (i + 1);

            double px3 = wn + (fftw_real)(i + 1) * wn;
            double py3 = hn + (fftw_real)j * hn;
            int idx3 = (j * DIM) + (i + 1);

            if (clamping == 1)
            {  // Clamp
                vy0 = clamp(values[idx0]);
                vy1 = clamp(values[idx1]);
                vy2 = clamp(values[idx2]);
                vy3 = clamp(values[idx3]);
            }
            else
            {  // Scale
                vy0 = scale(values[idx0], min, max);
                vy1 = scale(values[idx1], min, max);
                vy2 = scale(values[idx2], min, max);
                vy3 = scale(values[idx3], min, max);
            }
            set_colormap(vy0);    glVertex2f(px0, py0);
            set_colormap(vy1);    glVertex2f(px1, py1);
            set_colormap(vy2);    glVertex2f(px2, py2);

            set_colormap(vy0);    glVertex2f(px0, py0);
            set_colormap(vy2);    glVertex2f(px2, py2);
            set_colormap(vy3);    glVertex2f(px3, py3);
        }
    }
    glEnd();
    if (scalar_dataset_idx != FLUID_DENSITY)
    	free(values);
}

void Visualization::draw_velocities(fftw_real wn, fftw_real hn, int DIM, fftw_real* direction_x, fftw_real* direction_y)
{	
	int i, j;

	float x_scale_factor = ((float)DIM / num_x_glyphs);
	float y_scale_factor = ((float)DIM / num_y_glyphs);
	for (i = 0; i < num_x_glyphs; i++)
	{
		for (j = 0; j < num_y_glyphs; j++)
		{
			float x_start = (wn + (fftw_real)i * wn) * x_scale_factor;
			float y_start = (hn + (fftw_real)j * hn) * y_scale_factor;

			int floor_x_index = i * x_scale_factor;
			int ceil_x_index = (floor_x_index + 1) % DIM;
			int floor_y_index = j * y_scale_factor;
			int ceil_y_index = (floor_y_index + 1) % DIM;

			float alpha = (x_start - (wn + (fftw_real)floor_x_index * wn)) / ((float)wn);
			float beta = (y_start - (hn + (fftw_real)floor_y_index * hn)) / ((float)hn);
			float anti_alpha = 1.0 - alpha;
			float anti_beta = 1.0 - beta;

			float value_x = (anti_alpha * anti_beta * direction_x[floor_y_index * DIM + floor_x_index] + 
				             alpha      * beta      * direction_x[ceil_y_index * DIM + ceil_x_index] + 
				             anti_alpha * beta      * direction_x[floor_y_index * DIM + ceil_x_index] + 
				             alpha      * anti_beta * direction_x[ceil_y_index * DIM + floor_x_index]);

			float value_y = (anti_alpha * anti_beta * direction_y[floor_y_index * DIM + floor_x_index] + 
				             alpha      * beta      * direction_y[ceil_y_index * DIM + ceil_x_index] + 
				             anti_alpha * beta      * direction_y[floor_y_index * DIM + ceil_x_index] + 
				             alpha      * anti_beta * direction_y[ceil_y_index * DIM + floor_x_index]);

			float x_end = x_start + vec_length * value_x;
			float y_end = y_start + vec_length * value_y;
			direction_to_color(value_x, value_y, color_dir);
			switch(glyph_shape)
			{
			case LINES:
				glBegin(GL_LINES);
				glVertex2f(x_start, y_start);
				glVertex2f(x_end, y_end);
				glEnd();
				break;
			case ARROWS:
				draw_arrow(x_start, y_start, x_end, y_end, 4);
				break;		  
			case TRIANGLES:
		  		draw_triangle(x_start, y_start, x_end, y_end);
		  		break;
			}
		}
	}
}
// Calculates angle from x_start, y_start, x_end and y_end in degrees
float Visualization::calc_angle(float x_dif, float y_dif)
{
	float angle;
	if (y_dif < 0) {
		angle = M_PI - atan(x_dif/y_dif);
	} else {
		angle = -atan(x_dif/y_dif);
	}
	// Convert angle from radians to degrees
	angle *= 180/M_PI;
	return angle;
}

void Visualization::draw_arrow(int x_start, int y_start, int x_end, int y_end, float head_width)
{
	float x_dif = x_end - x_start;
	float y_dif = y_end - y_start;
	float arrow_length = sqrt(x_dif * x_dif + y_dif * y_dif);
	float angle = calc_angle(x_dif, y_dif);
	// Translate and rotate to the arrow's origin and its angle
	glPushMatrix();
	glTranslatef(x_start, y_start, 0.0f);
	glRotatef(angle, 0.0, 0.0, 1.0);

	// We can now draw the arrow w.r.t. the origin
	glBegin(GL_LINES);	
	{
		glVertex2f(0, 0);
		glVertex2f(0, arrow_length);

		glVertex2f(0, arrow_length);
		glVertex2f(head_width, arrow_length - head_width);

		glVertex2f(0, arrow_length);
		glVertex2f(-head_width, arrow_length - head_width);
	}
	glEnd();

	// Pop the matrix (i.e. restore the previous matrix on the stack)
	glPopMatrix();

}

void Visualization::draw_triangle(int x_start, int y_start, int x_end, int y_end)
{
	/*float x_dif = x_end - x_start;
	float y_dif = y_end - y_start;
	float angle = calc_angle(x_dif, y_dif);*/
}
