#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 100 //número máximo de produtos cadastrados

typedef struct{
	char nome[50];
	int index;
}key_product;

typedef struct{
	int qtd_estq, qtd_vend;
	int deleted;		
	float v_compra, v_venda;
}info_product;

void openFiles(FILE *f1, FILE *f2); //cria os arquivos (se eles não existirem)
void sellProduct(key_product k, info_product info, FILE *f1, FILE *f2); //vende um produto
void registerProduct(key_product k, info_product info, FILE *f1, FILE *f2); //cadastra um produto
void removeProduct(key_product k, info_product info, FILE *f1, FILE *f2); //remove um produto
void consultStock(key_product k, info_product info, FILE *f1, FILE *f2); //consulta o estoque
void consultProfit(key_product k, info_product info, FILE *f1, FILE *f2); //consulta o lucro
void replaceStock(key_product k, info_product info, FILE *f1, FILE *f2); //repõe o estoque
void endDay(key_product k, info_product info, FILE *f1, FILE *f2); //encerra o dia
void sortFile(FILE *f1); //coloca os nomes dos produtos em um vetor e os ordena
void quicksort(key_product *aux, int n); //método de ordenação usado para ordenar os nomes no arquivo
void sort(int esq, int dir, key_product *aux); //auxiliar de quicksort
void partition(int esq, int dir, int *i, int *j, key_product *aux); //auxiliar de quicksort
int verifyDeleted(key_product k, info_product info, FILE *f1, FILE *f2); //verifica se há arquivos deletados
int binarySearch(char *aux, key_product k, FILE *f1); //realiza busca binária
int fileSize(FILE *f1); //verifica a quantidade de produtos cadastrados

int main(){
	int op; 
	key_product k;
	info_product info;
	FILE *f1, *f2;
	
	openFiles(f1, f2);

	printf("---------MENU---------\n");
	printf("1: Vender Produto\n");
	printf("2: Cadastrar Produto\n");
	printf("3: Remover Produto\n");
	printf("4: Consultar Estoque\n");
	printf("5: Consultar Lucro\n");
	printf("6: Repor Estoque\n");
	printf("0: Sair\n");
	
	do{
		printf("\nOpcao: ");
		scanf("%d", &op);
		
		switch(op){
			case 1:
				{
					sellProduct(k, info, f1, f2);
					break;
				}
				
			case 2:
				{
					registerProduct(k, info, f1, f2);
					break;
				}
				
			case 3:
				{
					removeProduct(k, info, f1, f2);
					break;
				}
				
			case 4:
				{
					consultStock(k, info, f1, f2);
					break;
				}
				
			case 5:
				{
					consultProfit(k, info, f1, f2);
					break;
				}
				
			case 6:
				{
					replaceStock(k, info, f1, f2);
					break;
				}
				
			case 0:
				{
					endDay(k, info, f1, f2);
					break;
				}
				
			default: printf("ERRO: Opcao Invalida\n");
		}
	}while(op!=0);
	
	return 0;
}

void openFiles(FILE *f1, FILE *f2){
	f1 = fopen("chaves.bin", "rb");
	f2 = fopen("infs.bin", "rb");
	
	if(f1 == NULL || f2 == NULL){
		f1 = fopen("chaves.bin", "w+b");
		f2 = fopen("infs.bin", "w+b");
	}
	
	fclose(f1);
	fclose(f2);
}

