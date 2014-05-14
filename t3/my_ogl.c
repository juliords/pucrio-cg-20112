#include "image.h"
#include "my_ogl.h"

#include <math.h>
#include <stdio.h>

#define PI 3.1415926535897932384626433

Image* canvas = NULL;

float light_ambient[3]; 
float light_diffuse[3]; 
float light_specular[3]; 
float light_position[3]; 

float* material_ambient; 
float* material_diffuse; 
float* material_specular;
float* material_shininess;

Matrix modelview, projection;
Vector external_eye;

void myoglInitCanvas(int w, int h) {
	canvas = imgCreate(w,h);
}

void myoglClose(char* name) {
	imgWriteBMP(name, canvas);
	imgDestroy(canvas);
}

void myoglLight(float* ambient,float* diffuse,float* specular,float* position)
{
	Vector ka, kd, ks;
	int i;
	
	for ( i = 0; i < 3; i++ ) {
		light_ambient[i] = ambient[i];
		light_diffuse[i] = diffuse[i];
		light_specular[i] = specular[i];
		light_position[i] = position[i];
	}
	
//	ka = algVector( material_ambient[0], material_ambient[1], material_ambient[2]);
//	kd = algVector( material_diffuse[0], material_diffuse[1], material_diffuse[2]);
//	ks = algVector( material_specular[0], material_specular[1], material_specular[2]);
	
}

float graus2rad( float graus ) {
	return ( graus / 180 ) * PI;
}

/* n = nearPlane e f = farPlane */
void myoglPerspective(float fovy, float aspect, float n, float f)
{
	float a, b;
	
	a = 2 * n * tan( graus2rad( fovy / 2 ) );		/* altura */
	b = a * aspect;									/* largura */
	
	/* r+l=0, t+b=0, r-l=b, t+b=a */
	projection = algMatrix4x4(	(2 * n)/b,	0,			0,							0,
								0,			(2 * n)/a, 	0,							0,
								0,			0,			- ( (f + n) / (f - n) ),	- (( 2 * f * n ) / (f - n)),
								0,			0,			-1,							0);
								
	algMatrixPrint( "Matriz projection", projection );
}

void myoglLookAt(float ex, float ey, float ez, float cx, float cy, float cz, float upx, float upy, float upz)
{
	Vector eye, center, up;
	Vector xe, ye, ze;
	Matrix r, t;
	
	/* Tranforma os parametros em vetor */
	external_eye = eye = algVector( ex, ey, ez, 1 );
	center = algVector( cx, cy, cz, 1 );
	up = algVector( upx, upy, upz, 1 );
	
	/* Calcula as coordenadas em eye */
	ze = algScale( ( 1 /  algNorm(algSub(eye, center)) ), (algSub( eye, center )));
	xe = algScale( ( 1 / algNorm(algCross( up, ze )) ), (algCross( up, ze )));
	ye = algCross( ze, xe );
	
	/* Translada para a origem */
	t = algMatrix4x4(	1,	0,	0,	-ex,
						0,	1,	0,	-ey,
						0,	0,	1,	-ez,
						0,	0,	0,	1);
	
	/* Roda o sistema */
	r = algMatrix4x4(	algGetX(xe),	algGetY(xe),	algGetZ(xe),	0,
						algGetX(ye),	algGetY(ye),	algGetZ(ye),	0,
						algGetX(ze),	algGetY(ze),	algGetZ(ze),	0,
						0,				0,				0,				1);
						
	/* modelview é a lookat. RT */
	modelview = algMult( r, t);
	
	algMatrixPrint( "Matriz modelview", modelview );	
}

void myoglMaterial( float* ambiente, float* diffuse, float* specular, float* shininess) 
{
	material_ambient = ambiente;
	material_diffuse = diffuse;
	material_specular = specular;
	material_shininess = shininess;
}
Vector algCross1( Vector v1, Vector v2 ) 
{
  Vector v = {
    v1.x* v2.x, 
    v1.y* v2.y, 
    v1.z* v2.z, 
    1
  };
  return v;
}

