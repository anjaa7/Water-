
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

//Pocetna sekvenca za random generator
int S[] = { 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
int J = 7, K = 10, MOD;
int n, k, p;
int pozivi = 0;

int generateRandomNumber() {
	int val = S[10 - J] * S[10 - K] % MOD;
	
	for (int i = 0; i < 9; i++) S[i] = S[i + 1];
	S[9] = val;

	return val;
}

typedef struct Node {
	int **a;
	int depth;
	int canWin;
	int winPosition;
	int korak_i, korak_j;
	struct Node* parent;
	struct Node** children;
}Node;
Node* winningNode = NULL;

typedef struct StackNode {
	Node* node;
	struct StackNode* next;
} StackNode;

typedef struct QueueNode {
	Node* node;
	struct QueueNode* next;
}QueueNode;

typedef struct Queue {
	QueueNode *front, *rear;
}Queue;

StackNode* newStackNode(Node* node)
{
	StackNode* stackNode = (StackNode*)malloc(sizeof(StackNode));
	stackNode->node = node;
	stackNode->next = NULL;
	return stackNode;
}

int emptyStack(StackNode* stack)
{
	return stack == NULL;
}

void pushStack(StackNode** root, Node* node)
{
	StackNode* stackNode = newStackNode(node);
	stackNode->next = *root;
	*root = stackNode;
}

Node* popStack(StackNode** root)
{
	if (emptyStack(*root)) return NULL;
	
	StackNode* temp = *root;
	*root = (*root)->next;
	Node* node = temp->node;
	free(temp);

	return node;
}

QueueNode* newQueueNode(Node *node)
{
	QueueNode* temp = (QueueNode*)malloc(sizeof(QueueNode));
	temp->node = node;
	temp->next = NULL;
	return temp;
}

Queue* createQueue()
{
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->front = q->rear = NULL;
	return q;
}

 int emptyQueue(Queue *q) {
	 return q->front == NULL;
 }

void pushQueue(Queue* q, Node* node)
{
	QueueNode* temp = newQueueNode(node);

	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	q->rear->next = temp;
	q->rear = temp;
}

Node* popQueue(Queue* q)
{
	if (q->front == NULL)
		return;

	QueueNode* temp = q->front;

	q->front = q->front->next;

	if (q->front == NULL)
		q->rear = NULL;

	Node* node = temp->node;
	//free(temp);
	return node;
}

Node* allocateNode(Node* parent) {
	pozivi++;
	Node *q = (Node*)malloc(sizeof(Node));
	q->a = (int**)malloc(sizeof(int*)*4);
	
	for (int i = 0; i < 4; i++) {
		q->a[i] = (int*)malloc(sizeof(int) * n);	
	}

	q->canWin = 0;
	q->winPosition = -1;
	q->korak_i = -1;
	q->korak_j = -1;

	if (parent) {
		q->depth = parent->depth + 1;
		q->parent = parent;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < n; j++) {
				q->a[i][j] = q->parent->a[i][j];
			}
		}
	}
	else {
		q->depth = 0;
		q->parent = NULL;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < n; j++) {
				q->a[i][j] = 0;
			}
		}
	}

	/*
	Svaki cvor ce imati n*n deteta zato sto posmatramo sve moguce kombinacije poteza tj.
	(a, b) -> jedan korak u kome se prebacuje iz boce a u bocu b
	Takvih kombinacija (ukljucujuci nevalidne) ima n*n.
	Nevalidna kombinacije je npr. (a, a) ili (a, b) gde je boca a prazna ili b puna
	ili se ne poklapaju boje vrhova obe boce
	*/

	q->children = (Node**)malloc(sizeof(Node*)*n*n);
	for (int i = 0; i < n*n; i++)q->children[i] = NULL;

	return q;
}

int checkForSolution(Node* node) {
	for (int j = 0; j < n; j++) {
		for (int i = 0; i < 3; i++) {
			if (node->a[i][j] != node->a[i + 1][j]) {
				return 0;
			}
		}
	}
	return 1;
}

