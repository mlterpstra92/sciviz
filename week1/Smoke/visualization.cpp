#include "visualization.h"
#include "model.h"
#include "GL/glui.h"

//rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
void Visualization::rainbow(float value,float* R,float* G,float* B)
{
	const float dx = 0.8;
	if (value<0)
	value=0; if (value>1) value=1;
	value = (6-2*dx)*value+dx;
	*R = fmax(0.0,(3-fabs(value-4)-fabs(value-5))/2);
	*G = fmax(0.0,(4-fabs(value-2)-fabs(value-4))/2);
	*B = fmax(0.0,(3-fabs(value-1)-fabs(value-2))/2);
}

//set_colormap: Sets three different types of colormaps
void Visualization::set_colormap(float vy)
{
	float R,G,B;

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

	glColor3f(R,G,B);
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

    glColor3f( 0.0f, 0.0f, 0.0f );
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

	display_text(200, 200, minStr);
	display_text(200, 300, maxStr);
}

void Visualization::draw_smoke(fftw_real wn, fftw_real hn, Model* model)
{
	int i, j, idx;
	double px, py;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (j = 0; j < model->DIM - 1; j++)           //draw smoke
    {
        glBegin(GL_TRIANGLE_STRIP);

        i = 0;
        px = wn + (fftw_real)i * wn;
        py = hn + (fftw_real)j * hn;
        idx = (j * model->DIM) + i;
        glColor3f(model->rho[idx],model->rho[idx],model->rho[idx]);
        glVertex2f(px,py);

        for (i = 0; i < model->DIM - 1; i++)
        {
            px = wn + (fftw_real)i * wn;
            py = hn + (fftw_real)(j + 1) * hn;
            idx = ((j + 1) * model->DIM) + i;
            set_colormap(model->rho[idx]);
            glVertex2f(px, py);
            px = wn + (fftw_real)(i + 1) * wn;
            py = hn + (fftw_real)j * hn;
            idx = (j * model->DIM) + (i + 1);
            set_colormap(model->rho[idx]);
            glVertex2f(px, py);
        }

        px = wn + (fftw_real)(model->DIM - 1) * wn;
        py = hn + (fftw_real)(j + 1) * hn;
        idx = ((j + 1) * model->DIM) + (model->DIM - 1);
        set_colormap(model->rho[idx]);
        glVertex2f(px, py);
        glEnd();
    }
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
