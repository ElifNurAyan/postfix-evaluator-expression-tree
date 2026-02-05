#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct Node{
    int type;          
    int operand;       
    char operator;    
    char unknown;      
    struct Node *left;
    struct Node *right;
}Node;

typedef struct StackNode{
    Node *node;
    struct StackNode *next;
}StackNode;

Node* createNode(int type,int operand, char operator,char unknown){
    Node* newNode=(Node*)malloc(sizeof(Node));
    newNode->type=type;
    newNode->operand=operand;
    newNode->operator=operator;
    newNode->unknown=unknown;
    newNode->left=NULL;
    newNode->right=NULL;
    return newNode;
}

void push(StackNode** top, Node* newNode){
    StackNode* stackNode = malloc(sizeof(StackNode));
    stackNode->node=newNode;
    stackNode->next=*top;
    *top=stackNode;
}

Node* pop(StackNode** top){
    if (*top == NULL) return NULL;
    StackNode* temp = *top;
    Node* result = temp->node;
    *top = (*top)->next;
    free(temp);
    return result;
}

int isOperator(char c){
    return (c=='+' || c=='-' || c=='*' || c=='/' || c=='^');
}

int isDivisionable(int a,int b){
    if(b==0) return 0;
    if(a % b != 0) return 0;
    return 1;
}

int makeCalculation(int left, int right, char ch){
    switch(ch){
        case '+': return left + right;
        case '-': return left - right;
        case '*': return left * right;
        case '/':
            if(!isDivisionable(left,right))
                return -100;  
            return left / right;
        case '^': return (int)pow(left,right);
    }
    return -100;
}

Node* cloneTree(Node* root){
    if(root==NULL) return NULL;
    Node* newNode = createNode(root->type, root->operand, root->operator, root->unknown);
    newNode->left = cloneTree(root->left);
    newNode->right = cloneTree(root->right);
    return newNode;
}

int isSatisfyable(char** tokens, int* unknown_index,int numberOfUnknowns, int* cases, int size){
    int stack=0;

    for(int i=0;i<size;i++){
        char ch = tokens[i][0];

        if(isdigit(ch)){
            stack++;
        }
        else if(isOperator(ch)){
           stack--;
        }
        else if(isalpha(ch)){
            int current = -1;
            for(int j=0;j<numberOfUnknowns;j++){
                if(unknown_index[j] == i){
                    current = j;
                    break;
                }
            }
            if(current == -1) return 0;

            if(cases[current] == 0){
                stack++;
            } else {
                if(stack < 2) return 0;
                stack--;
            }
        }
        else return 0;
    }

     return (stack==1);
    
}

int** cases(int* unknown_index, int numberOfUnknowns,char** tokens,int size,int* trueCount,int unknownOperandNumber,int unknownOperatorNumber){

    int capacity = 10;
    int** trueCases = malloc(capacity * sizeof(int*));
    *trueCount = 0;
    int maxBinary = 1 << numberOfUnknowns;

    int* caseArray = malloc(numberOfUnknowns * sizeof(int));

    for(int binary=0; binary<maxBinary; binary++){
        int operatorCount=0, operandCount=0;

        for(int j=0;j<numberOfUnknowns;j++){
            int bit = (binary >> j) & 1;
            caseArray[j] = bit;
            if(bit==0) operandCount++;
            else operatorCount++;
        }

        if(operandCount != unknownOperandNumber || operatorCount != unknownOperatorNumber)
            continue;

        if(isSatisfyable(tokens,unknown_index,numberOfUnknowns,caseArray,size)){
            if(*trueCount >= capacity){
                capacity*=2;
                trueCases = realloc(trueCases, capacity*sizeof(int*));
            }

            trueCases[*trueCount] = malloc(numberOfUnknowns*sizeof(int));
            for(int k=0;k<numberOfUnknowns;k++)
                trueCases[*trueCount][k] = caseArray[k];

            (*trueCount)++;
        }
    }

    free(caseArray);
    return trueCases;
}
Node* buildTree(char** tokens, int** cases, int size,int a,int numberOfUnknowns, int* unknown_index){
    StackNode* stack=NULL;

    for(int i=0;i<size;i++){
        char ch = tokens[i][0];

        if(isdigit(ch)){
            push(&stack, createNode(0, atoi(tokens[i]), '\0','\0'));
        }
        else if(isOperator(ch)){
            Node* right = pop(&stack);
            Node* left = pop(&stack);
            Node* newNode = createNode(1,-1,ch,'\0');
            newNode->left = left;
            newNode->right = right;
            push(&stack,newNode);
        }
        else if(isalpha(ch)){
            int which=-1;
            for(int j=0;j<numberOfUnknowns;j++){
                if(unknown_index[j] == i){
                    which=j;
                    break;
                }
            }

            if(cases[a][which] == 0){
                push(&stack, createNode(0,-1,'\0',ch));
            } else {
                Node* right = pop(&stack);
                Node* left = pop(&stack);
                Node* newNode = createNode(1,-1,'\0',ch);
                newNode->left=left;
                newNode->right=right;
                push(&stack,newNode);
            }
        }
    }

    Node* root = pop(&stack);
    return root;
}

