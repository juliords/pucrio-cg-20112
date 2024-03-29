/**
 *	@file object.c Object*: defini��o e opera��es com primitivas.
 *		As primitivas suportadas atualmente s�o: esferas, tri�ngulos e paralelep�pedos.
 *
 *	@date
 *			Criado em:			01 de Dezembro de 2002
 *			�ltima Modifica��o:	05 de outubro de 2009
 *
 */

#include "object.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "algebra.h"

/**
 *   Tipo objeto
 */
struct _Object
{
	/**
	 *  Tipo do objeto.
	 */
	int type;
	/**
	 *  Material* do objeto.
	 */
	int material;
	/**
	 *  Dados do objeto.
	 */
	void *data;
};

/**
 *   Objeto esfera.
 */
struct _Sphere
{

	/**
	 *  Posi��o do centro da esfera na cena.
	 */
	Vector center;

	/**
	 *  Raio da esfera.
	 */
	double radius;
};


/**
 *   Objeto caixa.
 */
struct _Box
{	
	/**
	 *  V�rtice de baixo e � esquerda do paralelep�pedo.
	 */
	Vector bottomLeft;	
	/**
	 *  V�rtice de cima e � direita do paralelep�pedo.
	 */
	Vector topRight;
};

/**
 *   Objeto tri�ngulo.
 */
struct _Triangle
{	
	/**
	 *  Primeiro v�rtice do tri�ngulo.
	 */
	Vector v0;
	/**
	 *  Segundo v�rtice do tri�ngulo.
	 */
	Vector v1;
	/**
	 *  Terceiro v�rtice do tri�ngulo.
	 */
	Vector v2;

	Vector tex0;  /* coordenada de textura do verive 0 */
	Vector tex1;  /* coordenada de textura do verive 1 */
	Vector tex2;  /* coordenada de textura do verive 2 */
};
/**
 *   Objeto malha.
 */
struct _Mesh
{	
	/**
	 *  V�rtice de baixo e � esquerda do paralelep�pedo.
	 */
	Vector bottomLeft;	
	/**
	 *  V�rtice de cima e � direita do paralelep�pedo.
	 */
	Vector topRight;
	/**
	 * Numero de vertices da malha.
	 */ 
	int nvertices;
	/**
	 * Numero de triangulos da malha.
	 */ 
	int ntriangles;
	/**
	 * Vetor dos vertices.
	 */ 
	float* coord;
	/**
	 * Vetor da incidencia dos triangulos.
	 */
	int* triangle;
};

struct _Set
{
	/*
	 * Indica qual operacao entre elementos: uniao, intersecao ou diferenca
	 */
	int relation;

	/*
	 * Objetos armazenados
	 */
	struct _Object* obj[2];
};

/************************************************************************/
/* Constantes Privadas                                                  */
/************************************************************************/
#define MIN( a, b ) ( ( a < b ) ? a : b )
#define MAX( a, b ) ( ( a > b ) ? a : b )

#ifndef EPSILON
#define EPSILON	1.0e-3
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


enum
{
	TYPE_UNKNOWN,
	TYPE_SPHERE,
	TYPE_TRIANGLE,
	TYPE_BOX,
	TYPE_MESH,
	TYPE_SET
};

enum
{
	SET_UNION,
	SET_INTERSECTION,
	SET_DIFFERENCE
};

/************************************************************************/
/* Defini��o das Fun��es Exportadas                                     */
/************************************************************************/

Object* objCreateSet( char* strRelation , Object* obj1 , Object* obj2 )
{
	Object* object;
	Set *set;
	int relation;

	if( !strRelation || !obj1 || !obj2 ) return NULL;

	switch( strRelation[0] )
	{
		case 'U': relation = SET_UNION; printf("\nSET_UNION\n"); break;
		case 'I': relation = SET_INTERSECTION; printf("\nSET_INTERSECTION\n"); break;
		case 'D': relation = SET_DIFFERENCE; printf("\nSET_DIFFERENCE\n"); break;
	}

	object = (Object *)malloc( sizeof(Object) );
	set = (Set *)malloc( sizeof(Set) );
	if(!object || !set) return NULL;

	set->relation = relation;
	set->obj[0] = obj1;
	set->obj[1] = obj2;

	object->type = TYPE_SET;
	object->data = set;

	object->material = obj1->material;

	return object;
}

