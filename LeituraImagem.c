#include<stdio.h>
#include<stdlib.h>

struct pixel{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

int main(){
	char *arquivoentrada = malloc(100);
	FILE *arqin,*arqout;

	/* ------------------------------*/
	/* Solicita o arquivo de entrada */
	/* ----------------------------- */
	//arquivo de entrada
	printf("Digite o nome do arquivo: \n");
	scanf("%s",arquivoentrada);
	arqin = fopen(arquivoentrada,"rb");
	//valida se o arquivo de entrada existe
	if(arqin==NULL)  printf("Erro na leitura do arquivo.");
	
	//arquivo de saida
	arqout = fopen("novo","wb");
	
	int offset,altura,largura;
	
	//Posiciona e pega o offset - posição que inicia os dados do arquivo 
	fseek(arqin,10,SEEK_SET); //Altera posição corrente dentro do arquivo - SEEK_SET,SEEK_CUR,SEEK_END
    	fread(&offset,4,1,arqin);
    
	//busca largura da imagem
	fseek(arqin,18,SEEK_SET);
	fread(&largura,4,1,arqin);
	
	//busca algura da imagem
	fseek(arqin,22,SEEK_SET);
    	fread(&altura,4,1,arqin);
	
	printf("Altura->%d Largura->%d",altura,largura);
	
	
	//Posiciona e pega o offset - posição que inicia os dados do arquivo 
	fseek(arqin,10,SEEK_SET); //Altera posição corrente dentro do arquivo - SEEK_SET,SEEK_CUR,SEEK_END
    	fread(&offset,4,1,arqin);
    	fseek(arqin,18,SEEK_SET);
    
	
	/* --------------------------- */
	/* ------ Copia cabeçalho ---- */
	/* --------------------------- */
	fseek(arqin,0,SEEK_SET);	
	int i,j;
        for(i=0;i<offset;i++)
	{
		char aux;
		fread(&aux,sizeof(char),1,arqin);	
		//fseek(arqin,1,SEEK_CUR);
		fwrite(&aux,sizeof(char),1,arqout);
	}
	/* --------------------------- */
	
	
	
	/* --------------------------- */
	/*  Copia restante do arquivo  */
	/* --------------------------- */
	unsigned char red,green,blue,media;	
	
	struct pixel *imagem[altura][largura];

	
	
	int lin =0,col=0;
	
	//Monta dados da imagem lida em uma Struct 
	while(fread(&red,sizeof(red),1,arqin)!=0)
	{
		
		fread(&green,sizeof(green),1,arqin);
		fread(&blue,sizeof(blue),1,arqin);
		
		media = (red+green+blue)/3;				
		
		struct pixel *p = (struct pixel*)malloc(sizeof(struct pixel));
		(*p).r = red;
		(*p).g = green;
		(*p).b = blue;

		(imagem[lin][col]) = p;
		
		col++;				
		if (col >= largura)
		{
			lin++;
			col=0;
		}
		
	}
	
	//Gera imagem apartir da Struct
	//Cabeçalho 
	
	//Dados
	fseek(arqout,offset,SEEK_SET);
	for(i=0;i<lin+1;i++){
		for(j=0;j<largura;j++){
			
			if(i < lin-1 || (i==lin && j<col)){
				fwrite(&(imagem[i][j])->r,sizeof(media),1,arqout);
				fwrite(&(imagem[i][j])->g,sizeof(media),1,arqout);
				fwrite(&(imagem[i][j])->b,sizeof(media),1,arqout);
			}
		}
	}

	
	fclose(arqin);
	fclose(arqout);
	
	return 0;
}
