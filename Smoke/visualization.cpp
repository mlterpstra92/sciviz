#include "visualization.h"
#include "model.h"
#include "GL/glui.h"
#include <iostream>
#define SETBIT(a, n) (a) |= (1 << (n))
#define UNSETBIT(a, n) (a) &= ~(1 << (n))
#define CHECKBIT(a, n) (a) & (1 << n)

void Visualization::determineValuesMinMax(Model* model, int dataset_idx, std::vector<fftw_real>& values, fftw_real *min, fftw_real* max)
{
	//determineValuesMinMax fills the values vector and determins min and max
	values.clear();
	int dim = model->DIM * 2 * (model->DIM /2+1);
	switch (dataset_idx)
	{
	case FLUID_VELOCITY:
		// Calculate magnitudes
		for (int i = 0; i < dim; i++)
		{
			values.push_back((fftw_real)sqrt(model->vx[i] * model->vx[i] + model->vy[i] * model->vy[i]));
		}    			
		*min = model->min_velo;
		*max = model->max_velo;

		break;
	case FORCE_FIELD:
		// Calculate magnitudes    
    	for (int i = 0; i < dim; i++)
    	{
    		values.push_back((fftw_real)sqrt(model->fx[i] * model->fx[i] + model->fy[i] * model->fy[i]));
    	}
    	*min = model->min_force;
    	*max = model->max_force;

		break;
	case DIVERGENCE_FORCE:
		divergence(model->fx, model->fy, values, model);
		*min = model->min_div;
		*max = model->max_div;
		break;
	case DIVERGENCE_VELOCITY:
		divergence(model->vx, model->vy, values, model);
		*min = model->min_div;
		*max = model->max_div;
		break;
	case FLUID_DENSITY:
	default:
		for(int i = 0; i < (model->DIM * model->DIM); ++i)
			values.push_back(model->rho[i]);

		*min = model->min_rho;
		*max = model->max_rho;
	}

}
//visualize: This is the main visualization function
void Visualization::visualize(Model* model)
{
    fftw_real  wn = (fftw_real)model->winWidth / (fftw_real)(model->DIM + 1)*0.8;   // Grid cell width
    fftw_real  hn = (fftw_real)model->winHeight / (fftw_real)(model->DIM + 1);  // Grid cell height

    if (drawMatter || drawIsolines)
    {	
    	std::vector<fftw_real> color_map_values;
    	std::vector<fftw_real> height_values(model->DIM * model-> DIM, 0);
    	fftw_real min_height = 0, max_height = 1;
    	// Scalar color_map_values
    	determineValuesMinMax(model, scalar_dataset_idx, color_map_values, &min, &max);
    	if (drawHeightplot)
    		determineValuesMinMax(model, height_dataset_idx, height_values, &min_height, &max_height);
        draw_smoke(wn, hn, model->DIM, color_map_values, height_values, min, max, min_height, max_height);
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
    if (enableStreamtubes)
    {
    	draw_streamtubes(wn, hn);
    }
}
//-----------COLOR MAPS ----------//
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
	*R = value ;
	*G = 0;
	*B = 1 - value;
}

void Visualization::zebra(float value, float* R,float* G,float* B)
{
	int val = std::min((int)(value * numColors), numColors - 1) & 1;
	*R = *G = *B = val;
}

//Scale all values between the overall min and max values
float Visualization::scale(float x, fftw_real min, fftw_real max)
{
    return (x - min) / (max - min);;
}

//clamp: Clamp all values between 0 and 1
float Visualization::clamp(float x, fftw_real min, fftw_real max)
{
    if (x >= max) {
        return 1.0;
    } else if (x < min) {
        return 0.0;
    } else {
        return x;
    }
}