Object* objCreateSphere( int material, const Vector center, double radius )
{
	Object* object;
	Sphere *sphere;

	object = (Object *)malloc( sizeof(Object) );
	sphere = (Sphere *)malloc( sizeof(Sphere) );

	sphere->center = center;
	sphere->radius = radius;

	object->type = TYPE_SPHERE;
	object->material = material;
	object->data = sphere;

	return object;
}


Object* objCreateTriangle( int material, const Vector v0, const Vector v1, const Vector v2, 
		const Vector tex0, const Vector tex1, const Vector tex2 )
{
	Object* object;
	Triangle *triangle;

	object = (Object *)malloc( sizeof(Object) );
	triangle = (Triangle *)malloc( sizeof(Triangle) );

	triangle->v0 = v0;
	triangle->v1 = v1;
	triangle->v2 = v2;

	triangle->tex0 = tex0;
	triangle->tex1 = tex1;
	triangle->tex2 = tex2;

	object->type = TYPE_TRIANGLE;
	object->material = material;
	object->data = triangle;

	return object;
}


Object* objCreateBox( int material, const Vector bottomLeft, const Vector topRight )
{
	Object* object;
	Box *box;

	object = (Object *)malloc( sizeof(Object) );
	box = (Box *)malloc( sizeof(Box) );

	box->bottomLeft = bottomLeft;
	box->topRight = topRight;

	object->type = TYPE_BOX;
	object->material = material;
	object->data = box;

	return object;
}

Object* objCreateMesh( int material, const Vector bottomLeft, const Vector topRight, const char* filename )
{
	Object* object;
	Mesh* mesh;
	FILE* fp=NULL;

	object = (Object *)malloc( sizeof(Object) );
	mesh = (Mesh*)malloc( sizeof(Mesh) );

	mesh->bottomLeft = bottomLeft;
	mesh->topRight = topRight;

	object->type = TYPE_MESH;
	object->material = material;
	object->data = mesh;

	fp = fopen(filename,"rt");
	if (fp!=NULL) {
		int i;
		float xm,xM,ym,yM,zm,zM;

		fscanf(fp,"%d",&mesh->nvertices);
		mesh->coord = (float*)malloc(3*mesh->nvertices*sizeof(float));
		for (i=0;i<mesh->nvertices;i++){
			fscanf(fp," %f %f %f",&mesh->coord[3*i],&mesh->coord[3*i+1],&mesh->coord[3*i+2]);
		}
		fscanf(fp,"%d",&mesh->ntriangles);
		mesh->triangle=(int*)malloc(3*mesh->ntriangles*sizeof(int));
		for (i=0;i<mesh->ntriangles;i++){
			fscanf(fp," %d %d %d",&mesh->triangle[3*i],&mesh->triangle[3*i+1],&mesh->triangle[3*i+2]);
		}
		xm=xM=mesh->coord[0]; ym=yM=mesh->coord[1]; zm=zM=mesh->coord[2];
		for (i=1;i<mesh->nvertices;i++){
			xm=(xm<mesh->coord[3*i+0])?xm:mesh->coord[3*i+0];
			ym=(ym<mesh->coord[3*i+1])?ym:mesh->coord[3*i+1];
			zm=(zm<mesh->coord[3*i+2])?zm:mesh->coord[3*i+2];
			xM=(xM>mesh->coord[3*i+0])?xM:mesh->coord[3*i+0];
			yM=(yM>mesh->coord[3*i+1])?yM:mesh->coord[3*i+1];
			zM=(zM>mesh->coord[3*i+2])?zM:mesh->coord[3*i+2];
		}
		for (i=0;i<mesh->nvertices;i++){
			mesh->coord[3*i+0] = (float) (bottomLeft.x+(topRight.x-bottomLeft.x)*(mesh->coord[3*i+0]-xm)/(xM-xm));
			mesh->coord[3*i+1] = (float) (bottomLeft.y+(topRight.y-bottomLeft.y)*(mesh->coord[3*i+1]-ym)/(yM-ym));
			mesh->coord[3*i+2] = (float) (bottomLeft.z+(topRight.z-bottomLeft.z)*(mesh->coord[3*i+2]-zm)/(zM-zm));
		}
	}

	return object;
}

