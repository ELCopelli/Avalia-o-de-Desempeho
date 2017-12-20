#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <string.h>
#include<omp.h>
#include <gperftools/profiler.h>

#define NTHREADS 4

/*********************************************************************************************
 **							Sistema de inspeção Automática de Madeira
 **		Alunos:
 **		-Éderson Luis Copelli 	(elcopelli@ucs.br)
 **		-Gustavo Pistore		(gpistore1@ucs.br)
 *********************************************************************************************/

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

	Pixel ** monta_imagem_lida_struct(FILE *arqin, Cabecalho *cabecalho)
	{


		Pixel **imagem;

		unsigned char red,green,blue;
		int i,j;		

		imagem = (Pixel**) calloc((*cabecalho).altura_img , sizeof(Pixel*));

		for (i = 0; i < (*cabecalho).altura_img; i++) {
			imagem[i] = (Pixel*) calloc((*cabecalho).largura_img , sizeof(Pixel));
		}

		int lin =0,col=0;
		int id, nthr;

			while(fread(&red,sizeof(blue),1,arqin)!=0)
			{
				fread(&green,sizeof(green),1,arqin);
				fread(&blue,sizeof(red),1,arqin);

				(imagem[lin][col]).r = red;
				(imagem[lin][col]).g = green;
				(imagem[lin][col]).b = blue;

				col++;
				if (col >= (*cabecalho).largura_img)
				{
					lin++;
					col=0;
				}
			}

		return imagem;	

	}

	void transforma_tons_de_cinza (int *histograma,int *tot_pixels,Cabecalho *cabecalho,Pixel **imagem)
	{
		
		int i,j;
		double media;
		(*tot_pixels)=0;				
		for (i=0;i<256;i++)
		{
			histograma[i]=0;
		}
		
		(*tot_pixels) = (*cabecalho).altura_img * (*cabecalho).largura_img;
		
		for(i=0;i<(*cabecalho).altura_img;i++){
			for(j=0;j<(*cabecalho).largura_img;j++){
					
					media = (((imagem[i][j]).r * 0.3)+ ((imagem[i][j]).g * 0.59)+((imagem[i][j]).b) * 0.11);					
					(imagem[i][j]).r = (int) media;
					(imagem[i][j]).g = (int) media;
					(imagem[i][j]).b = (int) media;
					histograma[(int)media]++;
				
			}
		}
	}


	int algoritmo_otsu(int *histograma, int tot_pixels)
	{
		int ind_WCV = 0,i;
		float min_WCV = 10000000;
		int soma_cores_f=0;
		double aux_mf = 0;
		double aux_o2f = 0;
		double Wf = 0;
		double Mf = 0;
		double O2f = 0;
		
		int soma_cores_b=0;
		double aux_mb = 0;
		double aux_o2b = 0;
		double Wb = 0;
		double Mb = 0;
		double O2b = 0;
		
		int lim=0;
		
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
		
		return ind_WCV;
		
	}

	void segmentacao_preto_e_branco (int ind_WCV, Cabecalho *cabecalho,Pixel **imagem)
	{
		int i,j;
		for(i=0;i<(*cabecalho).altura_img;i++){
			for(j=0;j<(*cabecalho).largura_img;j++){

				int cor_seg;
				
				if((imagem[i][j]).r > ind_WCV){
					cor_seg = 255;
				}
				else cor_seg = 0;

					(imagem[i][j]).r =  cor_seg;
					(imagem[i][j]).g =  cor_seg;
					(imagem[i][j]).b =  cor_seg;
					
			}
		}
	}

	int calcula_limite_superior(Cabecalho *cabecalho,Pixel **imagem)
	{
		int limite_sup = 0;
		int i,j;
		//Isso faz buscando a primeira linha que possui somente brancos
		for(i=0;i<(*cabecalho).altura_img;i++){
			int qtde_px_preto = 0;
			for(j=0;j<(*cabecalho).largura_img;j++){
				if((imagem[(*cabecalho).altura_img-1-i][j]).r == 0 ){
					qtde_px_preto++;
					}
			}
			if(qtde_px_preto ==0 ){
				limite_sup = i;
				break;
			}
		}
		
		//Tratamenro para considerar informações de nos de limites
		for(i=limite_sup;i>0;i--)
		{
			int qtde_pontos = 0;
			for(j=0;j<(*cabecalho).largura_img;j++){
				if((imagem[(*cabecalho).altura_img-1-i][j]).r == 0 && (imagem[(*cabecalho).altura_img-1-i-1][j]).r == 255) {
					qtde_pontos++;
					}
			}
			
		    if(qtde_pontos > 6)
			{
					limite_sup = i + 1;
					break;
			}
				   
		}
		
		
		return limite_sup;
	}

	int calcula_limite_inferior(Cabecalho *cabecalho,Pixel **imagem)
	{
		int limite_inf;
		int i,j;
		//Isso faz buscando a primeira linha que possui somente brancos de baixo para cima
		for(i=(*cabecalho).altura_img-1;i>=0;i--){
			int qtde_px_preto = 0;
			for(j=0;j<(*cabecalho).largura_img;j++){
				if((imagem[(*cabecalho).altura_img-1-i][j]).r == 0 ){
					qtde_px_preto++;
				}
			}
			if(qtde_px_preto ==0 ){
				limite_inf = i;
				break;
			}
		}
		//Tratamenro para considerar informações de nos de limites
		for(i=limite_inf;i<(*cabecalho).altura_img-1;i++)
		{
			int qtde_pontos = 0;
			for(j=0;j<(*cabecalho).largura_img;j++){
				if((imagem[(*cabecalho).altura_img-1-i][j]).r == 0 && (imagem[(*cabecalho).altura_img-1-i-1][j]).r == 255) {
					qtde_pontos++;
					}
			}
			
		    if(qtde_pontos > 6)
			{
					limite_inf = i - 1;
					break;
			}
				   
		}
		
		return limite_inf;
		
	}


	void monta_matriz_nos (Cabecalho *cabecalho,Pixel **imagem, int **matriz_nos, int *qtde_nos, int limite_sup,int limite_inf)
	{
		
		int i,j;
		
		for(i=0;i<(*cabecalho).altura_img;i++){
			for(j=0;j<(*cabecalho).largura_img;j++){
				matriz_nos[i][j]=0;
			}
		}

		for(i=0;i<(*cabecalho).altura_img;i++){
			for(j=0;j<(*cabecalho).largura_img;j++){
				int no_prox=0;
				if((imagem[(*cabecalho).altura_img-1-i][j]).r == 0 && i > limite_sup && i < limite_inf  ){
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
						(*qtde_nos)++;
						no_prox = (*qtde_nos);
					}
		
					matriz_nos[i][j]=no_prox;

				}
			}
		}		
	}

	void refinamento_dados_nos(Cabecalho *cabecalho,Pixel **imagem, int **matriz_nos)
	{
		int i,j;
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
	}

	void calcula_informacoes_nos(Cabecalho *cabecalho, int **matriz_nos, Inf_nos *informacoes_nos, int *qtde_nos_finais,int qtde_nos)
	{
		
		int i, j;
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
					}//Inf_nos *informacoes_nos = malloc(sizeof(Inf_nos) * 1);
			
				}
			}
		
			//printf("qtde_nos-> %d\n",qtde_nos);

			//Calcula centro dos nós
			for(i=0;i<qtde_nos;i++)
			{
			   
				//desconsidera informações pequenas, com menos de x pixels
				int num_pixel_desc = 4;
				if(		( informacoes_nos[i].max_vertical - informacoes_nos[i].min_vertical) <= num_pixel_desc
				   ||	( informacoes_nos[i].max_horizontal - informacoes_nos[i].min_horizontal) <= num_pixel_desc)
				{
					informacoes_nos[i].centro_vertical   = 0;
			   		informacoes_nos[i].centro_horizontal = 0;
				}
				else
				{
					informacoes_nos[i].centro_vertical   = (informacoes_nos[i].min_vertical   + informacoes_nos[i].max_vertical) / 2;
			   		informacoes_nos[i].centro_horizontal = (informacoes_nos[i].min_horizontal + informacoes_nos[i].max_horizontal) / 2;			   
				}
								
			}

			
			//Verifica se não tem nó proximo que possa ser considerado  o mesmo
			for(i=0;i<qtde_nos;i++)
			{
				for(j=0;j<qtde_nos;j++)
				{
					if(informacoes_nos[i].centro_vertical > 0 && informacoes_nos[j].centro_vertical > 0 && i != j )
					{
						int proximidade_mesmo_no = 20;
						if( (abs(informacoes_nos[i].centro_vertical -  informacoes_nos[j].centro_vertical)  +  abs(informacoes_nos[i].centro_horizontal -  informacoes_nos[j].centro_horizontal)) < proximidade_mesmo_no)
						{
							informacoes_nos[j].centro_vertical   = 0;
							informacoes_nos[j].centro_horizontal = 0;
						}
					}
				}
			}
		
			//Conta quantos nós existem após o refinamento
			*qtde_nos_finais =0;
			for(i=0;i<qtde_nos;i++)
			{
				if(informacoes_nos[i].centro_vertical>0)
				{
					(*qtde_nos_finais)++;
				}
			}
	}

