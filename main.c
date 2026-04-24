#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>

#define DEBUG 0
 
#define push(val) stack[stackCounter++] = (val)
#define pop() stack[--stackCounter]
#define peek() stack[stackCounter - 1]
 

typedef enum {
  OP_NIL,
  OP_NUM,
  OP_IDENTIFIER,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,

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

  OP_STORE,
  OP_LOAD,
  OP_STORE8,
  OP_LOAD8,
  OP_PTR,

  OP_IF,
  OP_ELSE,
  OP_WHILE,
  OP_DO,
  OP_END,

  OP_DUMP,
  OP_EXIT,
  OP_SYSCALL,
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

  { "store", OP_STORE},
  { "load", OP_LOAD},
  { "store8", OP_STORE8},
  { "load8", OP_LOAD8},
  { "ptr", OP_PTR},

  { "if", OP_IF },
  { "else", OP_ELSE },
  { "while", OP_WHILE },
  { "do" , OP_DO},
  { "end", OP_END },

  { "dump", OP_DUMP},
  { "exit", OP_EXIT },
  { "syscall", OP_SYSCALL},

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

      /*
      if (op.kind == OP_NIL){
        op.kind = OP_IDENTIFIER;
        memcpy(op.as.identifier, start, len);

      }
      */
      
      append_OpDA(ops, op);
      continue;
      
    }

    if (*src == '+') append_OpDA(ops, (Op){OP_ADD});
    if (*src == '-') append_OpDA(ops, (Op){OP_SUB});
    if (*src == '*') append_OpDA(ops, (Op){OP_MUL});
    if (*src == '/') {
      src++;
      if (*src == '/'){
        while (*src != '\n') src++;
      }else{
        append_OpDA(ops, (Op){OP_DIV});
      }
      continue;

    }
    if (*src == '%') append_OpDA(ops, (Op){OP_MOD});
    if (*src == '=') {
      src++;
      if (*src == '=') {
        append_OpDA(ops, (Op){OP_EE});
      }else{
        append_OpDA(ops, (Op){OP_EQ});
      }
      continue;
    }
    if (*src == '!'){
      src++;
      if (*src == '='){
        append_OpDA(ops, (Op){OP_NE});
      }else{  
        // i could introduce Not operation
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

  int stack[1024]; //stack for if/while addrs 
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

      }else {

        ops->data[i].as.jump = top; // End know where to jump back to
        ops->data[top].as.jump = i; // other stuff know where to jump forward

      }
    }else if (ops->data[i].kind == OP_ELSE){
      int ifIdx = pop();
      ops->data[ifIdx].as.jump = i; // if jumps to else 

      push(i);

    }
  }
}

int doSyscall(int num, int64_t *args, int64_t argc) {
    switch (argc) {
        case 0: return syscall(num);
        case 1: return syscall(num, args[0]);
        case 2: return syscall(num, args[0], args[1]);
        case 3: return syscall(num, args[0], args[1], args[2]);
        case 4: return syscall(num, args[0], args[1], args[2], args[3]);
        case 5: return syscall(num, args[0], args[1], args[2], args[3], args[4]);
        case 6: return syscall(num, args[0], args[1], args[2], args[3], args[4], args[5]);
        default:
            return -1;
    }
}

int main(int argc, char *argv[]) {

  if (argc != 2){
    printf("Usage: %s <name>.vctr\n", argv[0]);
    exit(1);
  }

  char *buf = readFile(argv[1]);
  Op_DA ops = {0};
  init_OpDA(&ops);
  lexer(buf, &ops);  


  uint8_t *heap = malloc(sizeof(uint8_t) * 1024);

  int64_t stack[256];
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

    } else if (ops.data[i].kind == OP_MOD){
      
      int n1 = pop();
      int n2 = pop();

      push(n1 % n2);
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
    //    Memory
    // 
    else if (ops.data[i].kind == OP_STORE){
      
      int addr = pop();
      int value = pop();

      heap[addr] = value;

    }else if (ops.data[i].kind == OP_LOAD){
      
      int addr = pop();
      push(heap[addr]);

    }else if (ops.data[i].kind == OP_STORE8){
      
      int64_t addr = pop();
      int64_t value = pop();
      memcpy(&heap[addr], &value, sizeof(int64_t));


    }else if (ops.data[i].kind == OP_LOAD8){
      
      int64_t addr = pop();
      int64_t value;
      memcpy(&value, &heap[addr], sizeof(int64_t));
      push(value);

    }else if (ops.data[i].kind == OP_PTR){

      int addr = pop();
      push((int64_t)&heap[addr]);

    }
    //
    //    Control flow
    //
    else if (ops.data[i].kind == OP_IF) {

      if (!pop()) i = ops.data[i].as.jump;

    } else if (ops.data[i].kind == OP_ELSE){

      i = ops.data[i].as.jump;

    } else if (ops.data[i].kind == OP_DO) {

      if (!pop()) i = ops.data[i].as.jump;

    } else if (ops.data[i].kind == OP_END){

       // if End belongs to a While jump to it
       if (ops.data[ops.data[i].as.jump].kind == OP_WHILE){
        i = ops.data[i].as.jump;
        continue;
       }

       // if End belongs to a If/Else just continue so we skip over end.jump

    }
    //
    // I/O
    //
    else if (ops.data[i].kind == OP_DUMP) {

      printf("%ld\n", peek());

    } else if (ops.data[i].kind == OP_EXIT){
      
      printf("EXIT\n");
      break;

    } else if (ops.data[i].kind == OP_SYSCALL){

      int64_t argc = pop();
      int64_t args[6];

      for (int i = argc - 1; i >= 0; --i){
        args[i] = pop();
      }

      int num = pop();

      push(doSyscall(num, args, argc));      
      
    }

#if DEBUG
    printf("OP: %d\n", ops.data[i].kind);
    printf("stack: [");
    for (int i = 0; i < 10; ++i) {
      printf("%ld, ", stack[i]);
    }
    printf("] stackCounter: %d\n", stackCounter);
    #endif

    ++i;
  }

   return 0;
}