void sellProduct(key_product k, info_product info, FILE *f1, FILE *f2){
	char aux[50];
	int q, p = 1;
	
	f1 = fopen("chaves.bin", "rb");
	f2 = fopen("infs.bin", "r+b");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
	
	printf("Digite o nome do produto que deseja comprar: ");
	scanf(" %[^\n]%*c", &aux);
	
	while(fread(&k, sizeof(key_product), 1, f1)){
		if(strcmp(aux, k.nome) == 0){
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET); //coloca o ponteiro na posição dos dados do produto no arquivo, levando em consideração sizeof(info_product)
			p = 0; //o produto existe
			break;
		}
	}
	
	if(p){
		printf("'%s' nao corresponde a nenhum produto.\n", aux);
		fclose(f1);
		fclose(f2);
		return;
	}
	
	fread(&info, sizeof(info_product), 1, f2);  //lê os dados do produto no arquivo
	printf("Valor do produto: R$%.2f\n", info.v_venda);
	
	printf("Informe a quantidade desejada: ");
	scanf("%d", &q);
	
	if(q>info.qtd_estq){
		printf("Nao ha estoque disponivel para atender a venda\n");
		fclose(f1);
		fclose(f2);
		return;
	}
	
	info.qtd_estq = info.qtd_estq - q;
	info.qtd_vend = info.qtd_vend + q;
	
	printf("Valor total de compra: R$%.2f\n", info.v_venda * q);
	printf("Agradecemos a preferencia!\n");
	
	fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET);  //retorna o ponteiro para o começo da linha
	fwrite(&info, sizeof(info_product), 1, f2);  //atualiza os dados
	
	fclose(f1);
	fclose(f2);
}

void registerProduct(key_product k, info_product info, FILE *f1, FILE *f2){
	f1 = fopen("chaves.bin", "r+b");
	f2 = fopen("infs.bin", "a+b");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
	
	fflush(stdin); //limpa o buffer do teclado
	printf("Nome do produto: ");
	scanf(" %[^\n]%*c", &k.nome);
	
	printf("Valor de compra: ");
	scanf("%f", &info.v_compra);
	printf("Valor de venda: ");
	scanf("%f", &info.v_venda);
	
	printf("Quantidade em estoque: ");
	scanf("%d", &info.qtd_estq);
	printf("Quantidade vendida: ");
	scanf("%d", &info.qtd_vend);
	
	info.deleted = 0; //false
	
	fseek(f1, 0L, SEEK_END);
	
	if(ftell(f1) == 0){ //não há nenhum produto cadastrado (o ponteiro está na posição 0)
		k.index = 1;
		fwrite(&k, sizeof(key_product), 1, f1);
		fwrite(&info, sizeof(info_product), 1, f2); //escreve as informações do produto no arquivo
		
		fclose(f1);
		fclose(f2);
		return;
	}
	
	else if(verifyDeleted(k, info, f1, f2)){ //verifica se há algum produto deletado
		sortFile(f1); //ordena o arquivo
		fclose(f1);
		return;
	}
	
	else{ 
		k.index = fileSize(f1);
		fwrite(&k, sizeof(key_product), 1, f1); //escreve os dados no arquivo para depois ordená-los
		sortFile(f1); //ordena o arquivo
		
		fseek(f2, 0L, SEEK_END);
		fwrite(&info, sizeof(info_product), 1, f2); //escreve as informações do produto no arquivo
	}

	fclose(f1);
	fclose(f2);
}

void removeProduct(key_product k, info_product info, FILE *f1, FILE *f2){
	char aux[50];
	int p = 1, q;
	
	f1 = fopen("chaves.bin", "r+b");
	f2 = fopen("infs.bin", "r+b");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
		
	printf("Digite o nome do produto a ser removido: ");
	scanf(" %[^\n]%*c", &aux);
	
	while(fread(&k, sizeof(key_product), 1, f1)){
		if(strcmp(aux, k.nome) == 0){
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET); //coloca o ponteiro na posição dos dados do produto no arquivo, levando em consideração sizeof(info_product)
			q = k.index; //armazena o index do produto
			p = 0; //o produto existe
			break;
		}
	}
	
	if(p){
		printf("'%s' nao corresponde a nenhum produto.\n", aux);
		fclose(f1);
		fclose(f2);
		return;
	}	
	
	fread(&info, sizeof(info_product), 1, f2);
	k.nome[0] = '\0'; //apaga a chave
	info.deleted = 1; //true
	
	fseek(f1, -56, SEEK_CUR); //volta pro começo do registro (56 = sizeof(key_product))
	fwrite(&k, sizeof(key_product), 1, f1); //apaga o produto deletado
	sortFile(f1); //ordena o arquivo
	
	fseek(f2, (q-1)*sizeof(info_product), SEEK_SET);	
	fwrite(&info, sizeof(info_product), 1, f2); //informa que o produto foi deletado
	printf("'%s' foi deletado com sucesso!\n", aux);
	
	fclose(f1);
	fclose(f2);
}

