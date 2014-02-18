/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/
	int bsize=sizeof(GzPixel)*width*height;
	*framebuffer = (char*)malloc(bsize);

	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{

/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/ *display=new GzDisplay[sizeof(GzDisplay)];
	(*display)->fbuf=new GzPixel[sizeof(GzPixel)*xRes*yRes];

	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	free(display);
	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */
	
	
	
	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{  int j;
	//display=new GzDisplay;
/* set everything to some default values - start a new frame */
	display->xres=256;
	display->yres=256;
	j=(display->xres)*(display->yres);
	display->open=1;
	display->dispClass=0;
	for(int i=0;i<j;i++)
	{
		display->fbuf[i].alpha=1;
		display->fbuf[i].z=INT_MAX;
		display->fbuf[i].red=3000;
		display->fbuf[i].green=3000;
		display->fbuf[i].blue=3000;
	}
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if(i<0) i=0;
	else if(i>MAXXRES) i=MAXXRES;
	if(j<0) j=0;
	else if(j>MAXYRES) j=MAXYRES;
		if(r<0) r=0;
	else if(r>4095) r=4095;
	//display->fbuf->red=r;
		if(b<0) b=0;
	else if(b>4095) b=4095;
	//display->fbuf->blue=b;
		if(g<0) g=0;
	else if(g>4095) g=4095;
	int findex=0;
	
	//xres=512;
	//yres=512;
	findex=ARRAY(i,j);

	display->fbuf[findex].red=r;
	display->fbuf[findex].green=g;
	display->fbuf[findex].blue=b;
	display->fbuf[findex].alpha=a;
	display->fbuf[findex].z=z;

	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */
	int findex,zed;
	findex=ARRAY(i,j);
	*r=display->fbuf[findex].red;
	*g=display->fbuf[findex].green;
	*b=display->fbuf[findex].blue;
	*a=display->fbuf[findex].alpha;
	*z=display->fbuf[findex].z;
	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	int i,j,index;
	char color[3];
	fprintf(outfile,"P6 %d %d 255\n",512,512);
	for(j=0;j<display->yres;j++)
	{
		for(i=0;i<display->xres;i++)
		{
			index=i+(j*512);
			color[0]=(char)(display->fbuf[index].red>>4)& 0xff;
			color[1]=(char)(display->fbuf[index].green>>4)& 0xff;
			color[2]=(char)(display->fbuf[index].blue>>4)& 0xff;
			fwrite(color,sizeof(color),1,outfile);
		}
	}

	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	int pixelindex,findex,xres,yres;
	
	
	for(int j=0;j<256;j++)
	{
		for(int i=0;i<=256;i++)
		{
			pixelindex=i+(j*256);
			findex=pixelindex*3;
			framebuffer[findex+0]=display->fbuf[pixelindex].blue>>4;
	        framebuffer[findex+1]=display->fbuf[pixelindex].green>>4;
			framebuffer[findex+2]=display->fbuf[pixelindex].red>>4;
		}
	}

	return GZ_SUCCESS;
}