double objMeshIntercept(Mesh* mesh,Vector origin,Vector direction,double distance)
{
	int i;
	for (i=0;i<mesh->ntriangles;i++)
	{
		int p0 = mesh->triangle[3*i+0];
		int p1 = mesh->triangle[3*i+1];
		int p2 = mesh->triangle[3*i+2];

		Vector v0 = {mesh->coord[3*p0+0],mesh->coord[3*p0+1],mesh->coord[3*p0+2],1};
		Vector v1 = {mesh->coord[3*p1+0],mesh->coord[3*p1+1],mesh->coord[3*p1+2],1};
		Vector v2 = {mesh->coord[3*p2+0],mesh->coord[3*p2+1],mesh->coord[3*p2+2],1};

		double dividend, divisor;
		double distance = -1.0;

		Vector v0ToV1 = algSub( v1, v0 );
		Vector v1ToV2 = algSub( v2, v1 );
		Vector normal = algCross( v0ToV1, v1ToV2 );
		Vector eyeToV0 = algSub( v0, origin );

		dividend = algDot( eyeToV0, normal );
		divisor = algDot( direction, normal );

		if( divisor <= -EPSILON )
		{
			distance = ( dividend / divisor );
		}

		if( distance >= 0.0001 )
		{
			double a0, a1, a2;

			Vector v2ToV0 = algSub( v0, v2 );
			Vector p = algAdd( origin, algScale( distance, direction ) );
			Vector n0 = algCross( v0ToV1, algSub( p, v0 ) );
			Vector n1 = algCross( v1ToV2, algSub( p, v1 ) );
			Vector n2 = algCross( v2ToV0, algSub( p, v2 ) );

			normal = algUnit(normal);
			a0 = ( 0.5 * algDot( normal, n0 ) );
			a1 = ( 0.5 * algDot( normal, n1 ) );
			a2 = ( 0.5 * algDot( normal, n2 ) );

			if ( (a0>0) && (a1>0) && (a2>0) )  
				return distance;
		}

	}

	return -1.0;
}