void Visualization::create_textures(){
	int old_colormap_idx = color_map_idx;
	glGenTextures(NUM_COLORMAPS,texture_id);			//Generate 3 texture names, for the textures we will create
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);				//Make sure that OpenGL will understand our CPU-side texture storage format

	for(int i=0;i<NUM_COLORMAPS;++i)
	{													//Generate all three textures:
		glBindTexture(GL_TEXTURE_1D,texture_id[i]);		//Make i-th texture active (for setting it)
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

		float textureImage[3*numColors];

		color_map_idx = (COLORMAP_TYPE)i;								//Activate the i-th colormap

		for(int j=0;j<numColors;++j)							//Generate all 'size' RGB texels for the current texture:
		{
			float v = float(j)/(numColors-1);				//Compute a scalar value in [0,1]
			float R,G,B,H,S,V;
			switch(color_map_idx){
			case COLOR_BLACKWHITE:
				R = G = B = v;
				break;
			case COLOR_RAINBOW:
				rainbow(v, &R, &G, &B);
				break;
			case COLOR_BIPOLAR:
				bipolar(v, &R, &G, &B);
				break;		
			case COLOR_ZEBRA:
				zebra(v, &R, &G, &B);
				break;		
			default:
				R = G = B = 0;
				break;
			}

			if (hue != 1.0 || saturation != 1.0)
			{   
				rgbToHSV(R, G, B, H, S, V);
				H -= hue;
				// Poor man's mod because fmod works weird
				if (H < 0.0f)
					H += 1.0f;
				if (H > 1.0f)
					H -= 1.0f;
				S *= saturation;
				hsvToRGB(R, G, B, H, S, V);
			}

			textureImage[3*j]   = R;
			textureImage[3*j+1] = G;
			textureImage[3*j+2] = B;
		}	
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, numColors, 0, GL_RGB, GL_FLOAT, textureImage);
	}	
	
	color_map_idx = old_colormap_idx;
}

//set_colormap: Sets three different types of colormaps
void Visualization::set_colormap(float value, float& R, float& G, float& B)
{
	// Create a color band when the limit Colors button is checked.
	if (limitColors == 1)
	{
		value *= numColors - 1;
		value = round(value); // Round value, otherwise only the max gets a different color
		value /= numColors  - 1;
	}
	float H = 0, S = 0, V = 0;
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
	case COLOR_ZEBRA:
		zebra(value, &R, &G, &B);
		break;
	}
	// Save calculations when Hue AND Saturation are set to 1
	if (hue != 1.0 || saturation != 1.0)
	{   
		rgbToHSV(R, G, B, H, S, V);
		H -= hue;
		// Poor man's mod because fmod works weird
		if (H < 0.0f)
			H += 1.0f;
		if (H > 1.0f)
			H -= 1.0f;
		S *= saturation;
		hsvToRGB(R, G, B, H, S, V);
	}
}

// calc RGB values of from HSV values
void Visualization::hsvToRGB(float& R,float& G,float& B, float H, float S, float V)
{
	int hueCase = (int)(H*6);
	float frac = 6.0*H - hueCase;
	float lx = V * (1.0 - S);
	float ly = V * (1.0 - S * frac);
	float lz = V * (1.0 - S*(1.0 - frac));
	switch(hueCase)
	{
		case 0:
		case 6: R = V; G = lz; B = lx; break;
		case 1: R = ly; G = V; B = lx; break;
		case 2: R = lx; G = V; B = lz; break;
		case 3: R = lx; G = ly; B = V; break;
		case 4: R = lz; G = lx; B = V; break;
		case 5: R = V; G = lx; B = ly; break;
	}
}
 
