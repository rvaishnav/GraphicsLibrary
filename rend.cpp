/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

short ctoi(float color);
void normalize(GzCoord vect);
void crossproduct(GzCoord a,GzCoord b,GzCoord c);
float dotproduct(GzCoord a,GzCoord b);
/* convert float color to GzIntensity short */
int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	int i,j;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
		{
			mat[i][j] = 0;
		}
	mat[0][0]=1;
	mat[1][1]=cos(PI*degree/180);
	mat[1][2]=-sin(PI*degree/180);
	mat[2][1]=sin(PI*degree/180);
	mat[2][2]=cos(PI*degree/180);
	mat[3][3]=1;
	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	int i,j;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
		{
			mat[i][j] = 0;
		}
			mat[0][0] = cos(PI*degree/180);
	mat[0][2]=sin(PI*degree/180);
	mat[1][1]=1;
	mat[2][0]=-sin(PI*degree/180);
	mat[2][2]=cos(PI*degree/180);
	mat[3][3]=1;
	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
		int i,j;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
		{
			mat[i][j] = 0;
		}
	mat[0][0]=cos(PI*degree/180);
	mat[0][1]=-sin(PI*degree/180);	
	mat[1][0]=sin(PI*degree/180);
	mat[1][1]=cos(PI*degree/180);
	mat[2][2]=1;
	mat[3][3]=1;
	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			if(i==j)
				mat[i][j] = 1;
			else
				mat[i][j] = 0;
		}
	mat[0][3] = translate[0];
	mat[1][3] = translate[1];
	mat[2][3] = translate[2];
	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			mat[i][j] = 0;
		}
	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
	mat[3][3] = 1;
	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 
	*render= new GzRender();
	(*render)->display=display;
	(*render)->renderClass=renderClass;
	(*render)->open=0;
	(*render)->matlevel=-1;
	(*render)->camera.FOV = DEFAULT_FOV;
	(*render)->camera.position[0] = DEFAULT_IM_X;
	(*render)->camera.position[1] = DEFAULT_IM_Y;
	(*render)->camera.position[2] = DEFAULT_IM_Z;
	(*render)->camera.lookat[0] = 0;
	(*render)->camera.lookat[1] = 0;
	(*render)->camera.lookat[2] = 0;
	(*render)->camera.worldup[0] = 0;
	(*render)->camera.worldup[1] = 1;
	(*render)->camera.worldup[2] = 0;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			(*render)->Xsp[i][j] = 0;
		}
	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	free(render);
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 
	int i,j;
	//FROM PERSPECTIVE TO SCREEN
	render->Xsp[0][0]=(render->display->xres)/2;
	render->Xsp[0][3]=(render->display->xres)/2;
	render->Xsp[1][1]=-(render->display->yres)/2;
	render->Xsp[1][3]=(render->display->yres)/2;
	render->Xsp[2][2]=tan((render->camera.FOV*PI)/360)*INT_MAX;
	render->Xsp[3][3]=1;
	GzPushMatrix(render, render->Xsp);

	
	//FROM IMAGE OR CAMERA TO PERSPECTIVE
	for(i=0;i<4;i++)
		for(j=0;j<4;j++)
		{
			if(i==j)
				render->camera.Xpi[i][j]=1;
			else
				render->camera.Xpi[i][j]=0;
		}
		render->camera.Xpi[3][2]=tan((render->camera.FOV*PI)/360);
	GzPushMatrix(render, render->camera.Xpi);

	
	//FROM WORLD TO IMAGE
	GzCoord camx,camy,camz;
	float updotz;
	for(i=0;i<3;i++)
	camz[i]=render->camera.lookat[i]-render->camera.position[i];
	normalize(camz);
	normalize(render->camera.worldup);
	updotz=dotproduct(render->camera.worldup,camz);
	
	for(i=0; i<3; i++)
		camy[i] = render->camera.worldup[i] - updotz*camz[i];
	normalize(camy);
	crossproduct(camy,camz,camx);
	float cdotx,cdoty,cdotz;
	cdotx=dotproduct(camx,render->camera.position);
	cdoty=dotproduct(camy,render->camera.position);
	cdotz=dotproduct(camz,render->camera.position);

	for(int i=0; i<3; i++)
	{
		render->camera.Xiw[0][i] = camx[i];
		render->camera.Xiw[1][i] = camy[i];
		render->camera.Xiw[2][i] = camz[i];
		render->camera.Xiw[3][i] = 0;
	}
		render->camera.Xiw[0][3] = -1*cdotx;
		render->camera.Xiw[1][3] = -1*cdoty;
		render->camera.Xiw[2][3] = -1*cdotz;
		render->camera.Xiw[3][3] = 1;
		GzPushMatrix(render, render->camera.Xiw);
		render->open = 1;
		render->numlights=0;
	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	int i;

	for(i=0;i<3;i++)
	{
	render->camera.position[i]=camera->position[i];
	render->camera.lookat[i]=camera->lookat[i];
	render->camera.worldup[i]=camera->worldup[i];
	}
	render->camera.FOV=camera->FOV;
	for(i=0;i<4;i++)
		for(int j=0;j<4;j++)
		{
			render->camera.Xiw[i][j]=camera->Xiw[i][j];
			render->camera.Xpi[i][j]=camera->Xpi[i][j];
		}
	return GZ_SUCCESS;	
}
void normalize(GzCoord vect)
{
	float length=sqrt(vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2]);
	for(int i=0;i<3;i++)
		vect[i]=vect[i]/length;
}
void crossproduct(GzCoord a,GzCoord b,GzCoord c)
{
	c[0]=a[1]*b[2]-a[2]*b[1];
	c[1]=a[2]*b[0]-a[0]*b[2];
	c[2]=a[0]*b[1]-a[1]*b[0];
	return;
}