double objIntercept( Object* object, Vector eye, Vector ray )
{
	switch( object->type )
	{
		case TYPE_SPHERE:
			{
				Sphere *s = (Sphere *)object->data;

				double a, b, c, delta;
				double distance = -1.0;

				Vector fromSphereToEye;

				fromSphereToEye = algSub( eye, s->center );

				a = algDot( ray, ray );
				b = ( 2.0 * algDot( ray, fromSphereToEye ) );
				c = ( algDot( fromSphereToEye, fromSphereToEye ) - ( s->radius * s->radius ) );

				delta = ( ( b * b ) - ( 4 * a * c ) );

				if( fabs( delta ) <= EPSILON )
				{
					distance = ( -b / (2 * a ) );
				}
				else if( delta > EPSILON )
				{
					double root = sqrt( delta );
					distance = MIN( ( ( -b + root ) / ( 2 * a ) ), ( ( -b - root ) / ( 2.0 * a ) )  );
				}

				return distance;
			}

		case TYPE_TRIANGLE:
			{
				Triangle *t = (Triangle *)object->data;


				double dividend, divisor;
				double distance = -1.0;

				Vector v0ToV1 = algSub( t->v1, t->v0 );
				Vector v1ToV2 = algSub( t->v2, t->v1 );
				Vector normal = algCross( v0ToV1, v1ToV2 );
				Vector eyeToV0 = algSub( t->v0, eye );

				dividend = algDot( eyeToV0, normal );
				divisor = algDot( ray, normal );

				if( divisor <= -EPSILON )
				{
					distance = ( dividend / divisor );
				}

				if( distance >= 0.0001 ) /* teste para ver se e' inteior */
				{
					double a0, a1, a2;

					Vector v2ToV0 = algSub( t->v0, t->v2 );
					Vector p = algAdd( eye, algScale( distance, ray ) );
					Vector n0 = algCross( v0ToV1, algSub( p, t->v0 ) );
					Vector n1 = algCross( v1ToV2, algSub( p, t->v1 ) );
					Vector n2 = algCross( v2ToV0, algSub( p, t->v2 ) );

					normal = algUnit(normal);
					a0 = ( 0.5 * algDot( normal, n0 ) );
					a1 = ( 0.5 * algDot( normal, n1 ) );
					a2 = ( 0.5 * algDot( normal, n2 ) );

					if ( (a0>0) && (a1>0) && (a2>0) )  
						return distance;
				}

				return -1.0;
			}

		case TYPE_BOX:
			{
				Box *box = (Box *)object->data;

				double xmin = box->bottomLeft.x;
				double ymin = box->bottomLeft.y;
				double zmin = box->bottomLeft.z;
				double xmax = box->topRight.x;
				double ymax = box->topRight.y;
				double zmax = box->topRight.z;

				double x, y, z;
				double distance = -1.0;

				if( ray.x > EPSILON || -ray.x > EPSILON )
				{
					if( ray.x > 0 )
					{
						x = xmin;
						distance = ( ( xmin - eye.x ) / ray.x );
					}
					else
					{
						x = xmax;
						distance = ( ( xmax - eye.x ) / ray.x );
					}

					if( distance > EPSILON )
					{
						y = ( eye.y + ( distance * ray.y ) ); 
						z = ( eye.z + ( distance * ray.z ) ); 
						if( ( y >= ymin ) && ( y <= ymax ) && ( z >= zmin ) && ( z <= zmax ) )
							return distance;
					}
				}

				if( ray.y > EPSILON || -ray.y > EPSILON )
				{
					if( ray.y > 0 )
					{
						y = ymin;
						distance = ( ( ymin - eye.y ) / ray.y );
					}
					else
					{
						y = ymax;
						distance = ( ( ymax - eye.y ) / ray.y );
					}

					if( distance > EPSILON )
					{
						x = ( eye.x + ( distance * ray.x ) ); 
						z = ( eye.z + ( distance * ray.z ) ); 
						if( ( x >= xmin ) && ( x <= xmax ) && ( z >= zmin ) && ( z <= zmax ) )
							return distance;
					}

				}

				if( ray.z > EPSILON || -ray.z > EPSILON )
				{
					if( ray.z > 0 )
					{
						z = zmin;
						distance = ( (zmin - eye.z ) / ray.z );
					}
					else
					{
						z = zmax;
						distance = ( ( zmax - eye.z ) / ray.z );
					}

					if( distance > EPSILON )
					{
						x = ( eye.x + ( distance * ray.x ) ); 
						y = ( eye.y + ( distance * ray.y ) ); 
						if( ( x >= xmin ) && ( x <= xmax ) && ( y >= ymin ) && ( y <= ymax ) )	
							return distance;
					}
				}

				return -1.0;
			}
		case TYPE_MESH:
			{
				Mesh* mesh = (Mesh*)object->data;

				double xmin = mesh->bottomLeft.x;
				double ymin = mesh->bottomLeft.y;
				double zmin = mesh->bottomLeft.z;
				double xmax = mesh->topRight.x;
				double ymax = mesh->topRight.y;
				double zmax = mesh->topRight.z;

				double x, y, z;
				double distance = -1.0;

				if( ray.x > EPSILON || -ray.x > EPSILON )
				{
					if( ray.x > 0 )
					{
						x = xmin;
						distance = ( ( xmin - eye.x ) / ray.x );
					}
					else
					{
						x = xmax;
						distance = ( ( xmax - eye.x ) / ray.x );
					}

					if( distance > EPSILON )
					{
						y = ( eye.y + ( distance * ray.y ) ); 
						z = ( eye.z + ( distance * ray.z ) ); 
						if( ( y >= ymin ) && ( y <= ymax ) && ( z >= zmin ) && ( z <= zmax ) )
							return objMeshIntercept(mesh,eye,ray,distance);
					}
				}

				if( ray.y > EPSILON || -ray.y > EPSILON )
				{
					if( ray.y > 0 )
					{
						y = ymin;
						distance = ( ( ymin - eye.y ) / ray.y );
					}
					else
					{
						y = ymax;
						distance = ( ( ymax - eye.y ) / ray.y );
					}

					if( distance > EPSILON )
					{
						x = ( eye.x + ( distance * ray.x ) ); 
						z = ( eye.z + ( distance * ray.z ) ); 
						if( ( x >= xmin ) && ( x <= xmax ) && ( z >= zmin ) && ( z <= zmax ) )
							return objMeshIntercept(mesh,eye,ray,distance);
					}

				}

				if( ray.z > EPSILON || -ray.z > EPSILON )
				{
					if( ray.z > 0 )
					{
						z = zmin;
						distance = ( (zmin - eye.z ) / ray.z );
					}
					else
					{
						z = zmax;
						distance = ( ( zmax - eye.z ) / ray.z );
					}

					if( distance > EPSILON )
					{
						x = ( eye.x + ( distance * ray.x ) ); 
						y = ( eye.y + ( distance * ray.y ) ); 
						if( ( x >= xmin ) && ( x <= xmax ) && ( y >= ymin ) && ( y <= ymax ) )	
							return objMeshIntercept(mesh,eye,ray,distance);
					}
				}

				return -1.0;
			}
		case TYPE_SET:
			{
				Set* set = (Set*) object->data;

				double aIn, bIn, aOut, bOut;		

				switch ( set->relation ) {
					case SET_UNION:
						{
							double dis1 = objIntercept( set->obj[0], eye, ray );
							double dis2 = objIntercept( set->obj[1], eye, ray );

							if(dis1 == -1.0) return dis2;
							if(dis2 == -1.0) return dis1;
							return MIN( dis1 , dis2 );
						}
					case SET_DIFFERENCE:
						{
							aIn = objIntercept( set->obj[0], eye, ray );
							bIn = objIntercept( set->obj[1], eye, ray );

							aOut = objInterceptMax( set->obj[0], eye, ray );
							bOut = objInterceptMax( set->obj[1], eye, ray );

							if ( aIn > bIn  )
							{
								return bIn;
							}
							else if ( bOut > aOut )
							{
								return bOut;
							}
							else
							{
								return -1.0;
							}
						}
					case SET_INTERSECTION:
						{
							aIn = objIntercept( set->obj[0], eye, ray );
							bIn = objIntercept( set->obj[1], eye, ray );

							aOut = objInterceptMax( set->obj[0], eye, ray );
							bOut = objInterceptMax( set->obj[1], eye, ray );

							if(aIn == -1.0 || bIn == -1.0) return -1.0;
							if ( ( aIn < bIn ) && ( aOut > bIn ) )
							{
								return bIn;
							}
							else if ( (bIn < aIn ) && ( bOut > aIn ) )
							{
								return aIn;
							}
							else
							{
								return -1.0;
							}
						}
				}

				return -1.0;
			}
		default:
			/* Tipo de Objeto Inv�lido: nunca deve acontecer */
			return -1.0;
	}
}

