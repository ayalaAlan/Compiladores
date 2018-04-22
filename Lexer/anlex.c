/*
 *	Analizador Léxico
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Práctica de Programación Nro. 1
 *
 *	Descripcion:
 *	Implementa un analizador léxico que reconoce números, identificadores,
 * 	palabras reservadas, operadores y signos de puntuación para un lenguaje
 * 	con sintaxis tipo Pascal.
 *
 */

/*********** Inclusión de cabecera **************/
#include "anlex.h"
#define NRO_CODIGOS 11	//nro de codigos definido en anlex.h


/************* Variables globales **************/

int consumir;			/* 1 indica al analizador lexico que debe devolver
						el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];		// string utilizado para cargar mensajes de error
token t;				// token global para recibir componentes del Analizador Lexico

// variables para el analizador lexico

FILE *archivo;			// Fuente json
FILE *fileOut;			//salida txt
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char id[TAMLEX];		// Utilizado por el analizador lexico
int delantero=-1;		// Utilizado por el analizador lexico
int fin=0;				// Utilizado por el analizador lexico
int numLinea=1;			// Numero de Linea

/**************** Funciones **********************/


// Rutinas del analizador lexico

void error(const char* mensaje)
{
	printf("Lin %d: Error Lexico. %s.\n",numLinea,mensaje);
}

void sigLex()
{
	int i=0;
	char c=0;
	int acepto=0;
	int estado=0;
	char msg[41];
	int nroCaracter; //variable utilizada para validar caracteres especiales
	entrada e;

	while((c=fgetc(archivo))!=EOF)
	{

		if (c==' '){
			fprintf(fileOut," ");
			continue;	//eliminar espacios en blanco
		}else if( c=='\t'){
			fprintf(fileOut,"\t");
			continue;
		}
		else if(c=='\n')
		{
			//incrementar el numero de linea
			fprintf(fileOut,"\n");
			numLinea++;
			continue;
		}
		else if (isalpha(c))
		{
			//false o true
			i=0;
			do{
				id[i]=c;
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tamaño de buffer");
			}while(isalpha(c));
			id[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			break;
		}

		else if (isdigit(c))
		{
				//es un numero
				i=0;
				estado=0;
				acepto=0;
				id[i]=c;

				while(!acepto)
				{
					switch(estado){
					case 0: //una secuencia netamente de digitos, puede ocurrir . o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=0;
						}
						else if(c=='.'){
							id[++i]=c;
							estado=1;
						}
						else if(tolower(c)=='e'){
							id[++i]=c;
							estado=3;
						}
						else{
							estado=6;
						}
						break;

					case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(c=='.')
						{
							i--;
							fseek(archivo,-1,SEEK_CUR);
							estado=6;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 2://la fraccion decimal, pueden seguir los digitos o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(tolower(c)=='e')
						{
							id[++i]=c;
							estado=3;
						}
						else
							estado=6;
						break;
					case 3://una e, puede seguir +, - o una secuencia de digitos
						c=fgetc(archivo);
						if (c=='+' || c=='-')
						{
							id[++i]=c;
							estado=4;
						}
						else if(isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 4://necesariamente debe venir por lo menos un digito
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 5://una secuencia de digitos correspondiente al exponente
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							estado=6;
						}break;
					case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
						if (c!=EOF)
							ungetc(c,archivo);
						else
							c=0;
						id[++i]='\0';
						acepto=1;
						t.pe=buscar(id);
						if (t.pe->compLex==-1)
						{
							strcpy(e.lexema,id);
							e.compLex=LIT_NUMERICO;
							insertar(e);
							t.pe=buscar(id);
						}
						t.compLex=LIT_NUMERICO;
						break;
					case -1:
						if (c==EOF)
							error("No se esperaba el fin de archivo");
						else
							error(msg);
						exit(1);
					}
				}
			break;
		}
		else if (c==':')
		{
            //asignacion
            t.compLex=DOS_PUNTOS;
            t.pe=buscar(":");
			break;
		}
		else if (c==',')
		{
			t.compLex=COMA;
			t.pe=buscar(",");
			break;
		}
		else if (c=='[')
		{
			t.compLex=L_CORCHETE;
			t.pe=buscar("[");
			break;
		}
		else if (c==']')
		{
			t.compLex=R_CORCHETE;
			t.pe=buscar("]");
			break;
		}
		else if (c=='\"')
		{//cadena de caracteres
			i=0;
			id[i]=c;
			i++;
			do{
				c=fgetc(archivo);
				if (c=='\"')
				{
					c=fgetc(archivo);
					if (c=='\"')
					{
						id[i]=c;
						i++;
						id[i]=c;
						i++;
					}
					else
					{
						id[i]='\"';
						i++;
						break;
					}
				}
				else if(c==EOF)
				{
					error("Se llego al fin de archivo sin finalizar un literal");
				}
				else{
					id[i]=c;
					i++;
				}
				nroCaracter = (int) c;	//devuelve numero negativo si es caracter especial
			}while(isascii(c) || nroCaracter < 0);
			id[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			if (t.pe->compLex==-1)
			{
				strcpy(e.lexema,id);
                e.compLex=LIT_CADENA;
				insertar(e);
				t.pe=buscar(id);
				t.compLex=e.compLex;
			}
			break;
		}
		else if (c=='{')
		{
			t.compLex=L_LLAVE;
			t.pe=buscar("{");
			break;
		}
		else if (c=='}')
		{
			t.compLex=R_LLAVE;
			t.pe=buscar("}");
			break;
		}
		else if (c!=EOF)
		{
			sprintf(msg,"%c no esperado",c);
			error(msg);
		}
	}
	if (c==EOF)
	{
		t.compLex=EOF;
		// strcpy(e.lexema,"EOF");
		sprintf(e.lexema,"EOF");
		t.pe=&e;
	}

}
int buscarPos(int compLex){
	int pos,i;
	int codigos[NRO_CODIGOS] = {-1,256,257,258,259,260,261,262,263,264}; //vector de codigos del compilador
	for(i=1;i<NRO_CODIGOS;i++){
		if(compLex == codigos[i] )
			return i;
	}

return 0;
}

int main(int argc,char* args[])
{
	//prototipo
	int buscarPos(int compLex);
	// inicializar analizador lexico

	initTabla();
	initTablaSimbolos();
	//almacena los nombres de los componentes lexicos (solo se utiliza para la impresion)
	char nomCodigos[NRO_CODIGOS][TAMLEX] = {"","L_CORCHETE","R_CORCHETE","L_LLAVE","R_LLAVE","COMA","DOS_PUNTOS","CADENA","NUMERO","BOOLEAN"};
	int pos;
	if(argc > 1)
	{
		if (!(archivo=fopen(args[1],"rt")))
		{
			printf("Archivo no encontrado.\n");
			exit(1);
		}
		if(!(fileOut = fopen("salida.txt","w"))){
			printf("\nUps ocurrio un error al crear el archivo de salida... ");
			exit(1);
		}
		while (t.compLex!=EOF){
			sigLex();
			pos= buscarPos(t.compLex);	//trae la posicion del tipo del componente lexico
			fprintf(fileOut,"%s ",nomCodigos[pos]);
		}
		fclose(archivo);
	}else{
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}

	return 0;
}