// calc HSV values of rgbhsvColor from RGB values
void Visualization::rgbToHSV(float R,float G,float B, float& H, float& S, float& V)
{	
	float M = std::max(std::max(R, G), B);
	float m = std::min(std::min(R, G), B);

	float d = M - m;
	V = M;
	S = (M > 0.00001) ? d / M : 0.0;
	if(S == 0.0) H = 0.0;
	else{
		if(R == M)
			H = (G - B) / d;
		else if(G == M)
			H = 2.0 + (B - R) / d;
		else 
			H = 4.0 + (R - G) / d;
		H /= 6.0;
		if(H < 0.0) H += 1.0;
	}
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
		float R, G, B;
		set_colormap((float)(i * stepSize + ((float)(i + 1) / numColors) * stepSize) / th, R, G, B);
		glColor3f(R, G, B);
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
void Visualization::draw_smoke(fftw_real wn, fftw_real hn, int DIM, std::vector<fftw_real> color_map_values, std::vector<fftw_real> height_values, fftw_real min_color, fftw_real max_color, fftw_real min_height, fftw_real max_height)
{
	int i, j;
    fftw_real vy0, vy1, vy2, vy3;
    fftw_real height0, height1, height2, height3;

    if(useTextures){
		glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D,texture_id[color_map_idx]);	
	}
 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 	
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
                vy0 = clamp(color_map_values.at(idx0), min_clamp_value, max_clamp_value);
                vy1 = clamp(color_map_values.at(idx1), min_clamp_value, max_clamp_value);
                vy2 = clamp(color_map_values.at(idx2), min_clamp_value, max_clamp_value);
                vy3 = clamp(color_map_values.at(idx3), min_clamp_value, max_clamp_value);
            }
            else
            {  // Scale
                vy0 = scale(color_map_values.at(idx0), min_color, max_color);
                vy1 = scale(color_map_values.at(idx1), min_color, max_color);
                vy2 = scale(color_map_values.at(idx2), min_color, max_color);
                vy3 = scale(color_map_values.at(idx3), min_color, max_color);
            }

            if (heightClamping)
            {
            	// Clamp heights
            	height0 = clamp(height_values.at(idx0), min_height_clamp_value, max_height_clamp_value) * height_scale;
	           	height1 = clamp(height_values.at(idx1), min_height_clamp_value, max_height_clamp_value) * height_scale;
	           	height2 = clamp(height_values.at(idx2), min_height_clamp_value, max_height_clamp_value) * height_scale;
	           	height3 = clamp(height_values.at(idx3), min_height_clamp_value, max_height_clamp_value) * height_scale;
            } else {
            	// Scale heights
            	height0 = scale(height_values.at(idx0), min_height, max_height) * height_scale;
	           	height1 = scale(height_values.at(idx1), min_height, max_height) * height_scale;
	           	height2 = scale(height_values.at(idx2), min_height, max_height) * height_scale;
	           	height3 = scale(height_values.at(idx3), min_height, max_height) * height_scale;
            }

           	

            if (drawMatter)
            {
				glBegin(GL_TRIANGLES);
				if(useTextures) {
					glTexCoord1f(vy0);
					glVertex3f(px0, py0, height0);
					glTexCoord1f(vy1);
					glVertex3f(px1, py1, height1);
					glTexCoord1f(vy2);
					glVertex3f(px2, py2, height2);
					glTexCoord1f(vy0);
					glVertex3f(px0, py0, height0);
					glTexCoord1f(vy2);
					glVertex3f(px2, py2, height2);
					glTexCoord1f(vy3);
					glVertex3f(px3, py3, height3);
				} else {
					float R, G, B;
					set_colormap(vy0, R, G, B); glColor3f(R, G, B);
					glVertex3f(px0, py0, height0);
					set_colormap(vy1, R, G, B); glColor3f(R, G, B);
					glVertex3f(px1, py1, height1);
					set_colormap(vy2, R, G, B); glColor3f(R, G, B);
					glVertex3f(px2, py2, height2);
					set_colormap(vy0, R, G, B); glColor3f(R, G, B);
					glVertex3f(px0, py0, height0);
					set_colormap(vy2, R, G, B); glColor3f(R, G, B);
					glVertex3f(px2, py2, height2);
					set_colormap(vy3, R, G, B); glColor3f(R, G, B);
					glVertex3f(px3, py3, height3);
				}
				glEnd();
            }
			if (drawIsolines)
            {
        		if (!multipleIsolines)
        		{
        			num_isoline_value = 1;
        			// lower_isoline_value = isoline_value;
        			// upper_isoline_value = isoline_value;
        		}
        		for (int iso_idx = 0; iso_idx < num_isoline_value; ++iso_idx)
        		{
        			float isoline_value = lower_isoline_value + ((double) iso_idx / num_isoline_value) * (upper_isoline_value - lower_isoline_value);
        			if (clamping)
        				isoline_value = clamp(isoline_value, min_clamp_value, max_clamp_value);
        			else
        				isoline_value = scale(isoline_value, min_color, max_color);

        			float R, G, B;
	            	double lambda_1, lambda_2, lambda_3, lambda_4, mean;
	            	uint8_t code = 0;
	            	if (vy1 > isoline_value)
	            		SETBIT(code, 3);
	            	if (vy2 > isoline_value)
	            		SETBIT(code, 2);
	            	if (vy3 > isoline_value)
	            		SETBIT(code, 1);
	            	if (vy0 > isoline_value)
	            		SETBIT(code, 0);
	            	glBegin(GL_LINES);
	            	if (useTextures)
	            		glTexCoord1f(isoline_value);
	            	else
	            	{
		            	set_colormap(isoline_value, R, G, B);
		            	glColor3f(R, G, B);
		            }
	            	switch (code)
	            	{
	        		case 1:
	        		case 14:
	            		lambda_1 = interpolate(vy0, vy3, isoline_value);
	            		lambda_2 = interpolate(vy0, vy1, isoline_value);
	            		glVertex2f(px0 + lambda_1 * wn, py0);
	            		glVertex2f(px0, py0 + lambda_2 * hn);
	            		break;
	        		case 2:
	        		case 13:
	        			lambda_1 = interpolate(vy0, vy3, isoline_value);
	            		lambda_2 = interpolate(vy3, vy2, isoline_value);
	            		glVertex2f(px0 + lambda_1 * wn, py0);
	            		glVertex2f(px3, py3 + lambda_2 * hn);
	            		break;
	        		case 3:
	        		case 12:
	            		lambda_1 = interpolate(vy0, vy1, isoline_value);
	            		lambda_2 = interpolate(vy3, vy2, isoline_value);
	            		glVertex2f(px0, py0 + lambda_1 * hn);
	            		glVertex2f(px3, py3 + lambda_2 * hn);
	            		break;
	        		case 4:
	        		case 11:
	            		lambda_1 = interpolate(vy1, vy2, isoline_value);
	            		lambda_2 = interpolate(vy3, vy2, isoline_value);
	            		glVertex2f(px1 + lambda_1 * wn, py1);
	            		glVertex2f(px3, py3 + lambda_2 * hn);
	            		break;
	            	case 5:
	            		// KLOPT DIT?
	        			mean = (vy0 + vy1 + vy2 + vy3) / 4;
	        			lambda_1 = interpolate(vy0, vy1, isoline_value);
	        			lambda_2 = interpolate(vy1, vy2, isoline_value);
	        			lambda_3 = interpolate(vy3, vy2, isoline_value);
	            		lambda_4 = interpolate(vy0, vy3, isoline_value);
	        			if (mean > isoline_value)
	        			{
		            		glVertex2f(px0, py0 + lambda_1 * hn);
		            		glVertex2f(px1 + lambda_2 * wn, py1);
		            		glVertex2f(px3, py3 + lambda_3 * hn);
		            		glVertex2f(px0 + lambda_4 * wn, py0);
	        			} else
	        			{
	        				glVertex2f(px0, py0 + lambda_1 * hn);
		            		glVertex2f(px0 + lambda_4 * wn, py0);
		            		glVertex2f(px3, py3 + lambda_3 * hn);
		            		glVertex2f(px1 + lambda_2 * wn, py1);
	        			}
	            		break;
	        		case 6:
	        		case 9:
	            		lambda_1 = interpolate(vy1, vy2, isoline_value);
	            		lambda_2 = interpolate(vy0, vy3, isoline_value);
	            		glVertex2f(px1 + lambda_1 * wn, py1);
	            		glVertex2f(px0 + lambda_2 * wn, py0);
	            		break;
	        		case 7:
	        		case 8:
	            		lambda_1 = interpolate(vy0, vy1, isoline_value);
	            		lambda_2 = interpolate(vy1, vy2, isoline_value);
	            		glVertex2f(px0, py0 + lambda_1 * hn);
	            		glVertex2f(px1 + lambda_2 * wn, py1);
	            		break;
	            	case 10:
	            		// KLOPT DIT?
	            		mean = (vy0 + vy1 + vy2 + vy3) / 4;
	        			lambda_1 = interpolate(vy0, vy1, isoline_value);
	        			lambda_2 = interpolate(vy1, vy2, isoline_value);
	        			lambda_3 = interpolate(vy3, vy2, isoline_value);
	            		lambda_4 = interpolate(vy0, vy3, isoline_value);
	            		if (mean <= isoline_value)
	        			{
		            		glVertex2f(px0, py0 + lambda_1 * hn);
		            		glVertex2f(px1 + lambda_2 * wn, py1);
		            		glVertex2f(px3, py3 + lambda_3 * hn);
		            		glVertex2f(px0 + lambda_4 * wn, py0);
	        			} else
	        			{
	        				glVertex2f(px0, py0 + lambda_1 * hn);
		            		glVertex2f(px0 + lambda_4 * wn, py0);
		            		glVertex2f(px3, py3 + lambda_3 * hn);
		            		glVertex2f(px1 + lambda_2 * wn, py1);
	        			}
	            		break;
	        		default:
	        			break;
	            		// Case 0, 15
	            	}
	            	glEnd();
            	}
            }
        }
    }
    if (useTextures)
		glDisable(GL_TEXTURE_1D);	
}

