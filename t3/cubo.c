#include <stdio.h>
#include <stdlib.h>

#include "my_ogl.h"

float vertice[][3] = {{-1,-1, 1}, {-1, 1, 1}, { 1, 1, 1}, { 1,-1, 1},
                      {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}, { 1,-1,-1}};

float normal[][3] =  { {0,0,1}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {-1,0,0} };

float material[][4] = { { 0.33f, 0.22f, 0.03f, 1}, 
						{ 0.78f, 0.57f, 0.11f, 1}, 
						{ 0.99f, 0.91f, 0.81f, 1},
						{ 27.8f, 0, 0, 0 }			};

static void drawQuad(float size, int n, int a, int b, int c, int d)
{
	float v0[3],v1[3],v2[3],v3[3];
	
	v0[0]=size*vertice[a][0]/2; v0[1]=size*vertice[a][1]/2; v0[2]=size*vertice[a][2]/2;
	v1[0]=size*vertice[b][0]/2; v1[1]=size*vertice[b][1]/2; v1[2]=size*vertice[b][2]/2;
	v2[0]=size*vertice[c][0]/2; v2[1]=size*vertice[c][1]/2; v2[2]=size*vertice[c][2]/2;
	v3[0]=size*vertice[d][0]/2; v3[1]=size*vertice[d][1]/2; v3[2]=size*vertice[d][2]/2;
	
	myoglDrawTriang(normal[n], v0,v1,v2);
	myoglDrawTriang(normal[n], v0,v2,v3);
}

void drawCube( float size)  
{
   drawQuad(size, 0, 0,3,2,1);
   drawQuad(size, 1, 2,3,7,6);
   drawQuad(size, 2, 3,0,4,7);
   drawQuad(size, 3, 1,2,6,5);
   drawQuad(size, 4, 4,5,6,7);
   drawQuad(size, 5, 5,4,0,1);
}

void initLight( )
{
	float position[] = {0.5f, 2.f, 0.f, 1.f};
	float low[] = { 0.2f, 0.2f, 0.2f, 1};
	float white[] = { 1, 1, 1, 1};

/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_AMBIENT,  low);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHTING);

	Isso que falta ser feito pelo myogl
	glEnable(GL_LIGHT0);
*/
	myoglLight(low, white, white, position);
}

int main( ) {

	/* Inicializa o myogl com os parametros corretos */
	myoglInitCanvas(640,480);
	myoglPerspective( 45, 4./3, 0.5, 10);
	myoglLookAt(1.5,1.5,1.2, 0,0,0, 0,0,1);
	
	/* Tem que ser antes do initLight */
	myoglMaterial(material[0] ,material[1], material[2], material[3]);
	
	initLight();
	
	/* desenha o cubo */
	drawCube( 1 );
	
	/* Testes para comparar resultado com a resposta de PabloCarneiro */
	myoglTest( -0.5, -0.5, 0.5); 
	myoglTest( 0.5, -0.5, 0.5);
	myoglTest( 0.5, 0.5, 0.5);
	myoglTest( -0.5, 0.5, 0.5);
	myoglTest( 0.5, -0.5, -0.5);
	myoglTest( 0.5, 0.5, -0.5);
	myoglTest( -0.5, 0.5, -0.5); /* */
	
	/* Salva a imagem */
	myoglClose("E:\\Malone\\PUC\\CG\\SVN\\trunk\\t3\\malone_julio.bmp");

	system("PAUSE");

	return 0;
}


