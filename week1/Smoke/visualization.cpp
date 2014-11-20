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
	if (value<0)
	{
		value=0;
	}
	if (value>1)
	{
		value=1;
	}
	value = (6-2*dx)*value+dx;
	*R = fmax(0.0, (3-fabs(value-4)-fabs(value-5))/2);
	*G = fmax(0.0, (4-fabs(value-2)-fabs(value-4))/2);
	*B = fmax(0.0, (3-fabs(value-1)-fabs(value-2))/2);
}

//diverge: Implements a color pallete that diverges
void Visualization::bipolar(float value,float* R,float* G,float* B)
{
	if (value<0)
	{
		value=0;
	}
	if (value>1)
	{
		value=1;
	}
	*R = value * fmax(0.0, (3-fabs(value-1)-fabs(value-2))/2);
	*G = 0;
	*B = 1 - (value * fmax(0.0, (3-fabs(value-1)-fabs(value-2))/2));
}




//set_colormap: Sets three different types of colormaps
void Visualization::set_colormap(float vy)
{
	float R,G,B,H,S,V;
	if (scalar_col==COLOR_BLACKWHITE)
		R = G = B = vy;
	else if (scalar_col==COLOR_RAINBOW)
		rainbow(vy,&R,&G,&B);
	else if (scalar_col==COLOR_BANDS)
	{
		const int NLEVELS = 7;
		vy *= NLEVELS; vy = (int)(vy); vy/= NLEVELS;
		rainbow(vy,&R,&G,&B);
	}
	else if (scalar_col == COLOR_BIPOLAR)
	{
		bipolar(vy,&R,&G,&B);
	}
	glColor3f(R,G,B);
}


//-(void)hsvToRGB:(struct rgbhsvColor*)color
void Visualization::hsvToRGB(float* R,float* G,float* B, float* H, float* S, float* V)
{
	if((*S)==0.0)
	{
		*R=round((*V)*255.0);
		*G=round((*V)*255.0);
		*B=round((*V)*255.0);
	}
	else 
	{
		float hTemp=0.0;
 
		if((*H)==360.0)
			hTemp=0.0;
		else
			hTemp=(*H)/60.0;
 
		int i=trunc(hTemp);
		float f=hTemp-i;
 
		float p=(*V)*(1.0-(*S));
		float q=(*V)*(1.0-((*S)*f));
		float t=(*V)*(1.0-((*S)*(1.0-f)));
 
		switch (i) 
		{
			default:
			case 0:
			case 6:
				*R=round((*V)*255.0);
				*G=round(t*255.0);
				*B=round(p*255.0);
				break;
			case 1:
				*R=round(q*255.0);
				*G=round((*V)*255.0);
				*B=round(p*255.0);
				break;
			case 2:
				*R=round(p*255.0);
				*G=round((*V)*255.0);
				*B=round(t*255.0);
				break;
			case 3:
				*R=round(p*255.0);
				*G=round(q*255.0);
				*B=round((*V)*255.0);
				break;
			case 4:
				*R=round(t*255.0);
				*G=round(p*255.0);
				*B=round((*V)*255.0);
				break;
			case 5:
				*R=round((*V)*255.0);
				*G=round(p*255.0);
				*B=round(q*255.0);
				break;
		}
 
	}
}
 
// calc HSV values of rgbhsvColor from RGB values
void Visualization::rgbToHSV(float* R,float* G,float* B, float* H, float* S, float* V)
{
	float maxRGBValue=MAX(MAX(*R, *G), *B);
	float minValue=MIN(MIN(*R, *G), *B);
	float maxValue=maxRGBValue;
	float hue,saturation,vvalue,delta;
 
	minValue=minValue/255.0;
	maxValue=maxValue/255.0;
 
	vvalue=maxValue;
	delta=vvalue-minValue;
 
	if(delta==0)
		saturation=0;
	else 
		saturation=round(delta/vvalue);
 
	if(saturation==0)
		hue=0;
	else 
	{
		if((*R)==maxRGBValue)
			hue=60.0*(float)((*R) - (*B))/255.0/delta;
		else 
		{
			if((*G)==maxRGBValue)
				hue=120.0+60.0*(float)((*B)-(*R))/255.0/delta;
			else 
			{
				if((*B)==maxRGBValue)
					hue=240.0+60.0*(float)((*R)-(*G))/255.0/delta;
			}
		}
		if(hue<0.0)
			hue+=360.0;
	}
	*H = round(hue);
	*S = round(saturation);
	*V = round(vvalue);
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
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < th; i += 10)
	{
		set_colormap(((float) i )/ th);
		glVertex2f(tw * 0.9, i);
		glVertex2f(tw, i);
		glVertex2f(tw * 0.9, i + 5);
		glVertex2f(tw, i + 5);
	}
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

            set_colormap(model->rho[idx0]);    glVertex2f(px0, py0);
            set_colormap(model->rho[idx1]);    glVertex2f(px1, py1);
            set_colormap(model->rho[idx2]);    glVertex2f(px2, py2);

            set_colormap(model->rho[idx0]);    glVertex2f(px0, py0);
            set_colormap(model->rho[idx2]);    glVertex2f(px2, py2);
            set_colormap(model->rho[idx3]);    glVertex2f(px3, py3);
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