double objInterceptMax( Object* object, Vector eye, Vector ray ){
	switch( object->type )
	{
		case TYPE_SPHERE:
			{
				Sphere *s = (Sphere *)object->data;

				double a, b, c, delta;
				double distance = -1.0;

				Vector fromSphereToEye;

				fromSphereToEye = algSub( eye, s->center );

				a = algDot( ray, ray );
				b = ( 2.0 * algDot( ray, fromSphereToEye ) );
				c = ( algDot( fromSphereToEye, fromSphereToEye ) - ( s->radius * s->radius ) );

				delta = ( ( b * b ) - ( 4 * a * c ) );

				if( fabs( delta ) <= EPSILON )
				{
					distance = ( -b / (2 * a ) );
				}
				else if( delta > EPSILON )
				{
					double root = sqrt( delta );
					distance = MAX( ( ( -b + root ) / ( 2 * a ) ), ( ( -b - root ) / ( 2.0 * a ) )  );
				}

				return distance;
			}

		case TYPE_TRIANGLE:
			{
				/* Triangulo e' plano, retorna o mesmo ponto de intersection */
				return objIntercept( object, eye, ray );
			}

		case TYPE_BOX:
			{
				/* Tem que pegar a maior distancia, ta pegando a menor */
				Box *box = (Box *)object->data;

				double xmin = box->bottomLeft.x;
				double ymin = box->bottomLeft.y;
				double zmin = box->bottomLeft.z;
				double xmax = box->topRight.x;
				double ymax = box->topRight.y;
				double zmax = box->topRight.z;

				double x, y, z;
				double distance = -1.0;

				if( ray.x > EPSILON || -ray.x > EPSILON )
				{
					if( ray.x < 0 )
					{
						x = xmin;
						distance = ( ( xmin - eye.x ) / ray.x );
					}
					else
					{
						x = xmax;
						distance = ( ( xmax - eye.x ) / ray.x );
					}

					if( distance > EPSILON )
					{
						y = ( eye.y + ( distance * ray.y ) ); 
						z = ( eye.z + ( distance * ray.z ) ); 
						if( ( y >= ymin ) && ( y <= ymax ) && ( z >= zmin ) && ( z <= zmax ) )
							return distance;
					}
				}

				if( ray.y > EPSILON || -ray.y > EPSILON )
				{
					if( ray.y < 0 )
					{
						y = ymin;
						distance = ( ( ymin - eye.y ) / ray.y );
					}
					else
					{
						y = ymax;
						distance = ( ( ymax - eye.y ) / ray.y );
					}

					if( distance > EPSILON )
					{
						x = ( eye.x + ( distance * ray.x ) ); 
						z = ( eye.z + ( distance * ray.z ) ); 
						if( ( x >= xmin ) && ( x <= xmax ) && ( z >= zmin ) && ( z <= zmax ) )
							return distance;
					}

				}

				if( ray.z > EPSILON || -ray.z > EPSILON )
				{
					if( ray.z < 0 )
					{
						z = zmin;
						distance = ( (zmin - eye.z ) / ray.z );
					}
					else
					{
						z = zmax;
						distance = ( ( zmax - eye.z ) / ray.z );
					}

					if( distance > EPSILON )
					{
						x = ( eye.x + ( distance * ray.x ) ); 
						y = ( eye.y + ( distance * ray.y ) ); 
						if( ( x >= xmin ) && ( x <= xmax ) && ( y >= ymin ) && ( y <= ymax ) )	
							return distance;
					}
				}

				return -1.0;
			}
		case TYPE_MESH:
			break;
		case TYPE_SET:
			{
				Set* set = (Set*) object->data;

				double aIn, bIn, aOut, bOut;		

				switch ( set->relation ) {
					case SET_UNION:
						{
							return MIN( objInterceptMax( set->obj[0], eye, ray ), objInterceptMax( set->obj[1], eye, ray ) );
						}
					case SET_DIFFERENCE:
						{
							aIn = objIntercept( set->obj[0], eye, ray );
							bIn = objIntercept( set->obj[1], eye, ray );

							aOut = objInterceptMax( set->obj[0], eye, ray );
							bOut = objInterceptMax( set->obj[1], eye, ray );

							if ( aIn < bIn  )
							{
								return bIn;
							}
							else if ( bOut < aOut )
							{
								return aOut;
							}						
						}
					case SET_INTERSECTION:
						{
							aIn = objIntercept( set->obj[0], eye, ray );
							bIn = objIntercept( set->obj[1], eye, ray );

							aOut = objInterceptMax( set->obj[0], eye, ray );
							bOut = objInterceptMax( set->obj[1], eye, ray );

							if ( ( aIn < bIn ) && ( aOut > bIn ) )
							{
								return bOut;
							}
							else if ( (bIn < aIn ) && ( bOut > aIn ) )
							{
								return aOut;
							}					
						}
				}

				return -1.0;
			}
		default:
			/* Tipo de Objeto Inv�lido: nunca deve acontecer */
			return -1.0;
	}
}