void consultStock(key_product k, info_product info, FILE *f1, FILE *f2){
	float total = 0;
	
	f1 = fopen("chaves.bin", "rb");
	f2 = fopen("infs.bin", "rb");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
	
	while(fread(&k, sizeof(key_product), 1, f1)){
		if(k.nome[0] != '\0'){
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET);
			fread(&info, sizeof(info_product), 1, f2);
			printf("Nome do produto: %s\n", k.nome);
			printf("Quantidade em estoque: %d\n", info.qtd_estq);
			printf("Valor do produto no estoque: R$%.2f\n", info.v_venda * info.qtd_estq);
			printf("\n");
			total = total + (info.v_venda * info.qtd_estq);
		}
	}
	
	printf("Valor total do estoque: R$%.2f\n", total);
	
	fclose(f1);
	fclose(f2);
}

void consultProfit(key_product k, info_product info, FILE *f1, FILE *f2){
	f1 = fopen("chaves.bin", "rb");
	f2 = fopen("infs.bin", "rb");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
	
	while(fread(&k, sizeof(key_product), 1, f1)){
		if(k.nome[0] != '\0'){
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET);
			fread(&info, sizeof(info_product), 1, f2);
			printf("Nome do produto: %s\n", k.nome);
			printf("Dinheiro gerado de vendas: R$%.2f\n", info.qtd_vend * info.v_venda);
			printf("Lucro: R$%.2f\n", (info.v_venda - info.v_compra) * info.qtd_vend);
		}
	}
	
	fclose(f1);
	fclose(f2);
}

void replaceStock(key_product k, info_product info, FILE *f1, FILE *f2){
	f1 = fopen("chaves.bin", "rb");
	f2 = fopen("infs.bin", "r+b");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
	
	char aux[50];
	int q, p;
	
	printf("Informe o produto a ser reposto: ");
	scanf(" %[^\n]%*c", &aux);
	
	p = binarySearch(aux, k, f1); //procura o produto e, se existir, retorna seu index
	
	if(p)
		fseek(f2, (p-1) * sizeof(info_product), SEEK_SET);
	
	else{
		printf("'%s' nao corresponde a nenhum produto.\n", aux);
		fclose(f1);
		fclose(f2);
		return;
	}

	while(1){
		printf("Informe a quantidade a ser reposta: ");
		scanf("%d", &q);
		
		if(q<=0) //continuará no looping até informar um valor válido
			printf("Valor invalido\n");
			
		else break;
	}
	
	fread(&info, sizeof(info_product), 1, f2);
	info.qtd_estq = info.qtd_estq + q;
	
	printf("Novo estoque de %s: %d", aux, info.qtd_estq);
	
	fseek(f2, (p-1)*sizeof(info_product), SEEK_SET);
	fwrite(&info, sizeof(info_product), 1, f2);
	
	fclose(f1);
	fclose(f2);
}

void endDay(key_product k, info_product info, FILE *f1, FILE *f2){
	f1 = fopen("chaves.bin", "rb");
	f2 = fopen("infs.bin", "r+b");
	
	if(f1 == NULL || f2 == NULL){
		printf("ERRO! Os arquivos nao foram abertos.\n");
		return;
	}
	
	int total_v = 0; 
	float total_l = 0;
	
	while(fread(&k, sizeof(key_product), 1, f1)){
		if(k.nome[0] != '\0'){
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET);
			fread(&info, sizeof(info_product), 1, f2);
			printf("Nome do produto: %s\n", k.nome);
			printf("Valor do produto: %.2f\n", info.v_venda);
			printf("Quantidade vendida do produto: %d\n", info.qtd_vend);
			printf("Quantidade em estoque do produto: %d\n", info.qtd_estq);
			printf("Valor total em estoque: %.2f\n", info.v_venda * info.qtd_estq);
			printf("\n");
			
			total_v = total_v + info.qtd_vend;
			total_l = total_l + ((info.v_venda - info.v_compra) * info.qtd_vend);
		}
	}
	
	printf("Total de vendas do dia: %d\n", total_v);
	printf("Lucro total do dia: %.2f\n", total_l);
	
	fseek(f1, 0L, SEEK_SET);
	while(fread(&k, sizeof(key_product), 1, f1)){
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET); //coloca o ponteiro na linha dos dados do produto
			fread(&info, sizeof(info_product), 1, f2); //lê os dados
			info.qtd_vend = 0; //zera as vendas
			
			fseek(f2, (k.index-1)*sizeof(info_product), SEEK_SET); //novamente coloca o ponteiro na linha dos dados do produto
			fwrite(&info, sizeof(info_product), 1, f2); //reescreve os dados (vendas zeradas)
	}
	
	fclose(f1);
	fclose(f2);
}