float dotproduct(GzCoord a,GzCoord b)
{
	float dot=a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
	return dot;
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/ int i,j;
render->matlevel=render->matlevel+1;
if(render->matlevel<MATLEVELS)
	{
		if(render->matlevel==0)
		{
		for(i=0;i<4;i++)
			for(j=0;j<4;j++)
				{
					render->Ximage[render->matlevel][i][j]=matrix[i][j];
					if(i==j)
					{
						render->Xnorm[render->matlevel][i][j] = 1;
					}
					else
					{
						render->Xnorm[render->matlevel][i][j] = 0;
					}
			    }

		}
		else
		{
			for(i=0;i<4;i++)
				for(j=0;j<4;j++)
				{
					render->Ximage[render->matlevel][i][j]=0;
					for(int k=0;k<4;k++)
					render->Ximage[render->matlevel][i][j]+=render->Ximage[render->matlevel-1][i][k]*matrix[k][j];		
				}
			
			double scale = 1.0/ sqrt(matrix[0][0]*matrix[0][0] + matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2]);		

			GzMatrix unitary;
			for(int i=0; i<3; i++)
				for(int j=0; j<3; j++)
				{
					unitary[i][j] = matrix[i][j]*scale;
				}

			unitary[0][3] = 0;
			unitary[1][3] = 0;
			unitary[2][3] = 0;
			unitary[3][3] = 1;
			unitary[3][0] = 0;
			unitary[3][1] = 0;
			unitary[3][2] = 0;

			for(i=0;i<4;i++)
				for(j=0;j<4;j++)
				{
					render->Xnorm[render->matlevel][i][j]=0;
					for(int k=0;k<4;k++)
					render->Xnorm[render->matlevel][i][j]+=render->Xnorm[render->matlevel-1][i][k]*unitary[k][j];		
				}
		}
	}
	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if(render->matlevel>=0)
	{
		render->matlevel=render->matlevel-1;
	}
	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
		
			for(int i=0;i<numAttributes;i++)
			{
				if(nameList[i]== GZ_RGB_COLOR)
				{	
					GzColor* col=(GzColor*)valueList[i];
					for(int j=0;j<3;j++)
					render->flatcolor[j]=(*col)[j];
				}

				else if(nameList[i]== GZ_DIRECTIONAL_LIGHT && render->numlights<MAX_LIGHTS)
				{
					render->lights[render->numlights].direction[0]= ((float*)valueList[i])[0];
					render->lights[render->numlights].direction[1]= ((float*)valueList[i])[1];
					render->lights[render->numlights].direction[2]= ((float*)valueList[i])[2];

					render->lights[render->numlights].color[0]= ((float*)valueList[i])[3];
					render->lights[render->numlights].color[1]= ((float*)valueList[i])[4];
					render->lights[render->numlights].color[2]= ((float*)valueList[i])[5];

					render->numlights++;					
				}
				else if(nameList[i]== GZ_AMBIENT_LIGHT)
				{
					render->ambientlight.direction[0]= ((float*)valueList[i])[0];
					render->ambientlight.direction[1]= ((float*)valueList[i])[1];
					render->ambientlight.direction[2]= ((float*)valueList[i])[2];

					render->ambientlight.color[0]= ((float*)valueList[i])[3];
					render->ambientlight.color[1]= ((float*)valueList[i])[4];
					render->ambientlight.color[2]= ((float*)valueList[i])[5];
				}
				else if(nameList[i]== GZ_SPECULAR_COEFFICIENT)
				{
					for(int j=0; j<3; j++)
					{
						render->Ks[j]= ((float*)valueList[i])[j];
					}
				}
				else if(nameList[i]== GZ_DIFFUSE_COEFFICIENT)
				{
					for(int j=0;j<3;j++)
					{
						render->Kd[j]= ((float*)valueList[i])[j];
					}
				}
				else if(nameList[i]== GZ_AMBIENT_COEFFICIENT)
				{
					for(int j=0;j<3;j++)
					{
						render->Ka[j]= ((float*)valueList[i])[j];
					}
				}
				else if(nameList[i]== GZ_DISTRIBUTION_COEFFICIENT)
				{
					render->spec = ((float*)valueList[i])[0];
				}
				else if(nameList[i] == GZ_INTERPOLATE)
				{
					render->interp_mode = ((int*)valueList[i])[0];
				}

			}
	return GZ_SUCCESS;
}
void computecolor(GzRender* render,GzCoord E,GzCoord normal,GzCoord tempcolor)
{  
	GzCoord diffuse,refL,specular;
	GzCoord refinednormal;
	for(int i=0;i<3;i++)
	{
		refinednormal[i]=normal[i];
		diffuse[i]=0;
		specular[i]=0;
	}
	float normaldotE,refLdotE;
	float normaldotlight[MAX_LIGHTS],ndotl[MAX_LIGHTS];
	normaldotE=dotproduct(normal,E);
	for(int i=0;i<render->numlights;i++)
	{	
		ndotl[i]=normaldotlight[i]=(render->lights[i].direction[0])*normal[0]+(render->lights[i].direction[1])*normal[1]+(render->lights[i].direction[2])*normal[2];
		if(ndotl[i]<0 && normaldotE<0)
		{
			for(int k=0;k<3;k++)
			refinednormal[k]=-normal[k];
			normaldotlight[i]=(render->lights[i].direction[0])*refinednormal[0]+(render->lights[i].direction[1])*refinednormal[1]+(render->lights[i].direction[2])*refinednormal[2];
		}
		if((ndotl[i]>0 && normaldotE>0)||(ndotl[i]<0 && normaldotE<0))
		{
			for(int k=0;k<3;k++)
			{
			diffuse[k]+=render->lights[i].color[k]*normaldotlight[i];
			refL[k] = 2*normaldotlight[i]*refinednormal[k] - render->lights[i].direction[k];
			}
			refLdotE=dotproduct(refL,E);
			if(refLdotE<0)
				refLdotE=0.0;
			else if(refLdotE>1)
				refLdotE=1.0;
			float REpowS=pow(refLdotE,render->spec);
			for(int k=0;k<3;k++)
				specular[k]+=render->lights[i].color[k]*REpowS;
		}
	}

	for(int i=0; i<3; i++)
	{
		tempcolor[i]=(specular[i] * render->Ks[i]) + (diffuse[i] * render->Kd[i]) + (render->ambientlight.color[i] * render->Ka[i]);
		if (tempcolor[i] > 1.0)
			tempcolor[i] = 1.0;
		else if (tempcolor[i] < 0.0)
			tempcolor[i] = 0.0;
	}		

}
int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, 
				  GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts  
