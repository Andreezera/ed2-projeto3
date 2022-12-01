#define MAXKEYS 3
#define MINKEYS 2
#define NIL (-1)
#define NOKEY -2
#define NO 0
#define YES 1

#include <stdio.h>

FILE* btfd; // global file descriptor for "btree.dat"

typedef struct {
	char CodCli[3], CodF[3], NomeCli[50], NomeFilme[50], Gen[50];
} reg; 
typedef struct
{
    short keycount;           // number of keys in page
    int key[3][MAXKEYS];        // the actual keys
    short child[MAXKEYS + 1]; // ptrs to rrns of descendants
} BTPAGE;
#define PAGESIZE sizeof(BTPAGE)
extern short root; // rrn of root page
extern FILE* btfd;   // file descriptor of btree file
extern int infd;   // file descriptor of input file
/* prototypes */
void btclose();
int btopen();
char btread(short rrn, BTPAGE *page_ptr);
int btwrite(short rrn, BTPAGE *page_ptr);
short create_root(int *key, short left, short right);
short create_tree(int *key);
short getpage();
short getroot();
int insert(short rrn, int *key, short *promo_r_child, int *promo_key);
void ins_in_page(int *key, short r_child, BTPAGE *p_page);
void pageinit(BTPAGE *p_page);
void putroot(short root);
int search_node(int *key, BTPAGE *p_page, short *pos);
void split(int *key, short r_child, BTPAGE *p_oldpage, int *promo_key, short *promo_r_child, BTPAGE *p_newpage);

int btopen()
{	
	if ((btfd = fopen("btree.dat", "r+b")) == NULL)
	{
		if ((btfd = fopen("btree.dat","w+b")) == NULL)
		{
			printf("Nao foi possivel abrir o arquivo índice primário, verifique se já está aberto em outro programa!");
			return NO;
		}
		return NO;
	}
	return YES;
}

int insert (short rrn, int *key, short *promo_r_child, int *promo_key)
{
	BTPAGE page, // current page
	newpage; // new page created if split occurs
	int found, promoted, p_b_key[3], i, j; // boolean values
	short pos,
	p_b_rrn; // rrn promoted from below
	if (rrn == NIL)
	{
		promo_key[0] = key[0];
		promo_key[1] = key[1];
		promo_key[2] = key[2];
		*promo_r_child = NIL;
		return(YES);
	}
	btread(rrn, &page);
	found = search_node(key, &page, &pos);
	
	if (found==YES)
	{
		printf ("Chave já inserida\n", key);
		return(-1);
	}
	promoted = insert(page.child[pos], key, &p_b_rrn, p_b_key);
	if (promoted==-1 || !promoted)
	{
		return(promoted);
	}
	
	if(page.keycount < MAXKEYS)
	{
		ins_in_page(p_b_key, p_b_rrn, &page);
		btwrite(rrn, &page);
		return(NO);
	}
	else
	{
		printf("\nDivisão de nó");
		split(p_b_key, p_b_rrn, &page, promo_key, promo_r_child, &newpage);
		btwrite(rrn, &page);
		btwrite(*promo_r_child, &newpage);
		return(YES);
	}
	
}

int insere(int *key)
{
    FILE *out;
    reg registro;
	
	if ((out = fopen("streaming.bin","a+b")) == NULL)
	{
		if ((out = fopen("streaming.bin","w+b")) == NULL)
		{
			printf("Nao foi possivel abrir o arquivo");
			return(-1);
		}
	}
    
    itoa(key[0], registro.CodCli, 10);
    itoa(key[1], registro.CodF, 10);
    	fflush(stdin);
    printf("\nDigite o nome do cliente:\n");
	gets(registro.NomeCli);
    printf("\nDigite o nome do filme:\n");
	gets(registro.NomeFilme);
    printf("\nDigite o gênero do filme:\n");
    gets(registro.Gen);

    printf("\n");
    
   
	if(fwrite(&registro, sizeof(reg), 1, out) == 1){
		printf("\nChave %s%s inserida com sucesso\n", registro.CodCli, registro.CodF);
		fclose(out);
		return(0);
	}
	
	return(-1);
	
}     


void btclose()
{
    fclose(btfd);
}

short getroot()
{
    short root;

    fseek(btfd, 0L, 0);
    if (fread(&root, 2, 1, btfd) == 0)
    {
        printf("Error: Unable to get root. \007\n");
        return(-99);
    }
    return (root);
}

