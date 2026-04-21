#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG 0
 
#define push(val) stack[stackCounter++] = (val)
#define pop() stack[--stackCounter]
#define peek() stack[stackCounter - 1]
 

typedef enum {
  OP_NIL,
  OP_NUM,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,

  OP_EQ,

  OP_EE,
  OP_NE,
  OP_GT,
  OP_LT,
  OP_GE,
  OP_LE,

  OP_DUP,
  OP_OVER,
  OP_SWAP,
  OP_ROT,
  OP_DROP,

  OP_IF,
  OP_WHILE,
  OP_DO,
  OP_END,

  OP_PRINT,
  OP_EXIT,
} Op_kind;

typedef struct{
  Op_kind kind;
  union{
    int val;  
    int jump;
  }as;
}Op;

typedef struct{
  Op *data;
  size_t count;
  size_t capacity;
}Op_DA;

void init_OpDA(Op_DA *ops){
  ops->capacity = 8;
  ops->count = 0;
  ops->data = malloc(ops->capacity*sizeof(Op));
}

void append_OpDA(Op_DA *ops, Op op){
  if (ops->count >= ops->capacity){
    ops->capacity *= 2;
    ops->data = realloc(ops->data, ops->capacity*sizeof(Op));
  }
  ops->data[ops->count++] = op;
}

void print_OpDA(Op_DA *ops){
  printf("[ ");
  for (int i = 0; i < ops->count; ++i){
    printf("{kind:%d val:%d}, ", ops->data[i].kind, ops->data[i].as.val);
  }
  printf("]\n");
}

void free_OpDA(Op_DA *ops){
  free(ops->data);
  ops->capacity = ops->count = 0;
}