- Clip - just discard any triangle with verts behind view plane 
       - test for triangles with all three verts off-screen 
- invoke triangle rasterizer  
*/ 

	if(nameList[0]==GZ_POSITION)
	{
			int status=0;
			GzIntensity red,green,blue,alpha;
			GzDepth z=INT_MAX;
			GzCoord v[3],n[3],edge,edge10,edge21,point,nrml,nrml21,nrmlpoint;
			int inside;
			float intz;
			for(int i=0;i<3;i++)
				for(int j=0;j<3;j++)
					v[i][j]=((GzCoord*)valueList[0])[i][j];

			float Xformvert[4];
			for(int i=0; i<3; i++)
			{
				float vert4d[4] = {v[i][0],v[i][1],v[i][2],1};
				for(int j=0; j<4; j++)
				{
					Xformvert[j]=0;
					for(int k=0;k<4;k++)
					Xformvert[j]+=(render->Ximage[render->matlevel][j][k]*vert4d[k]);
				}
				v[i][0] = Xformvert[0]/Xformvert[3];
				v[i][1] = Xformvert[1]/Xformvert[3];
				v[i][2] = Xformvert[2]/Xformvert[3];
			}

			for(int i=0;i<3;i++)
				for(int j=0;j<3;j++)
					n[i][j]=((GzCoord*)valueList[1])[i][j];

			float Xformnorm[4];
			for(int i=0; i<3; i++)
			{
				float norm4d[4] = {n[i][0],n[i][1],n[i][2],1};
				for(int j=0; j<4; j++)
				{
					Xformnorm[j]=0;
					for(int k=0;k<4;k++)
						Xformnorm[j]+=(render->Xnorm[render->matlevel][j][k]*norm4d[k]);
				}
				n[i][0] = Xformnorm[0]/Xformnorm[3];
				n[i][1] = Xformnorm[1]/Xformnorm[3];
				n[i][2] = Xformnorm[2]/Xformnorm[3];
			}



			float xmax, xmin, ymax, ymin;
			ymin=xmin=INT_MAX;
			ymax=xmax=0;
						for(int i=0;i<3;i++)
					{	if(v[i][0]<xmin)
							xmin=v[i][0];
						if(v[i][0]>xmax)
							xmax=v[i][0];
						if(v[i][1]<ymin)
							ymin=v[i][1];
						if(v[i][1]>ymax)
							ymax=v[i][1];
					}
		
			
			edge10[0] = v[1][0] - v[0][0];
			edge10[1] = v[1][1] - v[0][1];
			edge10[2] = v[1][2] - v[0][2];

			edge21[0] = v[2][0] - v[1][0];
			edge21[1] = v[2][1] - v[1][1];
			edge21[2] = v[2][2] - v[1][2];
			crossproduct(edge21, edge10, nrml21);

			edge10[2] = 0;
			edge21[2] = 0;
			crossproduct(edge21, edge10, nrml);

			float A, B, C, D;
		
			A = nrml21[0];
			B = nrml21[1];
			C = nrml21[2];
			D = -(A*v[1][0] + B*v[1][1] + C*v[1][2]);

			GzCoord E,colorsA,colorsB,colorsC,colorsD,color[3];
			GzCoord normA,normB,normC,normD;
			
			E[0] = 0;
			E[1] = 0;
			E[2] =-1;

			for(int i=0;i<3;i++)
			{
			  GzCoord tempcolor;
			  GzCoord tempnormal;

			  for(int j=0;j<3;j++)
			  	tempnormal[j]=n[i][j];
				
				computecolor(render,E,tempnormal,tempcolor);
			  for(int j=0;j<3;j++)	
				color[i][j]=tempcolor[j];
			  
			}

			if(render->interp_mode == GZ_COLOR)
			{
				GzCoord colornormal[3];
				for(int i=0; i<3; i++)
				{
					edge10[2] = color[1][i] - color[0][i];
					edge21[2] = color[2][i] - color[1][i];				
					crossproduct(edge21, edge10, colornormal[i]);
					colorsA[i] = colornormal[i][0];
					colorsB[i] = colornormal[i][1];
					colorsC[i] = colornormal[i][2];
					colorsD[i] = -(colorsA[i]*v[0][0] + colorsB[i]*v[0][1] + colorsC[i]*color[0][i]);
				}
			}
			
			else if(render->interp_mode == GZ_NORMALS)
			{				
				GzCoord interN[3];
				for(int i=0; i<3; i++)
				{
					edge10[2] = n[1][i]-n[0][i];
					edge21[2] = n[2][i]-n[1][i];				
					crossproduct(edge21,edge10,interN[i]);
					normA[i] = interN[i][0];
					normB[i] = interN[i][1];
					normC[i] = interN[i][2];
					normD[i] = -(normA[i]*v[0][0] + normB[i]*v[0][1] + normC[i]*n[0][i]);
				}
			}
			for(int x=floor(xmin);x<=ceil(xmax) && x<(render->display->xres);x++)
			{
				if(x<0)
					x=0;
				for(int y=floor(ymin);y<=ceil(ymax) && y<(render->display->yres);y++)
				{
					if(y<0)
						y=0;							
					inside=0;
					
					for(int i=0; i<3; i++)
					{	
						edge[0] = v[(i+1)%3][0] - v[i][0];
						edge[1] = v[(i+1)%3][1] - v[i][1];
						edge[2] = 0;

						point[0] = v[(i+1)%3][0] - x;
						point[1] = v[(i+1)%3][1] - y;
						point[2] = 0;

						crossproduct(edge, point, nrmlpoint);
						
						float cond = dotproduct(nrml,nrmlpoint);

						if(cond > 0)		
							inside=inside+1;
						else
							break;
					}
					if(inside==3)
					{
						intz = -(A*x+B*y+D)/C;
						status=GzGetDisplay(render->display,x,y,&red,&green,&blue,&alpha,&z);
						if(intz<z)
						{	GzCoord tempcolor;
							if(render->interp_mode==GZ_NORMALS)
							{	GzCoord intnormal;
								for(int i=0;i<3;i++)
									intnormal[i]=-(normA[i]*x + normB[i]*y + normD[i])/normC[i];
								normalize(intnormal);
								computecolor(render,E,intnormal,tempcolor);
							}
							
							else if(render->interp_mode==GZ_COLOR)
							{
								for(int i=0;i<3;i++)
								tempcolor[i]=-(colorsA[i]*x+colorsB[i]*y+colorsD[i])/colorsC[i];
							}
							status=GzPutDisplay(render->display,x,y,ctoi(tempcolor[0]),ctoi(tempcolor[1]),ctoi(tempcolor[2]),1,intz);
						}
					}
				}
			}				
	}
		
	return GZ_SUCCESS;
}



/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