int main(){
    //ProfilerStart("file.log");
	char *direntrada = malloc(100);
	FILE *arqout,*txtsaida;
	DIR *dir;
	struct dirent *lsdir;
	
	int num_imagem = 0;	
	int a;
	
	double media;

	int qtde_nos = 0;
	
	int histograma[256];
	int tot_pixels=0;	
	
	/* ------------------------------*/
	/* Solicita o arquivo de entrada */
	/* ----------------------------- */

	//arquivo de entrada
	printf("Digite o nome do diretorio: \n");
	//scanf("%s",direntrada);
	direntrada = "validos"; // CAMINHO DA IMAGEM FIXO PARA TESTES
	dir = opendir(direntrada);
	printf("Processando as imagens no diretorio %s \n",direntrada);
	//arquivo de saída
	txtsaida = fopen("saida.txt","w");
	if(txtsaida == NULL){
		printf("Erro na criação do arquivo.");
		exit;
	}
	
	//atribui os arquivos para o vetor de arquivos
	int num_arquivos=0;
	struct dirent *arquivos = malloc(sizeof(struct dirent) * 1);
	
    
	while ( ( lsdir = readdir(dir) ) != NULL ) {
		if ( !strcmp(lsdir->d_name, ".") || !strcmp(lsdir->d_name, "..") ){
		}
		else
		{
			num_arquivos++;
			arquivos = realloc(arquivos, num_arquivos * sizeof(struct dirent));
			arquivos[num_arquivos-1]=*lsdir;
			//printf("\nalocando-> %d %s",num_arquivos-1,arquivos[num_arquivos-1].d_name);
		}
	}
	
	char caminhos[1000][1000];

	int c;		

	//caminhos = (char**) calloc(num_arquivos , sizeof(char));

	for (c = 0; c < num_arquivos; c++) {
		//caminhos[c] = (char*) calloc(100 , sizeof(char));
		char caminho[1000];
		strcpy(caminho, direntrada);
		strcat(caminho,"/");
		strcat(caminho,arquivos[c].d_name);
		strcat(caminho,"\0");
		strcpy(caminhos[c],caminho);
		//printf("caminho %d -> %s\n ",c,caminhos[c]);
	}
	
	
	
	
	char arquivoentrada[1000];
	strcpy(arquivoentrada,caminhos[a]);	
	//while ( ( lsdir = readdir(dir) ) != NULL ) {
	FILE *arqin;
	omp_set_num_threads(NTHREADS);
	#pragma omp parallel private ( a ,histograma,qtde_nos,tot_pixels,arquivoentrada,arqin) 
	{
		int id;
		int nthr;
		
		id = omp_get_thread_num();
		nthr = omp_get_num_threads();
		
	    
		for(a=id;a<num_arquivos;a+=nthr){	
			//printf("\n\nThread ***** %d ****",id);
			int i = 0;
			
			
				//printf("\n  Processando Imagem-> %d - %s",a,caminhos[a]);
				arqin = fopen(caminhos[a],"rb");
				//printf("\n  Abriu imagem-> %d - %s",a,caminhos[a]);
				//valida se o arquivo de entrada existe
				if(arqin==NULL)  printf("Erro na leitura do arquivo.");

				//Busca dados do cabeçalho da imagem e grama em estrutura
				Cabecalho *cabecalho = malloc(sizeof(Cabecalho));

				fseek(arqin,14,SEEK_SET);
				fread(&(*cabecalho).tam_cabecalho	,4,1,arqin);
				fseek(arqin,18,SEEK_SET);
				fread(&(*cabecalho).largura_img		,4,1,arqin);
				fseek(arqin,22,SEEK_SET);
				fread(&(*cabecalho).altura_img		,4,1,arqin);
				fseek(arqin,26,SEEK_SET);

				fseek(arqin,50,SEEK_SET);
				fread(&(*cabecalho).num_cores_imp	,4,1,arqin);

				/* ------------------------------------------ */
				/*  Monta dados da imagem lida em uma Struct  */
				/* ------------------------------------------ */

				int lin =0,col=0;

				Pixel **imagem;
				imagem  = monta_imagem_lida_struct (arqin,&(*cabecalho));

				lin = (*cabecalho).altura_img;
				col = (*cabecalho).largura_img;			

				/* ------------------------------- */
				/*  Transforma para tons de cinza  */
				/* ------------------------------- */

				transforma_tons_de_cinza ( histograma, &tot_pixels,&(*cabecalho), &(*imagem));		

				/* ---------------------------------------------- */
				/*  Cálula o valor limear para converter para PB  */
				/* ---------------------------------------------- */
				//Algoritmo de Otsu		
				int ind_WCV = 0;

				ind_WCV  = algoritmo_otsu(histograma,tot_pixels);			
				//printf("Limear-> %d - %d\n",ind_WCV,num_imagem);

				/* ----------------------------------------- */
				/*  Segmentação da Imagem em Preto e Branco  */
				/* ----------------------------------------- */

				segmentacao_preto_e_branco (ind_WCV, &(*cabecalho), &(*imagem));					

				/* ------------------------------- */
				/* 	 Busca Limite superior     */
				/* -------------------------------*/

				int limite_sup = calcula_limite_superior ( &(*cabecalho), &(*imagem));

				/* ------------------------------- */
				/* 	 Busca Limite inferior     */
				/* ------------------------------- */

				int limite_inf = calcula_limite_inferior ( &(*cabecalho), &(*imagem));			

				/* ----------------------------------------- */
				/*  Monta matriz com informalções de nos     */
				/* ----------------------------------------- */

				qtde_nos         = 0;

				int **matriz_nos;

				matriz_nos = (int**) calloc((*cabecalho).altura_img , sizeof(int*));
	
				for (i = 0; i < (*cabecalho).altura_img; i++) {
					matriz_nos[i] = (int*) calloc((*cabecalho).largura_img , sizeof(int));
				}

				monta_matriz_nos (&(*cabecalho), &(*imagem), &(*matriz_nos),&qtde_nos,limite_sup,limite_inf);
                
				/* ----------------------------------------- */
				/*     Refina os dados de nós encontrados    */
				/* ----------------------------------------- */
				//se tem um nó ao ao redor, com númeração menor, assume a menor numeração - para casos onde não é possivel identificar o nó proximo pelo algoritmo principal

				refinamento_dados_nos(&(*cabecalho), &(*imagem), &(*matriz_nos));
				refinamento_dados_nos(&(*cabecalho), &(*imagem), &(*matriz_nos));

				/* ----------------------------------------- */
				/*  Calcula informações e o centro dos nós   */
				/* ----------------------------------------- */

				Inf_nos *informacoes_nos = malloc(sizeof(Inf_nos) * 1);			
				informacoes_nos = realloc(informacoes_nos, qtde_nos * sizeof(Inf_nos));
				int qtde_nos_finais = 0;

				//printf("qtde_nos-> %d qtde_nos_finais->%d\n",qtde_nos,qtde_nos_finais);
				calcula_informacoes_nos(&(*cabecalho),&(*matriz_nos),informacoes_nos,&qtde_nos_finais,qtde_nos);
				//printf("qtde_nos-> %d qtde_nos_finais->%d\n",qtde_nos,qtde_nos_finais);
				/* ---------------------------------------------- */
				/*  Escreve dados na imagem no arquivo de saída   */
				/* ---------------------------------------------- */
				
				#pragma omp critical
				{
					escrevesaida(txtsaida, caminhos[a],limite_sup,limite_inf,qtde_nos,qtde_nos_finais,informacoes_nos);
				}
			
				//Liberação de memória 
				free(informacoes_nos);
				free(cabecalho);
				free(*imagem);	
				free(imagem);	
				free(*matriz_nos);
				free(matriz_nos);
				fclose(arqin);


		}
	
	}
	closedir(dir);
	fclose(txtsaida);
	printf("\nProcessamento dos arquivos finalizado! \n");
	//ProfilerStop();
	return 0;
}