Node* completeTree(Node* root,int* operands,char* operators,int* operandIndex, int* operatorIndex){

    if(root==NULL) return NULL;

    completeTree(root->left, operands, operators, operandIndex, operatorIndex);
    completeTree(root->right, operands, operators, operandIndex, operatorIndex);

    if(root->type==0 && root->operand==-1){
        root->operand = operands[*operandIndex];
        (*operandIndex)++;
    }
    if(root->type==1 && root->operator=='\0'){
        root->operator = operators[*operatorIndex];
        (*operatorIndex)++;
    }

    return root;
}

int evaluateTree(Node* root){
    if(root->type==0)
        return root->operand;

    int left = evaluateTree(root->left);
    int right = evaluateTree(root->right);

    return makeCalculation(left,right,root->operator);
}
int main(int arg_no, char* arg_n[]) {

    FILE *fptr = fopen(arg_n[1], "r");
    if (!fptr) {
        printf("File Error!");
        exit(1);
    }

    char line[320];
    fgets(line, 320, fptr);
    fclose(fptr);

    int max_size = 40;
    char** tokens = malloc(max_size * sizeof(char*));
    int size = 0;

    char* token = strtok(line, " ");

    while (token != NULL && strcmp(token, "=") != 0) {

        if (size >= max_size) {
            max_size *= 2;
            tokens = realloc(tokens, max_size * sizeof(char*));
        }

        tokens[size] = malloc(strlen(token) + 1);
        strcpy(tokens[size], token);
        size++;

        token = strtok(NULL, " ");
    }

    fptr = fopen(arg_n[1], "r");

    int ch = fgetc(fptr);
    int result = 0;

    while (ch != EOF) {
        if (ch == '=') {

            ch = fgetc(fptr);
            while (ch == ' ') ch = fgetc(fptr);

            while (isdigit(ch)) {
                result = result * 10 + (ch - '0');
                ch = fgetc(fptr);
            }
            break;
        }
        ch = fgetc(fptr);
    }
    fclose(fptr);

    int unknown_capacity = 25;
    int* unknown_index = malloc(unknown_capacity * sizeof(int));

    int numberOfUnknowns = 0;
    int numberOfOperands = 0;
    int numberOfOperators = 0;

    for (int i = 0; i < size; i++) {

        if (isalpha(tokens[i][0])) {
            if (numberOfUnknowns >=unknown_capacity) {
                unknown_capacity *= 2;
                unknown_index = realloc(unknown_index, unknown_capacity * sizeof(int));
            }
            unknown_index[numberOfUnknowns] = i;
            numberOfUnknowns++;
        }
        else if (isdigit(tokens[i][0])) numberOfOperands++;
        else if (isOperator(tokens[i][0])) numberOfOperators++;
    }

    int unknownOperatorNumber = (numberOfOperands + numberOfUnknowns - 1 - numberOfOperators)/2;
    int unknownOperandNumber  = numberOfUnknowns - unknownOperatorNumber;

    int trueCount = 0;
    int** trueCases = cases(unknown_index,numberOfUnknowns,tokens,size,&trueCount,unknownOperandNumber,unknownOperatorNumber);

    int* operands = malloc(unknownOperandNumber * sizeof(int));
    char* operators = malloc(unknownOperatorNumber * sizeof(char));
    char operatorList[5] = {'+', '-', '*', '/', '^'};

    for (int a = 0; a < trueCount; a++) {

        Node* expressionTree = buildTree(tokens, trueCases, size, a, numberOfUnknowns, unknown_index);
        int possibleCases = pow(20, unknownOperandNumber) * pow(5, unknownOperatorNumber);
        for (int m = 0; m < possibleCases; m++) {
            int temp = m;
            for (int i = 0; i < unknownOperandNumber; i++) {
                operands[i] = (temp % 20) + 1;
                temp /= 20;
            }
            for (int j = 0; j < unknownOperatorNumber; j++) {
                operators[j] = operatorList[temp % 5];
                temp /= 5;
            }
            Node* copy = cloneTree(expressionTree);
            int operandIndex = 0;
            int operatorIndex = 0;
            completeTree(copy, operands, operators, &operandIndex, &operatorIndex);
            int solution = evaluateTree(copy);
            if (solution == result) {

                for (int k = 0; k < unknownOperandNumber; k++)
                    printf("%d ", operands[k]);

                for (int k = 0; k < unknownOperatorNumber; k++)
                    printf("%c ", operators[k]);

                printf("\n");
            }
        }
    }
    return 0;
}