void myoglDrawTriang(float* normal, float* v0, float* v1, float* v2) 
{
	Vector pos[3], norm[3], v01, v12, v20, vert0, vert1, vert2, p, t1, t2, t3, luz, l, v, r, n, rgb;
	int px[3], py[3];
	Color color, colorw;
	int w = imgGetWidth(canvas), h = imgGetHeight(canvas);
	int i, j;
	Vector vlight_ambient = algVector(light_ambient[0],light_ambient[1],light_ambient[2],1);
	Vector vlight_diffuse = algVector(light_diffuse[0],light_diffuse[1],light_diffuse[2],1);
	Vector vmaterial_specular = algVector(material_specular[0],material_specular[1],material_specular[2],1);
	Vector vmaterial_diffuse = algVector(material_diffuse[0],material_diffuse[1],material_diffuse[2],1);
	
	/* transforma os valores passados em vetores */
	pos[0] = algVector( v0[0], v0[1], v0[2], 1);
	pos[1] = algVector( v1[0], v1[1], v1[2], 1);
	pos[2] = algVector( v2[0], v2[1], v2[2], 1);
	
	for ( i = 0; i < 3; i++ ) {
		/* conta para pegar as coordenadas x e y (acredite) */
		norm[i] = algUnit( algTransf( projection, algTransf( modelview, pos[i])));
		
		/* Posicao x e y da imagem */
		px[i] = w * ( algGetX(norm[i]) + 1 ) / 2;
		py[i] = h * ( algGetY(norm[i]) + 1) / 2;	/* era pra ser py[i] = h - h * ..., mas tava invertida */

		/* pinta o vertice */
		imageSetPixel( canvas, px[i], py[i], colorCreate3b( 25, 100, 55));
	}

	v01 = algSub( algVector( px[0], py[0], 0, 1 ), algVector( px[1], py[1], 0, 1) );
	v12 = algSub( algVector( px[1], py[1], 0, 1 ), algVector( px[2], py[2], 0, 1) );
	v20 = algSub( algVector( px[2], py[2], 0, 1 ), algVector( px[0], py[0], 0, 1) );

	vert0 = algVector( px[0], py[0], 0, 1);
	vert1 = algVector( px[1], py[1], 0, 1);
	vert2 = algVector( px[2], py[2], 0, 1);

	luz = algVector(light_position[0],light_position[1],light_position[2],1);
	n = algVector(normal[0],normal[1],normal[2],1);

	l =  algSub(pos[0],luz);
	v = algUnit(algSub(external_eye, pos[0]));
	r = algSub(algScale(2,algScale(algDot(luz, n), n)), luz) ;	

	rgb = algAdd(algCross1(vlight_ambient, vmaterial_diffuse), algAdd(algScale(algDot(n,l), algCross1(vlight_diffuse, vmaterial_diffuse)), algScale(algDot(r,v), algCross1(vlight_diffuse, vmaterial_specular))));
	color = colorCreate3b( algGetX(rgb), algGetY(rgb), algGetZ(rgb));
	colorw = colorCreate3b( 255, 255, 255);

	/* Pinta todos os pontos internos ao triangulo */
	for ( i = 0; i < w; i++ ) {
		for ( j = 0; j < h; j++ ) {
			p = algVector( i, j, 0, 1 );

			t1 = algCross(v01, algSub(p, vert0));
			t2 = algCross(v12, algSub(p, vert1));
			t3 = algCross(v20, algSub(p, vert2));

			if( algDot(t1, t2) > 0 && algDot(t1,t3) > 0 )
			{
				imageSetPixel( canvas, i, j, color);
			}
		}
	}
}

void myoglTest( float x, float y, float z ){

	Vector vet = algVector( x, y, z, 1), luz;
	int w = imgGetWidth(canvas), h = imgGetHeight(canvas);
	
	Vector norm = algUnit(algTransf(projection, algTransf( modelview, vet)) );

	luz = algVector(light_position[0],light_position[1],light_position[2],1);
	
//	algVectorPrint( "Mult pelo projection", algTransf( projection, vet) );
//	algVectorPrint( "Mult pelo modelview", algTransf( modelview, vet) );	
//	algVectorPrint( "projection do mult pelo modelview", algTransf(projection, algTransf( modelview, vet) ));
	algVectorPrint( "Vetor Luz", algSub(vet,luz) );
	algVectorPrint( "Vetor V(Eye)", algUnit(algSub(external_eye, vet)) );
	
	algVectorPrint( "\nval", norm);
	printf( "pos x = %f\npos y = %f \n\n", w * ( algGetX(norm) + 1 ) / 2,  h - h * ( algGetY(norm) + 1) / 2);

}
