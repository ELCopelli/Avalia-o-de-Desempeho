#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <string.h>

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
	int     tam_imagem;
	int		resolucao_hori;
	int		resolucao_ver;
	int		num_cores_usu_img;
	int		num_cores_imp;
} Cabecalho;

typedef struct inf_nos{
    int min_vertical;
    int max_vertical;
    int min_horizontal;
    int max_horizontal;
    int centro_vertical;
    int centro_horizontal;
} Inf_nos;

void escrevesaida(FILE *Arquivo, char *linha,int limite_sup, int limite_inf,int qtde_nos,int qtde_nos_finais,Inf_nos *informacao_nos){
	fprintf(Arquivo,"%s \t%d\t%d\t%d",linha,limite_sup,limite_inf,qtde_nos_finais);
	int e;
	for(e=0;e<qtde_nos;e++)
        {
            if(informacao_nos[e].centro_vertical > 0){
				fprintf(Arquivo," %d  %d ",informacao_nos[e].centro_vertical,informacao_nos[e].centro_horizontal);
			}
        }
	fprintf(Arquivo,"\n");
}

int main(){

	char *direntrada = malloc(100);
	char *arquivoentrada = malloc(100);
	FILE *arqin,*arqout,*txtsaida;
	DIR *dir;
	struct dirent *lsdir;
	
	int num_imagem = 0;	
	int i,j,p,q;
	int lim=i-1;
	int soma_cores_b=0;
	double aux_mb = 0;
	double aux_o2b = 0;
	double Wb = 0;
	double Mb = 0;
	double O2b = 0;
	
	double media;

	int soma_cores_f=0;
	double aux_mf = 0;
	double aux_o2f = 0;
	double Wf = 0;
	double Mf = 0;
	double O2f = 0;
	
	int qtde_nos = 0;
	
	int histograma[256];
	int tot_pixels=0;	
	
	/* ------------------------------*/
	/* Solicita o arquivo de entrada */
	/* ----------------------------- */

	//arquivo de entrada
	printf("Digite o nome do diretorio: \n");
	scanf("%s",direntrada);
	dir = opendir(direntrada);
	printf("Processando as imagens no diretorio %s \n",direntrada);
	//arquivo de saída
	txtsaida = fopen("saida.txt","w");
	if(txtsaida == NULL){
		printf("Erro na criação do arquivo.");
		exit;
	}

	while ( ( lsdir = readdir(dir) ) != NULL ) {
		
		num_imagem++;
		
		if ( !strcmp(lsdir->d_name, ".") || !strcmp(lsdir->d_name, "..") ){
			//tratamento para quando o sistema acha "." e ".."
		}
		else{
			
			strcpy(arquivoentrada, direntrada);
			strcat(arquivoentrada,"/");
			strcat(arquivoentrada,lsdir->d_name);

			arqin = fopen(arquivoentrada,"rb");

			//valida se o arquivo de entrada existe
			if(arqin==NULL)  printf("Erro na leitura do arquivo.");

			//arquivo de saida
			arqout = fopen("novo","wb");

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

			//printf("%d - %d - %d \n\n",(*cabecalho).altura_img,(*cabecalho).largura_img,(*cabecalho).tam_arquivo);

			/* --------------------------- */
			/* ------ Copia cabeçalho ---- */
			/* --------------------------- */

			fseek(arqin,0,SEEK_SET);			
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
			histograma[256];
			tot_pixels=0;
			for (i=0;i<256;i++)
			{
				histograma[i]=0;
			}

			for(i=0;i<lin;i++){
				for(j=0;j<(*cabecalho).largura_img;j++){

					if(i < lin || (i==lin && j<col)){
						media = (((imagem[i][j])->r * 0.3)+ ((imagem[i][j])->g * 0.59)+((imagem[i][j])->b) * 0.11);

						(imagem[i][j])->r = (int) media;
						(imagem[i][j])->g = (int) media;
						(imagem[i][j])->b = (int) media;
						histograma[(int)media]++;
						tot_pixels++;
					}
				}
			}

			/* ---------------------------------------------- */
			/*  Cálula o valor limear para converter para PB  */
			/* ---------------------------------------------- */
			//Algoritmo de Otsu		
			int ind_WCV = 0;
			float min_WCV = 10000000;
			for(i=0;i<256;i++)
			{
				lim=i-1;
				soma_cores_b=0;
				aux_mb = 0;
				aux_o2b = 0;
				Wb = 0;
				Mb = 0;
				O2b = 0;

				soma_cores_f=0;
				aux_mf = 0;
				aux_o2f = 0;
				Wf = 0;
				Mf = 0;
				O2f = 0;

				//Background
				while(lim>=0)
				{
					soma_cores_b= soma_cores_b + histograma[lim];
					aux_mb = aux_mb + (lim * histograma[lim]);
					lim--;
				}
				if(soma_cores_b > 0)
				{
					Wb = (double)soma_cores_b / (double)tot_pixels;
					Mb = (double)aux_mb / (double)soma_cores_b;

					lim = i-1;
					while(lim>=0)
					{
						aux_o2b = aux_o2b + (((lim-Mb)*(lim-Mb)) * histograma[lim]);
						lim--;
					}

					O2b =  aux_o2b / (double)soma_cores_b;
				}

				//Foreground
				lim=i;
				while(lim<256)
				{
					soma_cores_f = (double)soma_cores_f + (double)histograma[lim];
					aux_mf = (double)aux_mf + (lim * histograma[lim]);
					lim++;
				}
				if(soma_cores_f > 0)
				{
					Wf = (double)soma_cores_f / (double)tot_pixels;
					Mf = (double)aux_mf / (double)soma_cores_f;
					lim = i;

					while(lim<256)
					{
						aux_o2f = aux_o2f + (((lim-Mf)*(lim-Mf)) * histograma[lim]);
						lim++;
					}

					O2f =  aux_o2f / (double)soma_cores_f;
				}
				double WCV = (Wb*O2b) + (Wf*O2f);
				if (WCV < min_WCV)
				{
					min_WCV = WCV;
					ind_WCV = i;
				}


			}
			printf("Linear-> %d - %d\n",ind_WCV,num_imagem);

			/* ----------------------------------------- */
			/*  Segmentação da Imagem em Preto e Branco  */
			/* ----------------------------------------- */

			for(i=0;i<lin;i++){
				for(j=0;j<(*cabecalho).largura_img;j++){

					if(i < lin || (i==lin && j<col)){

						int cor_seg;

						if((imagem[i][j])->r > ind_WCV){
							cor_seg = 255;
						}
						else cor_seg = 0;

						(imagem[i][j])->r =  cor_seg;
						(imagem[i][j])->g =  cor_seg;
						(imagem[i][j])->b =  cor_seg;
					}
				}
			}

			/* ------------------------------- */
			/* 	 Busca Limite superior     */
			/* ------------------------------- */

			int limite_sup = 0;
			//Isso faz buscando a primeira linha que possui somente brancos
			for(i=0;i<(*cabecalho).altura_img;i++){
				int qtde_px_preto = 0;
				for(j=0;j<(*cabecalho).largura_img;j++){
					if((imagem[(*cabecalho).altura_img-1-i][j])->r == 0 ){
					qtde_px_preto++;
					}
				}
				if(qtde_px_preto ==0 ){
					limite_sup = i;
					break;
				}
			}

			/* //Isso faz pela media dos pontos de inicio superior da madeira, mas distorce demais a quantidade de nós
			int limite[lin];
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
			}*/


			/* ------------------------------- */
			/* 	 Busca Limite inferior     */
			/* ------------------------------- */

			int limite_inf = 0;
			//Isso faz buscando a primeira linha que possui somente brancos de baixo para cima
			for(i=(*cabecalho).altura_img-1;i>=0;i--){
				int qtde_px_preto = 0;
				for(j=0;j<(*cabecalho).largura_img;j++){
					if((imagem[(*cabecalho).altura_img-1-i][j])->r == 0 ){
					qtde_px_preto++;
					}
				}
				if(qtde_px_preto ==0 ){
					limite_inf = i;
					break;
				}
			}

			/*//Isso faz pela media dos pontos de inicio inferir da madeira, mas distorce demais a quantidade de nós
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
			limite_inf = lin - limite_inf;*/

		//	printf("*******************************\n");
		//	printf("LIMITE_SUPERIOS->%d \n",limite_sup);
		//	printf("LIMITE_INFERIOR->%d \n",limite_inf);
		//	printf("*******************************\n");


			/* ----------------------------------------- */
			/*  Monta matriz com informalções de nos     */
			/* ----------------------------------------- */

			qtde_nos         = 0;
			int matriz_nos[(*cabecalho).altura_img][(*cabecalho).largura_img];

			Inf_nos *informacoes_nos = malloc(sizeof(Inf_nos) * 1);

			for(i=0;i<(*cabecalho).altura_img;i++){
				for(j=0;j<(*cabecalho).largura_img;j++){
					matriz_nos[i][j]=0;
				}
			}

			for(i=0;i<(*cabecalho).altura_img;i++){
				for(j=0;j<(*cabecalho).largura_img;j++){
						int no_prox=0;
					if((imagem[(*cabecalho).altura_img-1-i][j])->r == 0 && i > limite_sup && i < limite_inf  ){
						//printf("preto-> %d - %d\n",i,j);
						//Verifica se existe algum ponto ao redor marcado como nó e assume o valor daquele nó
						if(i-1 >= 0){
							if(matriz_nos[i-1][j]>0 && no_prox ==0  ){
								no_prox = matriz_nos[i-1][j];
							}
							if(j-1>=0 ){
								if(matriz_nos[i-1][j-1]>0 && no_prox ==0 ){
									no_prox = matriz_nos[i-1][j-1];
								}
							}
							if(j+1<(*cabecalho).largura_img){
								if(matriz_nos[i-1][j+1]>0 && no_prox ==0 ){
									no_prox = matriz_nos[i-1][j+1];
								}
							}
						}
						if(i+1 < (*cabecalho).altura_img){
							if(matriz_nos[i+1][j]>0 && no_prox ==0 ){
								no_prox = matriz_nos[i+1][j];
							}
							if(j-1>=0){
								if(matriz_nos[i+1][j-1]>0 && no_prox ==0 ){
									no_prox = matriz_nos[i+1][j-1];
								}
							}
							if(j+1<(*cabecalho).largura_img){
								if(matriz_nos[i+1][j+1]>0 && no_prox ==0 ){
									no_prox = matriz_nos[i+1][j+1];
								}
							}

						}
						if(j+1 < (*cabecalho).largura_img){
							if(matriz_nos[i][j+1]>0 && no_prox ==0 ){
								no_prox = matriz_nos[i][j+1];
							}
						}
						if(j-1 >= 0){
							if(matriz_nos[i][j-1]>0 && no_prox ==0 ){
								no_prox = matriz_nos[i][j-1];
							}
						}
						//se não encontrou um nó proximo considera como um novo nó
						if(no_prox == 0){
							qtde_nos++;
							no_prox = qtde_nos;

						}

						matriz_nos[i][j]=no_prox;


					}
				}
			}


			/* ----------------------------------------- */
			/*     Refina os dados de nós encontrados    */
			/* ----------------------------------------- */
			//se tem um nó ao ao redor, com númeração menor, assume a menor numeração - para casos onde não é possivel identificar o nó proximo pelo algoritmo principal
			for(i=(*cabecalho).altura_img-1;i>=0;i--){
				for(j=(*cabecalho).largura_img-1;j>=0;j--){
					if(matriz_nos[i][j] > 0){
						int menor_no = matriz_nos[i][j];
						if(i-1 >= 0){
							if(matriz_nos[i-1][j] != 0 && matriz_nos[i-1][j] < menor_no ){
								menor_no = matriz_nos[i-1][j];
							}
							if(j-1>=0 ){
								if(matriz_nos[i-1][j-1] != 0 && matriz_nos[i-1][j-1] < menor_no ){
									menor_no = matriz_nos[i-1][j-1];
								}
							}
							if(j+1<(*cabecalho).largura_img){
								if(matriz_nos[i-1][j+1]!= 0 && matriz_nos[i-1][j+1] < menor_no ){
									menor_no = matriz_nos[i-1][j+1];
								}
							}
						}
						if(i+1 < (*cabecalho).altura_img){
							if(matriz_nos[i+1][j]!=0 && matriz_nos[i+1][j] < menor_no ){
								menor_no = matriz_nos[i+1][j];
							}
							if(j-1>=0){
								if(matriz_nos[i+1][j-1]!=0 && matriz_nos[i+1][j-1] < menor_no  ){
									menor_no = matriz_nos[i+1][j-1];
								}
							}
							if(j+1<(*cabecalho).largura_img){
								if(matriz_nos[i+1][j+1]!=0 && matriz_nos[i+1][j+1] < menor_no){
									menor_no = matriz_nos[i+1][j+1];
								}
							}

						}
						if(j+1 < (*cabecalho).largura_img){
							if(matriz_nos[i][j+1]!=0 && matriz_nos[i][j+1] < menor_no ){
								menor_no = matriz_nos[i][j+1];
							}
						}
						if(j-1 >= 0){
							if(matriz_nos[i][j-1]!=0 && matriz_nos[i][j-1] < menor_no){
								menor_no = matriz_nos[i][j-1];
							}
						}

						matriz_nos[i][j] = menor_no;
					}
				}
			}


			/* ----------------------------------------- */
			/*  Calcula informações e o centro dos nós   */
			/* ----------------------------------------- */
			informacoes_nos = realloc(informacoes_nos, qtde_nos * sizeof(Inf_nos));
			for(i=0;i<qtde_nos;i++){	
				informacoes_nos[i].min_vertical   = 0;
				informacoes_nos[i].max_vertical   = 0;
				informacoes_nos[i].min_horizontal = 0;
				informacoes_nos[i].max_horizontal = 0;
				informacoes_nos[i].centro_vertical   = 0;
				informacoes_nos[i].centro_horizontal = 0;
			}

			//Controla as informações do de posicionamento dos nos
			for(i=0;i<(*cabecalho).altura_img;i++){
				for(j=0;j<(*cabecalho).largura_img;j++){

					if(matriz_nos[i][j] > 0){
							if(informacoes_nos[matriz_nos[i][j]-1].min_vertical == 0)
							{
								informacoes_nos[matriz_nos[i][j]-1].min_vertical = i;
								informacoes_nos[matriz_nos[i][j]-1].max_vertical = i;
								informacoes_nos[matriz_nos[i][j]-1].min_horizontal = j;
								informacoes_nos[matriz_nos[i][j]-1].max_horizontal = j;
							}
							//Controle vertical
							if(i < informacoes_nos[matriz_nos[i][j]-1].min_vertical)
							{
								informacoes_nos[matriz_nos[i][j]-1].min_vertical = i;
							}
							if(i > informacoes_nos[matriz_nos[i][j]-1].max_vertical)
							{
								informacoes_nos[matriz_nos[i][j]-1].max_vertical = i;
							}
							//Controle horizontal
							if(j < informacoes_nos[matriz_nos[i][j]-1].min_horizontal)
							{
								informacoes_nos[matriz_nos[i][j]-1].min_horizontal = j;
							}
							if(j > informacoes_nos[matriz_nos[i][j]-1].max_horizontal)
							{
								informacoes_nos[matriz_nos[i][j]-1].max_horizontal = j;
							}
					}
				}
			}

			//Calcula centro dos nós
			for(i=0;i<qtde_nos;i++)
			{
			   informacoes_nos[i].centro_vertical   = (informacoes_nos[i].min_vertical   + informacoes_nos[i].max_vertical) / 2;
			   informacoes_nos[i].centro_horizontal = (informacoes_nos[i].min_horizontal + informacoes_nos[i].max_horizontal) / 2;
			   //printf("pos nó --> %d - %d \n",informacoes_nos[i].centro_vertical,informacoes_nos[i].centro_horizontal);
			}

			//Conta quantos nós existem após o refinamento
			int qtde_nos_finais =0;
			for(i=0;i<qtde_nos;i++)
			{
				if(informacoes_nos[i].centro_vertical>0)
				{
					qtde_nos_finais++;
				}
			}

			/* AUXILIAR  DEPOIS DA DE TIRAR */
			//Gera imagem apartir da Struct	*/
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

			escrevesaida(txtsaida, arquivoentrada,limite_sup,limite_inf,qtde_nos,qtde_nos_finais,informacoes_nos);

			//Liberação de memória 
			free(informacoes_nos);
			free(cabecalho);
			
			//Isso é para desalocar a memoria da imagem (**Esse é o pulo do gato do estouro de memória que existia**)
			for(i=0;i<(*cabecalho).altura_img;i++){
				for(j=0;j<(*cabecalho).largura_img;j++){
					free(imagem[i][j]);
				}
			}
           
			fclose(arqin);
			fclose(arqout);
			
		}
	}
	closedir(dir);
	fclose(txtsaida);
	printf("Processamento dos arquivos finalizado! \n");
	return 0;
}

/*
Coisas para ver:
1 - Existe um tamanho mínimo para ser considerado nó?
2 - Forma de identificação dos limites verticais, qual a melhor forma? (para funcionar nós grudados nas bordas como a figura 18)


/home/ederson/Documentos/AvaliacaodeDesempenho/imagens_teste
*/