Vector objInterceptExit( Object* object, Vector point, Vector d )
{
	switch( object->type )
	{
		case TYPE_SPHERE:
			{
				Sphere *s = (Sphere *)object->data;

				double a, b, c, delta, distance;
				//double distance = -1.0;

				Vector fromSphereToEye;

				fromSphereToEye = algSub( point, s->center );

				a = algDot( d, d );
				b = ( 2.0 * algDot( d, fromSphereToEye ) );
				c = ( algDot( fromSphereToEye, fromSphereToEye ) - ( s->radius * s->radius ) );

				delta = ( ( b * b ) - ( 4 * a * c ) );

				if( fabs( delta ) <= EPSILON )
				{
					distance = ( -b / (2 * a ) );
				}
				else if( delta > EPSILON )
				{
					double root = sqrt( delta );
					distance = MAX( ( ( -b + root ) / ( 2 * a ) ), ( ( -b - root ) / ( 2.0 * a ) )  );
				}

				return algAdd(point, algScale(distance, d));
			}

		case TYPE_TRIANGLE:
		case TYPE_BOX:
			break;
	}
	return point;
}

static int compareVector( Vector a , Vector b )
{
	if(a.x - b.x > EPSILON) return 0;
	if(a.y - b.y > EPSILON) return 0;
	if(a.z - b.z > EPSILON) return 0;
	else return 1;
}

