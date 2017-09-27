#include<stdio.h>
#include<stdlib.h>

typedef struct pixel{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Pixel;

#pragma pack (push,1)
typedef struct cabecalho{
	unsigned short 	tipo;
	int 	       	tam_arquivo;
	unsigned short	reservado1;
	unsigned short 	reservado2;
	int		deslocamento;

	int		tam_cabecalho;
	int		largura_img;
	int		altura_img;
	unsigned short	num_planos;
	unsigned short	qtde_bits_pixel;
	int		compressao;
	int 		tam_imagem;
	int		resolucao_hori;
	int		resolucao_ver;
	int		num_cores_usu_img;
	int		num_cores_imp;
} Cabecalho;

void escrevesaida(FILE *Arquivo, char *linha){
	fprintf(Arquivo,"%s \n",linha);
}

int main(){
	
	char *arquivoentrada = malloc(100);
	FILE *arqin,*arqout,*txtsaida;
		

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
	txtsaida = fopen("saida.txt","w");
	if(txtsaida == NULL){
		printf("Erro na criação do arquivo.");
		exit;
	}
	
	//Busca dados do cabeçalho da imagem e grama em estrutura	
	Cabecalho *cabecalho = malloc(sizeof(Cabecalho));
	
	fseek(arqin,0,SEEK_SET);
	fread(&(*cabecalho).tipo	,2,1,arqin);
	fseek(arqin,2,SEEK_SET);
	fread(&(*cabecalho).tam_arquivo	,4,1,arqin);
	fseek(arqin,6,SEEK_SET);
	fread(&(*cabecalho).reservado1	,2,1,arqin);
	fseek(arqin,8,SEEK_SET);
	fread(&(*cabecalho).reservado2	,2,1,arqin);
	fseek(arqin,10,SEEK_SET);
	fread(&(*cabecalho).deslocamento,4,1,arqin);

	fseek(arqin,14,SEEK_SET);
	fread(&(*cabecalho).tam_cabecalho	,4,1,arqin);
	fseek(arqin,18,SEEK_SET);
	fread(&(*cabecalho).largura_img		,4,1,arqin);
	fseek(arqin,22,SEEK_SET);
	fread(&(*cabecalho).altura_img		,4,1,arqin);
	fseek(arqin,26,SEEK_SET);
	fread(&(*cabecalho).num_planos		,2,1,arqin);
	fseek(arqin,28,SEEK_SET);
	fread(&(*cabecalho).qtde_bits_pixel	,2,1,arqin);
	fseek(arqin,30,SEEK_SET);
	fread(&(*cabecalho).compressao		,4,1,arqin);
	fseek(arqin,34,SEEK_SET);
	fread(&(*cabecalho).tam_imagem		,4,1,arqin);
	fseek(arqin,38,SEEK_SET);
	fread(&(*cabecalho).resolucao_hori	,4,1,arqin);
	fseek(arqin,42,SEEK_SET);
	fread(&(*cabecalho).resolucao_ver	,4,1,arqin);
	fseek(arqin,46,SEEK_SET);
	fread(&(*cabecalho).num_cores_usu_img	,4,1,arqin);
	fseek(arqin,50,SEEK_SET);
	fread(&(*cabecalho).num_cores_imp	,4,1,arqin);

	printf("%d - %d - %d \n\n",(*cabecalho).altura_img,(*cabecalho).largura_img,(*cabecalho).tam_arquivo);

	
	/* --------------------------- */
	/* ------ Copia cabeçalho ---- */
	/* --------------------------- */
	fseek(arqin,0,SEEK_SET);	
	int i,j;
        for(i=0;i<(*cabecalho).deslocamento;i++)
	{
		char aux;
		fread(&aux,sizeof(char),1,arqin);	
		//fseek(arqin,1,SEEK_CUR);
		fwrite(&aux,sizeof(char),1,arqout);
	}
	/* --------------------------- */
	
	
	/* ------------------------------------------ */
	/*  Monta dados da imagem lida em uma Struct  */
	/* ------------------------------------------ */
	unsigned char red,green,blue,media;	
	
	Pixel *imagem[(*cabecalho).altura_img][(*cabecalho).largura_img];
	
	int lin =0,col=0;
	
	while(fread(&red,sizeof(blue),1,arqin)!=0)
	{
		
		fread(&green,sizeof(green),1,arqin);
		fread(&blue,sizeof(red),1,arqin);
		
		media = (red+green+blue)/3;				
		
		struct pixel *p = (struct pixel*)malloc(sizeof(struct pixel));
		(*p).r = red;
		(*p).g = green;
		(*p).b = blue;

		(imagem[lin][col]) = p;
		
		col++;				
		if (col >= (*cabecalho).largura_img)
		{
			lin++;
			col=0;
		}
		
	}
	

        /* ------------------------------- */
	/*  Transforma para tons de cinza  */
	/* ------------------------------- */
	for(i=0;i<lin;i++){
		for(j=0;j<(*cabecalho).largura_img;j++){
			
			if(i < lin || (i==lin && j<col)){
				double media = (((imagem[i][j])->r * 0.3)+ ((imagem[i][j])->g * 0.59)+((imagem[i][j])->b) * 0.11);

				(imagem[i][j])->r = (int) media;
				(imagem[i][j])->g = (int) media;
				(imagem[i][j])->b = (int) media;
			}
		}
	}
	
	/* ------------------------------- */
	/* 	 Busca Limite superior     */
	/* ------------------------------- */
	int limite_sup = 0;	
	int limite[lin	];	
	for(i=0;i<lin;i++){
		limite[i]=0;	
	}
	for(j=0;j<(*cabecalho).largura_img;j++){
		for(i=lin-1;i>=0;i--){
			if(i < lin || (i==lin && j<col)){
				if((imagem[i][j])->r > 30 && lin-i>68 ){
					limite[lin-i]++;				
					//printf("|%d - %d> %d |\t",i,j,(imagem[i][j])->r );
					break;
				}
			}
		}
	 	//printf("\n");
	}
	for(i=0;i<lin;i++){
		if(limite[i] > limite[limite_sup]){
			limite_sup = i;
		}	
	}

	/* ------------------------------- */
	/* 	 Busca Limite inferior     */
	/* ------------------------------- */
	int limite_inf = 0;	
	for(i=0;i<lin;i++){
		limite[i]=0;	
	}
	for(j=0;j<(*cabecalho).largura_img;j++){
		for(i=0;i<lin;i++){
			if(i < lin || (i==lin && j<col)){
				if((imagem[i][j])->r > 30){
					limite[i]++;				
					//printf("|%d - %d> %d |\t",i,j,(imagem[i][j])->r );
					break;
				}
			}
		}
	 	//printf("\n");
	}
	for(i=0;i<lin;i++){
		if(limite[i] > limite[limite_inf]){
			limite_inf = i;
		}	
	}
	limite_inf = lin - limite_inf;
	printf("*******************************\n");
	printf("LIMITE_SUPERIOS->%d \n",limite_sup);
	printf("LIMITE_INFERIOR->%d \n",limite_inf);
	printf("*******************************\n");

	/* ----------------------------------------- */
	/*  Segmentação da Imagem em Preto e Branco  */
	/* ----------------------------------------- */
	for(i=0;i<lin;i++){
		for(j=0;j<(*cabecalho).largura_img;j++){
			
			if(i < lin || (i==lin && j<col)){
								
				int cor_seg;
				
				if((imagem[i][j])->r > 100){
					cor_seg = 255;				
				}
				else cor_seg = 0;
				
				(imagem[i][j])->r =  cor_seg;
				(imagem[i][j])->g =  cor_seg;
				(imagem[i][j])->b =  cor_seg;
			}
		}
	}
	
	//Gera imagem apartir da Struct	
	//Dados
	fseek(arqout,(*cabecalho).deslocamento,SEEK_SET);
	for(i=0;i<lin;i++){
		for(j=0;j<(*cabecalho).largura_img;j++){
			
			if(i < lin || (i==lin && j<col)){
				fwrite(&(imagem[i][j])->b,sizeof(blue),1,arqout);
				fwrite(&(imagem[i][j])->g,sizeof(green),1,arqout);
				fwrite(&(imagem[i][j])->r,sizeof(red),1,arqout);
			}

		}
	}	
	//Formato do arquivo
	//<nome do arquivo> <linhainicial> <linhafinal> <número de nós encontrados> <L1> <C1> <L2> <C2> ...<Ln><Cn>
	escrevesaida(txtsaida, arquivoentrada);
	

	fclose(arqin);
	fclose(arqout);
	
	return 0;
}