void generateTree(Node* root) {
	Queue* q = createQueue();
	StackNode* stack = NULL;

	pushStack(&stack, root);
	pushQueue(q, root);

	while (!emptyQueue(q)) {
		Node* node = popQueue(q);

		if (checkForSolution(node)) {
			node->canWin = 1;
			winningNode = node;
			continue;
		}

		if (node->depth == p)continue;

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (i == j)node->children[i*n + j] = NULL;
				else if (node->a[0][j] != 0)node->children[i*n + j] = NULL; //druga boce je puna
				else if (node->a[3][i] == 0)node->children[i*n + j] = NULL; //prva boca je prazna
				else {
					int boja_i = 0, boja_j = 0;
					//nalazimo boju vrha obe boce
					for (int k = 0; k < 4; k++) {
						if (node->a[k][i] != 0) {
							boja_i = node->a[k][i];
							break;
						}
					}
					for (int k = 0; k < 4; k++) {
						if (node->a[k][j] != 0) {
							boja_j = node->a[k][j];
							break;
						}
					}

					if (boja_j != 0 && boja_i != boja_j) node->children[i*n + j] = NULL; //ne poklapaju se boje vrhova, takodje boja_j nije 0 jer j nije prazna
					else {
						// moguce je prebaciti barem jednu boju iz boce i u bocu j jer se poklapaju boje ili je j prazna
						node->children[i*n + j] = allocateNode(node);
						node->children[i*n + j]->korak_i = i;
						node->children[i*n + j]->korak_j = j;

						int pos_i = 0, pos_j = 0;

						while (node->a[pos_i][i] == 0)pos_i++;
						while (pos_j <= 3 && node->a[pos_j][j] == 0)pos_j++;
						if (pos_j == 4)pos_j = 3;
						if (node->a[pos_j][j] != 0)pos_j--;

						while (pos_i <= 3 && pos_j >= 0 && node->a[pos_i][i] == boja_i) {
							node->children[i*n + j]->a[pos_i][i] = 0;
							node->children[i*n + j]->a[pos_j][j] = boja_i;
							pos_i++;
							pos_j--;
						}

						pushQueue(q, node->children[i*n + j]);
					}
				}
			}
		}
		pushStack(&stack, node);
	}

	while (!emptyStack(stack)) {
		Node* node = popStack(&stack);
		for (int i = 0; i < n*n; i++) {
			if (node->children[i] != NULL && node->children[i]->canWin) {
				node->canWin = 1;
				node->winPosition = i;
				break;
			}
		}
	}
}

void preorderTraversal(Node *node) {
	if (node == NULL)return;

	StackNode* stack = NULL;
	pushStack(&stack, node);

	while (!emptyStack(stack)) {
		Node* tmp = popStack(&stack);

		printf("Nivo %d, korak %d -> %d. Moguca pobeda? %d\n", tmp->depth, tmp->korak_i, tmp->korak_j, tmp->canWin);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < n; j++) {
				printf("%d ", tmp->a[i][j]);
			}
			printf("\n");
		}
		printf("\n");

		for (int i = n * n - 1; i >= 0; i--) {
			if (tmp->children[i] != NULL)pushStack(&stack, tmp->children[i]);
		}
	}
}

void printValidSolution() {
	if (winningNode == NULL) {
		printf("Nema resenja\n");
		return;
	}
	printf("Jedno validno resenje\n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < n; j++) {
			printf("%d ", winningNode->a[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int tryToFit(Node* node, int rnd, int color) {
	int pos = 0;
	while (pos <= 3 && node->a[pos][rnd] == 0)pos++;
	if (pos == 4 || node->a[pos][rnd] != 0)pos--;

	if (pos >= 0 && node->a[pos][rnd] == 0) {
		node->a[pos][rnd] = color;
		return 1;
	}
	return 0;
}

void generateMatrix(Node* node) {
	for (int j = 0; j < 4; j++) {
		for (int i = 1; i <= n - k; i++) {
			int rnd = generateRandomNumber();
			int t = 0;
			while (tryToFit(node, (rnd + t) % (n - k), i) != 1)t++;
		}
	}
}

int main() {
	Node* root = NULL;
	while (1) {
		printf("Unesite izbor\n");
		printf("0. Kraj programa\n");
		printf("1. Generisanje igre\n");
		printf("2. Ispis stabla\n");
		printf("3. Igraj\n");
		printf("4. Ispis jednog validnog resenja\n");
		int c;
		scanf("%d", &c);

		if (c == 1) {
			printf("Unesi n, k i p\n");
			scanf("%d %d %d", &n, &k, &p);
			MOD = n - k;
			root = allocateNode(NULL);
			generateMatrix(root);
			generateTree(root);
		}
		else if (c == 2) {
			preorderTraversal(root);
			printf("\n");
		}
		else if (c == 3) {
			Node* currentState = root;
			while (1) {
				if (currentState == NULL) {
					printf("Kraj igre, nije doslo do pobede\n\n");
					break;
				}

				if (checkForSolution(currentState)) {
					printf("Kraj igre, pobeda\n");
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < n; j++) {
							printf("%d ", currentState->a[i][j]);
						}
						printf("\n");
					}
					printf("\n");
					break;
				}

				printf("Trenutno stanje\n");
				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < n; j++) {
						printf("%d ", currentState->a[i][j]);
					}
					printf("\n");
				}

				printf("\nPotrebna pomoc? 1-DA : 2-NE\n");
				int c;
				scanf("%d", &c);
				if (c == 1) {
					if (currentState->canWin == 0) {
						printf("\nOdavde se ne moze pobediti\n");
						break;
					}
					else {
						currentState = currentState->children[currentState->winPosition];
						printf("\nKorak %d -> %d\n", currentState->korak_i, currentState->korak_j);
					}
				}
				else {
					int korak_i, korak_j;
					printf("\nUnesi poziciju prve boce i poziciju druge boce\n");
					scanf("%d %d", &korak_i, &korak_j);
					if (korak_i >= n || korak_i < 0 || korak_j >= n || korak_j < 0) {
						printf("Nevalidan potez\n");
						break;
					}
					currentState = currentState->children[korak_i*n + korak_j];
				}
			}
		}
		else if (c == 4) {
			printValidSolution();
			printf("\n");
		}
		else {
			break;
		}
	}

	return 0;
}
asp_2.txt
Displaying asp_2.txt.