void putroot(short root)
{
    fseek(btfd, 0L, 0);
    fwrite(&root, sizeof(short), 1, btfd);
}

short create_tree(int *key)
{
    btfd = fopen("btree.dat", "w+b");
    fclose(btfd);
    btopen();
    return (create_root(key, NIL, NIL));
}

short getpage()
{
    long addr;
    fseek(btfd, 0, 2);
    addr = ftell(btfd) - 2;
    
    if(addr<0) return 0;
    
	return ((short)addr / PAGESIZE);
}

char btread(short rrn, BTPAGE *page_ptr)
{
    long addr;
    addr = (long)rrn * (long)PAGESIZE + 2L;
    fseek(btfd, addr, 0);
    return (fread(page_ptr, PAGESIZE, 1, btfd));
}

int btwrite(short rrn, BTPAGE *page_ptr)
{
    long addr;
    addr = (long)rrn * (long)PAGESIZE + 2L;
    fseek(btfd, addr, 0);
    return (fwrite(page_ptr, PAGESIZE, 1, btfd));
}

short create_root(int *key, short left, short right)
{
    BTPAGE page;

    short rrn;
    rrn = getpage();
    pageinit(&page);
    page.key[0][0] = key[0];
    page.key[0][1] = key[1];
    page.key[0][2] = key[2];
    page.child[0] = left;
    page.child[1] = right;
    page.keycount = 1;
    btwrite(rrn, &page);
    putroot(rrn);
    return (rrn);
}

void pageinit(BTPAGE *p_page)
{
    int j;
    for (j = 0; j < MAXKEYS; j++)
    {
        p_page->key[j][0] = NOKEY;
        p_page->key[j][1] = NOKEY;
        p_page->key[j][2] = NOKEY;
        p_page->child[j] = NIL;
    }

    p_page->child[MAXKEYS] = NIL;
}

int search_node(int *key, BTPAGE *p_page, short *pos)
{
    int i=0;
    
    for (i = 0; i < p_page->keycount && key[0]+key[1] >= p_page->key[i][0] + p_page->key[i][1]; i++){
    	*pos = i;
    	if (*pos < p_page->keycount && key[0] == p_page->key[*pos][0] && key[1] == p_page->key[*pos][1])
		{
        	return (YES);
    	}
	}
	*pos = i;
	return (NO);    
}

void listarTodosDados(short rrn, BTPAGE *p_page){
	BTPAGE page, // current page
	newpage;
	FILE *out;
	reg REG; // new page created if split occurs
	int found, promoted, p_b_key[3], a=0, i; // boolean values
	short pos,
	p_b_rrn; // rrn promoted from below
	
	btread(rrn, p_page);
	
	for (i = 0; i < MAXKEYS; i++){
    	if(p_page->child[i] != -1){
    		listarTodosDados(p_page->child[i], &page);
		}    	
	}
	
	if((out = fopen("streaming.bin", "rb")) == NULL){
		printf("\nNão foi possível abrir o arquivo principal\n");
		return;
	}
	
	for (i = 0; i < p_page->keycount; i++){
		fseek(out, p_page->key[i][2]*sizeof(REG), 0);
		fread(&REG, sizeof(reg), 1, out);
		
    	printf("Código do cliente: %s\n", REG.CodCli);
    	printf("Código do filme: %s\n", REG.CodF);
    	printf("Nome do cliente: %s\n", REG.NomeCli);
    	printf("Nome do filme: %s\n", REG.NomeFilme);
    	printf("Genero do filme: %s\n\n", REG.Gen);
	}
}

short listarClienteByKey(short rrn, BTPAGE *p_page, int codC, int codF){
	BTPAGE page, // current page
	newpage;
	FILE *out;
	reg REG; // new page created if split occurs
	int found, promoted, p_b_key[3], a=0, i; // boolean values
	short pos,
	p_b_rrn; // rrn promoted from below
	int posicaoAchada = 0;
	btread(rrn, p_page);
	for (i = 0; i < MAXKEYS; i++){
		if(p_page->key[i][0]==codC && p_page->key[i][1]==codF){
			posicaoAchada = p_page->key[i][2];
			if((out = fopen("streaming.bin", "rb")) == NULL){
				printf("\nNão foi possível abrir o arquivo principal\n");
				return -1;
			}else{
				int pedro = fseek(out, posicaoAchada*sizeof(REG), 0);
				fread(&REG, sizeof(reg), 1, out);		
				printf("Chave %d%d encontrada, página : %d, posição: %d\n", codC, codF,rrn, i);
		    	printf("Código do cliente: %s\n", REG.CodCli);
		    	printf("Código do filme: %s\n", REG.CodF);
		    	printf("Nome do cliente: %s\n", REG.NomeCli);
		    	printf("Nome do filme: %s\n", REG.NomeFilme);
		    	printf("Genero do filme: %s\n\n", REG.Gen);
		    	return 1;
				
			}
		}else if(p_page->child[i] != -1){
    		if(listarClienteByKey(p_page->child[i], &page, codC, codF) == 1){
    			return 1;
			};
		}
	}
}