int verifyObjSet( Object* set , Vector point )
{
	Set* iset;
	int i;

	if(set->type != TYPE_SET) return -1;

	iset = (Set*)set->data;

	for(i = 0; i < 2; i++)
	{
		Object* obj = iset->obj[i];

		if(obj->type == TYPE_SPHERE)
		{
			Sphere *sphere = (Sphere*)obj->data;
			Vector vet = algSub( point , sphere->center );

			if( fabs( sphere->radius - algNorm( vet ) ) < EPSILON )
				return i;
		}
		else if(obj->type == TYPE_TRIANGLE)
		{
			Triangle *triangle = (Triangle *)obj->data;

			//normal gerada entre esses tres pontos deve ser igual
			Vector norm1 = algUnit( algCross( algSub( triangle->v0 , triangle->v1 ) , algSub( triangle->v0, triangle->v2 ) ) );
			Vector norm2 = algUnit( algCross( algSub( triangle->v0 , triangle->v1 ) , algSub( triangle->v0, point ) ) );
			Vector norm3 = algUnit( algCross( algSub( triangle->v0 , point ) , algSub( triangle->v0, triangle->v2 ) ) );

			if( compareVector( norm1 , norm2 ) && compareVector( norm2 , norm3) )
			{
				return i;
			}	
		}
		else if(obj->type == TYPE_BOX)
		{
			Box *box = (Box *)obj->data;
			/* Seleciona a face mais pr�xima de point */
			if( fabs( point.x - box->bottomLeft.x ) < EPSILON )
			{
				return i;
			}
			else if( fabs( point.x - box->topRight.x ) < EPSILON )
			{
				return i;
			}
			else if( fabs( point.y - box->bottomLeft.y ) < EPSILON )
			{
				return i;
			}
			else if( fabs( point.y - box->topRight.y ) < EPSILON )
			{
				return i;
			}
			else if( fabs( point.z - box->bottomLeft.z ) < EPSILON )
			{
				return i;
			}
			else if( fabs( point.z - box->topRight.z ) < EPSILON )
			{
				return i;
			}
		}
		else if(obj->type == TYPE_SET)
		{
			int ret = verifyObjSet( obj , point );
			if(ret != -1) return i;
		}

	}
	return -1;
}

