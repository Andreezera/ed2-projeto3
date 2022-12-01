/* Feito por:
Paulo José dos Santos
João Victor Figueredo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bt.h"
#include <locale.h> //necessário para usar setlocale


#define REGSIZE sizeof(reg)
typedef struct {
	int CodCli, CodF, offset;
} primKey;

typedef struct {
	char CodCli[3], CodF[3];
} typeBusca;
#define BUSCASIZE sizeof(typeBusca)
void insereArq (int tempInsercoes, short *promo_rrn);
void buscaP(int codC, int codF), buscaPArq(int tempBuscas);
int insere(int *key);

int main(void)
{	
	setlocale(LC_ALL,"");
	BTPAGE page;
	int op=1, tempInsercoes=0, codC, codF, buscasS=0, promoted, pos;
	short root,     // rrn of root page
    promo_rrn;
    int promo_key[3], // key promoted from below
    key[3];
    
  	FILE *pout, *out;
	
	  // rrn promoted from below
	        // next key to insert in tree
	
	//scanf(" %c", &key);
  	
	if ((pout = fopen("streaming.bin","rb")) == NULL)
	{
		if ((pout = fopen("streaming.bin","w+b")) == NULL)
		{
			printf("Não foi possivel abrir o arquivo principal, verifique se já está aberto em outro programa!");
			return 0;
		}
	}
	
	fclose(pout);
	
	
  	while (op != 7)
   	{
   		printf("\nPor favor digite a opção:\n1.Inserir\n2.Listar todos os dados\n3.Listar dados de um cliente específico\n4.Insere do arquivo\n5.Busca do arquivo\n0.Sair\n");
   		scanf(" %d", &op);
   		
   		switch(op){
	   	
	   		case 1:
	   			if ((out = fopen("streaming.bin","a+b")) == NULL)
				{
					if ((out = fopen("streaming.bin","w+b")) == NULL)
					{
						printf("Nao foi possivel abrir o arquivo");
						return;
					}
				}
    			fseek(out, 0, SEEK_END);
    			pos=ftell(out);
    			fclose(out);
    			
	   			printf("\nDigite o código do cliente:\n");
    			scanf(" %d", &key[0]);
    			printf("\nDigite o código do filme:\n");
    			scanf(" %d", &key[1]);
    			key[2] = (pos/REGSIZE);
    			
	   				if (btopen())
					{
    					root = getroot();
    					promoted = insert(root, key, &promo_rrn, promo_key);
    					if (promoted==1)
        					root = create_root(promo_key, root, promo_rrn);
					}else
					{
    					root = create_tree(key);
					}
    				
    				if(promoted != -1){
    					insere(key);
					}
    				
        			btclose();
				
	   			break;
	   			
	   		case 2:
					if (btopen())
					{
    					root = getroot();
    					listarTodosDados(root, key);				
					}else{
						printf("Não foi possível abrir o arquivo de indíces");
					}
					btclose();
				break;	   			
			
			case 3:
				printf("\nInsira o codigo do cliente:\n");
				int codC, codF;
				scanf(" %d", &codC);
				printf("\nInsira o codigo do filme:\n");
				scanf(" %d", &codF);
				printf("\n\n");
				if (btopen())
				{				
    				root = getroot();
    				int foiAchado = listarClienteByKey(root, &page, codC, codF);	
					if(foiAchado < 1){
						printf("Chave %d%d não encontrada.\n", codC, codF);
					}			
				}else{
					printf("Não foi possível abrir o arquivo de indíces\n");
				}
					btclose();
				break;				
			case 4:
				printf("\nDigite quantas inserções serão feitas:\n");
	   			scanf(" %d", &tempInsercoes);
				insereArq(tempInsercoes, &promo_rrn);
				break;
			case 5:
					if (btopen())
					{		
    					printf("\nDigite quantas buscas serão feitas:\n");
			   			scanf(" %d", &tempInsercoes);
						buscaPArq(tempInsercoes);				
					}else{
						printf("Não foi possível abrir o arquivo de indíces\n");
					}
					btclose();
		
				break;
			case 0:
				break;
			
		} 
   	}
   	
   	printf("\nSaindo...\n");	
}


void insereArq (int tempInsercoes, short *promo_rrn){
	
	int pos, posI, teste, i=0, posicao, insercoes=0, key[3], promoted, promo_key[3];
	short root;
	FILE *pout, *out, *b_out;
	reg REG;
	
	if((b_out = fopen("utilizados.bin", "r+b")) == NULL){
  		b_out = fopen("utilizados.bin", "w+b");
  		fwrite(&insercoes, sizeof(int), 1, b_out);
  		fwrite(&insercoes, sizeof(int), 1, b_out);
  		insercoes = 0;
	}else{
		fread(&insercoes, sizeof(int), 1, b_out);
	}
	
	if ((out = fopen("streaming.bin","a+b")) == NULL)
	{
		if ((out = fopen("streaming.bin","w+b")) == NULL)
		{
			printf("Nao foi possivel abrir o arquivo");
			return;
		}
	}
	
	if ((pout = fopen("insere.bin","r+b")) == NULL)
	{
		printf("Nao foi possivel abrir o arquivo\n\n");
		return;
	}
	
  	posI = ftell(out);
  	
  	fseek(pout, 0, SEEK_END);
  	fseek(out, 0, SEEK_END);
  	
  	pos = ftell(out);	
  	posicao = ftell(pout);
  	
  	fseek(out, posI, 0);
  	
  	fseek(pout, sizeof(reg)*insercoes, 0);
  		
  	while (posicao!=ftell(pout) && i<tempInsercoes)
   	{
		fread(&REG, sizeof(reg), 1, pout);
        
        key[0] = atoi(REG.CodCli);
        key[1] = atoi(REG.CodF);
        
    	key[2] = (pos/REGSIZE);
    			
	   	if (btopen())
		{
    		root = getroot();
    		promoted = insert(root, key, &promo_rrn, promo_key);
    		if (promoted==1)
        		root = create_root(promo_key, root, promo_rrn);
		}else
			{
    			root = create_tree(key);
			}
    				
    		if(promoted != -1){
    			fwrite(&REG, sizeof(reg), 1, out);
    			printf("\nChave %s%s inserida com sucesso\n", REG.CodCli, REG.CodF);
    			pos += sizeof(reg);
			}
    				
        	btclose();
			
		i++;
   	}
   		
   	insercoes += i;
   	
   	fseek(b_out, 0, 0);
   	fwrite(&insercoes, sizeof(int), 1, b_out);
   	fclose(b_out);

	fclose(out);
	fclose(pout);
	
   	if(i!=tempInsercoes){
   		printf("\nO arquivo tinha apenas %d inserções disponíveis, foram inseridos %d registros\n", i, i);
   		return;
	}	
   	
	
}

void buscaPArq(int tempBuscas){
	int i=0, posicao, buscas=0, key[2];
	short root;
	FILE *pout, *b_out;
	BTPAGE page;
	typeBusca BUSCA;
	
	if((b_out = fopen("utilizados.bin", "r+b")) == NULL){
  		b_out = fopen("utilizados.bin", "w+b");
  		fwrite(&buscas, sizeof(int), 1, b_out);
  		fwrite(&buscas, sizeof(int), 1, b_out);
  		buscas = 0;
	}else{
		fseek(b_out, sizeof(int), 0);
		fread(&buscas, sizeof(int), 1, b_out);
	}
	
	
	if ((pout = fopen("busca.bin","r+b")) == NULL)
	{
		printf("Nao foi possivel abrir o arquivo de busca\n\n");
		return;
	}
	
  	
  	fseek(pout, 0, SEEK_END);
	  	
  	posicao = ftell(pout);
  	
  	fseek(pout, BUSCASIZE*buscas, 0);

  	while(posicao!=ftell(pout) && i < tempBuscas)
   	{
   		fread(&BUSCA, BUSCASIZE, 1, pout);
   		
        key[0] = atoi(BUSCA.CodCli);
        key[1] = atoi(BUSCA.CodF);
        
	   	if (btopen())
			{			
				root = getroot();	
				listarClienteByKey(root, &page, key[0], key[1]);
				
			}
		else{
			printf("\nNão foi possível abrir o arquivo de indíces\n");
			
			return;
		}
		btclose();
			
		i++;
   	}
   	
	if(i!=tempBuscas){
   		printf("\nO arquivo tinha apenas %d buscas disponíveis, foram buscados %d registros\n", i, i);
   		return;
	}
	   	
   	buscas += i;
   	
   	fseek(b_out, sizeof(int), 0);
   	fwrite(&buscas, sizeof(int), 1, b_out);
   	fclose(b_out);

	fclose(pout);   	
	
}