void buscarPorArquivo(){
	int buscas = 0;
	printf("Quantas buscaas desejas realizar?\n");
	scanf(" %d", &buscas);
}

void ins_in_page(int *key, short r_child, BTPAGE *p_page)
{
    int j;
    for (j = p_page->keycount; key[0]+key[1] < p_page->key[j - 1][0]+p_page->key[j - 1][1] && j > 0; j--)
    {
        p_page->key[j][0] = p_page->key[j - 1][0];
        p_page->key[j][1] = p_page->key[j - 1][1];
        p_page->key[j][2] = p_page->key[j - 1][2];
        p_page->child[j + 1] = p_page->child[j];
    }
    p_page->keycount++;
    p_page->key[j][0] = key[0];
    p_page->key[j][1] = key[1];
    p_page->key[j][2] = key[2];
    p_page->child[j + 1] = r_child;
}

void split(int *key, short r_child, BTPAGE *p_oldpage, int *promo_key, short *promo_r_child, BTPAGE *p_newpage)
{
    int j;
    short mid;
    int workkeys[MAXKEYS + 1][3];

    short workchil[MAXKEYS + 2];
    for (j = 0; j < MAXKEYS; j++)
    {
    	workkeys[j][0] = p_oldpage->key[j][0];
    	workkeys[j][1] = p_oldpage->key[j][1];
    	workkeys[j][2] = p_oldpage->key[j][2];
        workchil[j] = p_oldpage->child[j];
    }
    workchil[j] = p_oldpage->child[j];
    
    for (j = MAXKEYS; key[0] + key[1] < workkeys[j - 1][0]+workkeys[j-1][1] && j > 0; j--)
    {
    	workkeys[j][0] = workkeys[j - 1][0];
    	workkeys[j][1] = workkeys[j - 1][1];
    	workkeys[j][2] = workkeys[j - 1][2];
        workchil[j + 1] = workchil[j];
    }
    
    workkeys[j][0] = key[0];
    workkeys[j][1] = key[1];
    workkeys[j][2] = key[2];
    
    workchil[j + 1] = r_child;
    *promo_r_child = getpage();
    pageinit(p_newpage);
    
    p_oldpage->key[0][0] = workkeys[0][0];
    p_oldpage->key[0][1] = workkeys[0][1];
    p_oldpage->key[0][2] = workkeys[0][2];
    
    p_oldpage->child[0] = workchil[0];
    
    for (j = 0; j < MINKEYS; j++)
    {
    	p_newpage->key[j][0] = workkeys[j + MINKEYS][0];
    	p_newpage->key[j][1] = workkeys[j + MINKEYS][1];
    	p_newpage->key[j][2] = workkeys[j + MINKEYS][2];
        p_newpage->child[j] = workchil[j + MINKEYS]; 
        
        p_oldpage->key[j+MINKEYS-1][0] = NOKEY;
        p_oldpage->key[j+MINKEYS-1][1] = NOKEY;
        p_oldpage->key[j+MINKEYS-1][2] = NOKEY;
    	p_oldpage->child[j+MINKEYS] = NIL;
    }
    
    
    
    p_oldpage->child[MINKEYS] = workchil[MINKEYS];
    p_newpage->child[MINKEYS] = workchil[j + MINKEYS];
    p_newpage->keycount = MAXKEYS - MINKEYS + 1;
    p_oldpage->keycount = MINKEYS-1;
    promo_key[0] = workkeys[MINKEYS-1][0];
    promo_key[1] = workkeys[MINKEYS-1][1];
    promo_key[2] = workkeys[MINKEYS-1][2];
    printf("\nChave %d %d promovida", promo_key[0], promo_key[1]);
}
