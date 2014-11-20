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

    if (drawMatter)
    {
        draw_smoke(wn, hn, model);
        draw_color_legend();
    }
    if (drawHedgehogs)
    {
        draw_velocities(wn, hn, model);
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

float Visualization::clamp(float x)
{
    if (x >= 1.0) {
        return 1.0;
    } else if (x < 0.0) {
        return 0.0;
    } else {
        return x;
    }
}

float Visualization::scale(float x, fftw_real min, fftw_real max)
{
    return (x - min) / (max - min);;
}

//set_colormap: Sets three different types of colormaps
void Visualization::set_colormap(float vy)
{
	if (limitColors == 1)
	{
		// Set color band
		vy *= numColors - 1;
		vy = (int)(vy);
		vy /= numColors - 1;
	}
	float R,G,B,H,S,V;
	if (scalar_col==COLOR_BLACKWHITE)
		R = G = B = vy;
	else if (scalar_col==COLOR_RAINBOW)
		rainbow(vy,&R,&G,&B);
	else if (scalar_col == COLOR_BIPOLAR)
	{
		bipolar(vy,&R,&G,&B);
	}
	if (hue != 1.0 || saturation != 1.0)
	{   // Save calculations when Hue AND Saturation are set to 1
		rgbToHSV(&R, &G, &B, &H, &S, &V);
		H *= hue;
		S *= saturation;
		hsvToRGB(&R, &G, &B, &H, &S, &V);
	}
	glColor3f(R,G,B);
}

//-(void)hsvToRGB:(struct rgbhsvColor*)color
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

void Visualization::display_text(float x, float y, char* const string)
{
	char * ch;
    glRasterPos3f( x, y, 0.0 );

    for( ch = string; *ch; ch++ ) {
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, (int)*ch );
    }
}

void Visualization::draw_color_legend()
{
	int tx, ty, tw, th, stepSize;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glBegin(GL_QUAD_STRIP);
	if (limitColors == 1)
	{
		stepSize = th / numColors;
	}
	else
	{
		stepSize = 5;
	}
	for (int i = 0; i < th; i += stepSize)
	{
		set_colormap(((float) i )/ th);
		glVertex2f(tw * 0.9, i);
		glVertex2f(tw, i);
		glVertex2f(tw * 0.9, i + stepSize / 2);
		glVertex2f(tw, i + stepSize / 2);
	}
	// Draw the remaining part, because integer division
	set_colormap(1);
	glVertex2f(tw * 0.9, th);
	glVertex2f(tw, th);
	glVertex2f(tw * 0.9, th - th % stepSize);
	glVertex2f(tw, th - th % stepSize);
	glEnd();

	float min = 0.0f;
	float max = 1.0f;

	char* minStr = NULL;
	char* maxStr = NULL;
	asprintf(&minStr, "%g", min);
	asprintf(&maxStr, "%g", max);

	glColor3f(1.0f, 1.0f, 1.0f);
	display_text(tw * 0.9 - 24, 0, minStr);
	display_text(tw * 0.9 - 24, th - 24, maxStr);
}

void Visualization::draw_smoke(fftw_real wn, fftw_real hn, Model* model)
{
	int i, j;
    fftw_real vy0, vy1, vy2, vy3;
 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (j = 0; j < model->DIM - 1; j++)            //draw smoke
    {
        for (i = 0; i < model->DIM - 1; i++)
        {
            double px0 = wn + (fftw_real)i * wn;
            double py0 = hn + (fftw_real)j * hn;
            int idx0 = (j * model->DIM) + i;

            double px1 = wn + (fftw_real)i * wn;
            double py1 = hn + (fftw_real)(j + 1) * hn;
            int idx1 = ((j + 1) * model->DIM) + i;

            double px2 = wn + (fftw_real)(i + 1) * wn;
            double py2 = hn + (fftw_real)(j + 1) * hn;
            int idx2 = ((j + 1) * model->DIM) + (i + 1);

            double px3 = wn + (fftw_real)(i + 1) * wn;
            double py3 = hn + (fftw_real)j * hn;
            int idx3 = (j * model->DIM) + (i + 1);

            if (clamping == 1)
            {  // Clamp
                vy0 = clamp(model->rho[idx0]);
                vy1 = clamp(model->rho[idx1]);
                vy2 = clamp(model->rho[idx2]);
                vy3 = clamp(model->rho[idx3]);
            }
            else
            {  // Scale
                vy0 = scale(model->rho[idx0], model->min_rho, model->max_rho);
                vy1 = scale(model->rho[idx1], model->min_rho, model->max_rho);
                vy2 = scale(model->rho[idx2], model->min_rho, model->max_rho);
                vy3 = scale(model->rho[idx3], model->min_rho, model->max_rho);
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
}

void Visualization::draw_velocities(fftw_real wn, fftw_real hn, Model* model)
{	
	int i, j, idx;
	glBegin(GL_LINES);				//draw velocities
	for (i = 0; i < model->DIM; i++)
	    for (j = 0; j < model->DIM; j++)
	    {
		  idx = (j * model->DIM) + i;
		  direction_to_color(model->vx[idx],model->vy[idx],color_dir);
		  glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
		  glVertex2f((wn + (fftw_real)i * wn) + vec_length * model->vx[idx], (hn + (fftw_real)j * hn) + vec_length * model->vy[idx]);
	    }
	glEnd();
}