Vector objNormalAt( Object* object, Vector point )
{
	if( object->type == TYPE_SPHERE )
	{
		Sphere *sphere = (Sphere *)object->data;

		return algScale( ( 1.0 / sphere->radius ),
				algSub( point, sphere->center ) );
	}
	else if ( object->type == TYPE_TRIANGLE )
	{
		Triangle *triangle = (Triangle *)object->data;

		return algCross( algSub( triangle->v1, triangle->v0 ),
				algSub( triangle->v2, triangle->v0 ) );
	}
	else if ( object->type == TYPE_BOX )
	{
		Box *box = (Box *)object->data;
		/* Seleciona a face mais pr�xima de point */
		if( fabs( point.x - box->bottomLeft.x ) < EPSILON )
		{
			return algVector( -1, 0, 0, 1  );
		}
		else if( fabs( point.x - box->topRight.x ) < EPSILON )
		{
			return algVector( 1, 0, 0, 1 );
		}
		else if( fabs( point.y - box->bottomLeft.y ) < EPSILON )
		{
			return algVector( 0, -1, 0, 1 );
		}
		else if( fabs( point.y - box->topRight.y ) < EPSILON )
		{
			return algVector( 0, 1, 0, 1 );
		}
		else if( fabs( point.z - box->bottomLeft.z ) < EPSILON )
		{
			return algVector( 0, 0, -1, 1 );
		}
		else if( fabs( point.z - box->topRight.z ) < EPSILON )
		{
			return algVector( 0, 0, 1, 1 );
		}
		else
		{
			return algVector( 0, 0, 0, 1 );
		}
	} 
	else if ( object->type == TYPE_SET )
	{
		Set* set = (Set*)object->data;
		int i = verifyObjSet(object,point);
		
		if(i >= 0)
		{
			object->material = set->obj[i]->material;
		}

		if(i < 0) return algVector( 0, 0, 0, 1 );
		else if(set->relation == SET_DIFFERENCE && i == 1)
		{
			return algMinus( objNormalAt(set->obj[i], point) );
		}
		else return objNormalAt(set->obj[i], point);
	}
	else
	{
		/* Tipo de Objeto Inv�lido: nunca deve acontecer */
		return algVector( 0, 0, 0, 1 );
	}
}

Vector objTextureCoordinateAt( Object* object, Vector point )
{
	if( object->type == TYPE_SPHERE )
	{
		/*...*/
		return algVector( 0, 0, 0, 1 );
	} 
	else if( object->type == TYPE_TRIANGLE )
	{
		/*...*/
		return algVector( 0, 0, 0, 1 );
	} 
	else if( object->type == TYPE_BOX )
	{
		/*...*/
		return algVector( 0, 0, 0, 1 );
	} 

	/* Tipo de Objeto Inv�lido: nunca deve acontecer */
	return algVector( 0, 0, 0, 1 );	
}

int objGetMaterial( Object* object )
{
	return object->material;
}

void objDestroy( Object* object )
{
	if( object->type == TYPE_SET )
	{
		Set* iset = (Set*)object->data;
		objDestroy(iset->obj[0]);
		objDestroy(iset->obj[1]);
	}

	free( object->data );
	free( object );
}