char *readFile(char *fileName){

  FILE *fp = fopen(fileName, "r");
  if (!fp) {
    perror("fopen");
    exit(1);
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  char *buf = malloc(size + 1);
  if (!buf) {
    fclose(fp);
    exit(1);
  }

  size_t read = fread(buf, 1, size, fp);
  buf[read] = '\0';

  fclose(fp);

  return buf;

}

static const struct { const char *word; Op_kind kind; } keywords[] = {
  { "dup", OP_DUP},
  { "over", OP_OVER},
  { "swap", OP_SWAP},
  { "rot", OP_ROT},
  { "drop", OP_DROP},

  { "if", OP_IF },
  { "while", OP_WHILE },
  { "do" , OP_DO},
  { "end", OP_END },

  { "print", OP_PRINT },
  { "exit", OP_EXIT },

};

Op_kind check_Keyword(const char *start, size_t len){
  for (int i = 0; keywords[i].word; ++i){
    if (memcmp(start, keywords[i].word, len) == 0 && keywords[i].word[len] == '\0'){
      return keywords[i].kind;
    }
  }
  return OP_NIL;
}

void lexer(char *source, Op_DA *ops){
  const char *src = source;

  while(*src){
    if (isspace(*src)) src++;
    
    // TODO: add support for minus nums
    if (isdigit(*src)){
      char *end;
      long val = strtol(src, &end, 10);
      src = end;
      Op op = {OP_NUM, .as.val = (int)val};
      append_OpDA(ops, op);
      continue;
    }

    if (isalpha(*src) || *src == '_'){
      const char *start = src;
      while (isalnum(*src) || *src == '_') src++;
      int len = src - start;

      Op_kind kind = check_Keyword(start, len);
      Op op = {kind};
      append_OpDA(ops, op);
      continue;
      
    }

    if (*src == '+') append_OpDA(ops, (Op){OP_ADD});
    if (*src == '-') append_OpDA(ops, (Op){OP_SUB});
    if (*src == '*') append_OpDA(ops, (Op){OP_MUL});
    if (*src == '/') append_OpDA(ops, (Op){OP_DIV});
    if (*src == '=') {
      src++;
      if (*src == '=') {
        append_OpDA(ops, (Op){OP_EE});
      }else if (*src == '!'){
        append_OpDA(ops, (Op){OP_NE});
      }else{
        append_OpDA(ops, (Op){OP_EQ});
      }
      continue;
    }
    if (*src == '>') {
      src++;
      if (*src == '=') {
        append_OpDA(ops, (Op){OP_GE});
      } else {
        append_OpDA(ops, (Op){OP_GT});
      }
      continue;
    }
    if (*src == '<') {
      src++;
      if (*src == '=') {
        append_OpDA(ops, (Op){OP_LE});
      } else {
        append_OpDA(ops, (Op){OP_LT});
      }
      continue;
    }

    src++;
  }

  int stack[1024];
  int stackCounter = 0;

  for (int i = 0; i < ops->count; i++) {
    if (ops->data[i].kind == OP_IF ||
        ops->data[i].kind == OP_WHILE ||
        ops->data[i].kind == OP_DO) {
      push(i);
    }else if (ops->data[i].kind == OP_END){

      int top = pop();

      if (ops->data[top].kind == OP_DO) {

        int whileIdx = pop();

        ops->data[i].as.jump = whileIdx;
        ops->data[top].as.jump = i;

      } else {

        ops->data[i].as.jump = top; // End know where to jump back to
        ops->data[top].as.jump = i; // other stuff know where to jump forward

      }
    }
  }
}
int main() {

  char *buf = readFile("./exemples/fib.vctr");
  //printf("%s", buf);
  Op_DA ops = {0};
  init_OpDA(&ops);
  lexer(buf, &ops);  
  //print_OpDA(&ops);

  int stack[256];
  int stackCounter = 0;

  int i = 0;
  while (i < ops.count) {

    if (ops.data[i].kind == OP_NUM) {

      push(ops.data[i].as.val);

    }
    //
    //    Arithmatic
    //
    else if (ops.data[i].kind == OP_ADD) {

      int n1 = pop();
      int n2 = pop();

      push(n1 + n2);

    } else if (ops.data[i].kind == OP_SUB) {

      int n1 = pop();
      int n2 = pop();

      push(n1 - n2);

    } else if (ops.data[i].kind == OP_MUL) {

      int n1 = pop();
      int n2 = pop();

      push(n1 * n2);

    } else if (ops.data[i].kind == OP_DIV) {

      int n1 = pop();
      int n2 = pop();

      push(n1 / n2);

    }
    //
    //    Comparison
    //
    else if (ops.data[i].kind == OP_EE) {

      int n1 = pop();
      int n2 = pop();

      push(n1 == n2);
      
    } else if (ops.data[i].kind == OP_NE) {

      int n1 = pop();
      int n2 = pop();

      push(!(n1 == n2));

    } else if (ops.data[i].kind == OP_GT) {

      int n1 = pop();
      int n2 = pop();

      push(n1 > n2);

    } else if (ops.data[i].kind == OP_GE) {

      int n1 = pop();
      int n2 = pop();

      push(n1 >= n2);

    } else if (ops.data[i].kind == OP_LT) {

      int n1 = pop();
      int n2 = pop();

      push(n1 < n2);

    } else if (ops.data[i].kind == OP_LE) {

      int n1 = pop();
      int n2 = pop();

      push(n1 <= n2);

    }
    //
    //    Stack manipulation
    // 
    else if (ops.data[i].kind == OP_DUP){

      push(peek());
      
    }else if(ops.data[i].kind == OP_OVER){

      push(stack[stackCounter - 2]);

    }else if (ops.data[i].kind == OP_SWAP){

      int n1 = pop();
      int n2 = pop();

      push(n1);
      push(n2);

    }else if (ops.data[i].kind == OP_ROT){

      int n1 = pop();
      int n2 = pop();
      int n3 = pop();

      push(n2);
      push(n1);
      push(n3);

    }else if (ops.data[i].kind == OP_DROP){
      pop();
    }
    //
    //    Keywords
    //
    else if (ops.data[i].kind == OP_IF) {

      if (!pop()) i = ops.data[i].as.jump;

    } else if (ops.data[i].kind == OP_DO) {

      if (!pop()) i = ops.data[i].as.jump;

    } else if (ops.data[i].kind == OP_END){

       // if End belongs to a While jump to it
       if (ops.data[ops.data[i].as.jump].kind == OP_WHILE){
        i = ops.data[i].as.jump;
        continue;
       }

       // if End belongs to a If just continue

    } else if (ops.data[i].kind == OP_PRINT) {

      printf("%d\n", peek());

    } else if (ops.data[i].kind == OP_EXIT){
      
      printf("EXIT\n");
      break;

    }

#if DEBUG
    printf("stack: [");
    for (int i = 0; i < 10; ++i) {
      printf("%d, ", stack[i]);
    }
    printf("] stackCounter: %d\n", stackCounter);
    #endif

    ++i;
  }

   return 0;
}