void sortFile(FILE *f1){
	int i = 0, j;
	key_product aux[MAX];
	
	fseek(f1, 0L, SEEK_SET);
	
	while(fread(&aux[i], sizeof(key_product), 1, f1))  //lê o arquivo e armazena os dados no vetor
		i++;
	
	quicksort(aux, i);
	fseek(f1, 0L, SEEK_SET);
	
	for(j=0;j<i;j++)
		fwrite(&aux[j], sizeof(key_product), 1, f1);  //escreve o vetor ordenado no arquivo
}

void quicksort(key_product *aux, int n){
	sort(0, n-1, aux);
}

void sort(int esq, int dir, key_product *aux){
	int i, j;
	partition(esq, dir, &i, &j, aux);
	
	if(esq < j)
		sort(esq, j, aux);
		
	if(i<dir)
		sort(i, dir, aux);
}

void partition(int esq, int dir, int *i, int *j, key_product *aux){
	key_product x, w;
	
	*i = esq;
	*j = dir;
	x = aux[(*i + *j)/2];
	
	do{
		while(strcmp(x.nome, aux[*i].nome) > 0)
		    (*i)++;
	     
	    while(strcmp(x.nome, aux[*j].nome) < 0)
			(*j)--;

		if (*i <= *j){
			w = aux[*i];
			aux[*i] = aux[*j];
			aux[*j] = w;
			(*i)++;
			(*j)--;
    	} 
	}while(*i <= *j);
}

int verifyDeleted(key_product k, info_product info, FILE *f1, FILE *f2){
	int q, p = 0;
	key_product aux_k;
	info_product aux_info;
	
	fseek(f1, 0L, SEEK_SET);
	
	while(fread(&aux_k, sizeof(key_product), 1, f1)){
		fseek(f2, (aux_k.index-1) * sizeof(info_product), SEEK_SET);
		fread(&aux_info, sizeof(info_product), 1, f2);
		
		if(aux_info.deleted == 1){
			p = 1;
			q = aux_k.index;
			break;
		}
	}	
	
	if(p){
		fclose(f2);
		
		f2 = fopen("infs.bin", "r+b");  					//a flag foi mudada para r+b com o objetivo de evitar
		if(f2 == NULL){										//que o produto seja inserido no final do arquivo
			printf("ERRO! O arquivo nao foi aberto.\n");
			return 0;
		}
		
		k.index = q;
		fseek(f1, -56, SEEK_CUR); //volta pro começo do registro (56 = sizeof(key_product))
		fwrite(&k, sizeof(key_product), 1, f1);
		
		fseek(f2, (q-1) * sizeof(info_product), SEEK_SET);
		fwrite(&info, sizeof(info_product), 1, f2);
		
		fclose(f2);
		return 1;
	}
	
	else return 0;
}

int binarySearch(char *aux, key_product k, FILE *f1){
	int i = 0;
	key_product q[MAX];
	
	while(fread(&k, sizeof(key_product), 1, f1)){
		strcpy(q[i].nome, k.nome);
		q[i].index = k.index;
		i++;
	}
	
	int inf = 0, sup = i-1;
	
	while(inf <= sup){
		int meio = (inf + sup) / 2;
		
		if (strcmp(aux, q[meio].nome) < 0)
			sup = meio - 1;
		
		else if (strcmp(aux, q[meio].nome) > 0)
			inf = meio + 1;
		
		else return q[meio].index; //se encontrou, retorna o index
	}
	
	return 0;  //não encontrou
}

int fileSize(FILE *f1){
	int i = 1;
	key_product aux;
	
	fseek(f1, 0L, SEEK_SET);
	
	while(fread(&aux, sizeof(key_product), 1, f1))
		i++;
		
	return i;
}