double Visualization::interpolate(double v1, double v2, double iso)
{
	return (v1 - iso) / (v1 - v2);
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

void Visualization::divergence(fftw_real* f_x, fftw_real* f_y, std::vector<fftw_real>& diff, Model* model)
{
	fftw_real prev_x, prev_y, next_x, next_y, divergence;
	model->max_div = -FLT_MAX;
	model->min_div = FLT_MAX;
	for (int j = 0; j < model->DIM; ++j)
	{
		for (int i = 0; i < model->DIM; ++i)
		{			 
			// Calculate previous and next for derivative
			prev_x = f_x[((i - 1 + model->DIM) % model->DIM) + j * model->DIM];
			next_x = f_x[((i + 1) % model->DIM) + j * model->DIM];

			prev_y = f_y[i + ((j - 1 + model->DIM) % model->DIM) * model->DIM];
			next_y = f_y[i + ((j + 1) % model->DIM) * model->DIM];
			
			divergence = next_x - prev_x + next_y - prev_y;
			diff.push_back(divergence);
			model->max_div = std::max(divergence, model->max_div);
			model->min_div = std::min(divergence, model->min_div);
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
	float x_dif = x_end - x_start;
	float y_dif = y_end - y_start;
	float triangle_height = sqrt(x_dif * x_dif + y_dif * y_dif);
	float angle = calc_angle(x_dif, y_dif);
	
	glPushMatrix();
	glTranslatef(x_start, y_start, 0.0f);
	glRotatef(angle, 0.0, 0.0, 1.0);

	glBegin(GL_TRIANGLES);
	{
		glVertex2f(-triangle_height / 4, 0);
		glVertex2f(triangle_height / 4, 0);
		glVertex2f(0, triangle_height);
	}
	glEnd();

	glPopMatrix();
}

void Visualization::addSeedPoint(double x, double y, double z)
{
	streamTubes.push_back({{x, y, 0}});
}

void Visualization::removeSeedPoint()
{
	if(!streamTubes.empty())
		streamTubes.pop_back();
}

void Visualization::draw_streamtubes(fftw_real wn, fftw_real hn)
{
	for (auto it=streamTubes.begin(); it != streamTubes.end(); ++it)
	{
		Point3d seed = (*it).front();
		glPushMatrix();
		glTranslatef(seed.x * wn + wn, seed.y * hn + hn, seed.z * 8);
		glutSolidSphere(4, 50, 50);
		glPopMatrix();
	}
}

void Visualization::set_last_z_value(double zval)
{
	std::list<Point3d>& elem = streamTubes.back();
	elem.front().z = zval;
}