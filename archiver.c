// gcc archiver.c -o archiver -lm -std=c99
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
/**
* Gerenciador de Archives
* @author Edson da Costa Vitor Junior
* @author Guilherme Atihe de Oliveira
* @author Gustavo Batistic Ribeiro
* @author Matheus Peron Baroni
*/
// Extrai um arquivo do archive sem remove-lo
int extract(char *archiveName, char *fileName) {
  FILE *archive, *file;
  char *archiveFileName = malloc(strlen(archiveName)+strlen(".arc")+1);
  strcpy(archiveFileName, archiveName);
  strcat(archiveFileName, ".arc");

  int iT, iF = 512, length;
  unsigned short bb;
  unsigned char b1, b2;
  int a, x;
  char *n;

  file = fopen(fileName, "wb+");
  if (file == NULL) {
    puts("Impossivel abrir arquivo.");
    exit(1);
  }

  archive = fopen(archiveFileName, "rb+");
  if (archive == NULL) {
    puts("Impossivel abrir archiver.");
    fclose(file);
    exit(1);
  }
  // Verifica se existem arquivos gravados
  fseek(archive, 6, SEEK_SET);
  a = fgetc(archive);
  if(a == 0) 
    return 0;
  a = fgetc(archive);
  while (a > 0) {
    // Caso o arquivo tenha sido excluido
    x = fgetc(archive);
    if(x == 1) {
      b1 = fgetc(archive);
      b2 = fgetc(archive);
      a = fgetc(archive);
      continue;
    }

    b1 = fgetc(archive);
    b2 = fgetc(archive);
    bb = b1 | b2 << 8;
    iT = ftell(archive);
    // Checar se o nome do arquivo corresponde com o arquivo que sera extraido
    fseek(archive, iF, SEEK_SET);
    a = fgetc(archive);
    n = calloc(a, sizeof(char *));
    for (int i = 0; i < a; ++i)
       n[i] = fgetc(archive);
    if (strcmp(n,fileName) == 0) {
      length = bb*512;
      // Copiar arquivo
      for (int i = 0; i < length; ++i) {
        a = fgetc(archive);
        fputc(a, file);
      }
      break;
    }
    free(n);
    fseek(archive, iT, SEEK_SET);
    a = fgetc(archive);
    iF += bb*512;
  }
  fflush(file);
  fclose(file);
  fclose(archive);
  return 0;
}

// Listar arquivos presentes no archive
int list(char *name) {
  FILE *archive;
  char *fileName = malloc(strlen(name)+strlen(".arc")+1);
  strcpy(fileName, name);
  strcat(fileName, ".arc");

  int iT, iF = 512;
  unsigned short bb;
  unsigned char b1, b2;
  int a, x;

  archive = fopen(fileName, "rb+");
  if (archive == NULL) {
    puts("Impossivel abrir archiver.");
    fclose(archive);
    exit(1);
  }

  printf("Lista de Arquivos presentes\n");
  // Verifica se existem arquivos gravados
  fseek(archive, 6, SEEK_CUR);
  a = fgetc(archive);
  printf("Numero de arquivos: %d\n", a);
  if(a == 0) 
    return 0;
  a = fgetc(archive);
  while (a > 0) {
    x = fgetc(archive);
    // Caso o arquivo nao tenha sido excluido
    if(x == 0) {
      printf("Arquivo %d: ", a);
      b1 = fgetc(archive);
      b2 = fgetc(archive);
      bb = b1 | b2 << 8;
      iT = ftell(archive);
      // Obter nome do arquivo
      fseek(archive, iF, SEEK_SET);
      a = fgetc(archive);
      for (int i = 0; i < a; ++i)
        printf("%c", fgetc(archive));
      printf("\n");
      fseek(archive, iT, SEEK_SET);
      a = fgetc(archive);
      iF += bb*512;
    // Caso o arquivo tenha sido excluido
    } else {
      // Pular para o proximo
      a = fgetc(archive);
      a = fgetc(archive);
      a = fgetc(archive);
    }
  }
  fclose(archive);
  return 0;
}

// Remove arquivo do archive
int subtract(char *archiveName, char *fileName) {
  FILE *archive, *tmp;
  char *archiveFileName = malloc(strlen(archiveName)+strlen(".arc")+1);
  strcpy(archiveFileName, archiveName);
  strcat(archiveFileName, ".arc");

  int iT, iF = 512, iX, iM, length;
  unsigned short bb;
  unsigned short s = 0; 
  unsigned char b1, b2;
  int a, x, count = 0;
  char *n;

  int o;

  archive = fopen(archiveFileName, "rb+");
  if (archive == NULL) {
    puts("Impossivel abrir archiver.");
    fclose(archive);
    exit(1);
  }

  tmp = fopen("temp.bin", "wb+");
  if (archive == NULL) {
    puts("Impossivel abrir arquivo temporario.");
    fclose(tmp);
    exit(1);
  }

  // Verifica se existem arquivos gravados
  fseek(archive, 6, SEEK_SET);
  a = fgetc(archive);
  if(a == 0) 
    return 0;

  a = fgetc(archive);
  while (a > 0) {
    // Pular itens ja excluidos
    x = fgetc(archive);
    if(x == 1) {
      b1 = fgetc(archive);
      b2 = fgetc(archive);
      a = fgetc(archive);
      continue;
    }

    // Guardar tamanho do arquivo
    b1 = fgetc(archive);
    b2 = fgetc(archive);
    bb = b1 | b2 << 8;

    iT = ftell(archive);
    // Pular pro inicio do arquivo
    fseek(archive, iF, SEEK_SET);
    // Leitura do nome do arquivo
    a = fgetc(archive);
    n = malloc(sizeof(char)*a);
    for (int i = 0; i < a; ++i)
      n[i] = fgetc(archive);

    if (strcmp(n, fileName) == 0) {
      iX = iF;
      iF += bb*512; // Comeco do proximo arq.
      length = bb*512;
      // Marcar arquivo como excluido
      fseek(archive, (iT-3), SEEK_SET);
      fputc(1, archive);
      // Decrementar contador de arquivos
      fseek(archive, 6, SEEK_SET);
      x = fgetc(archive);
      fseek(archive, -1, SEEK_CUR);
      fputc(x-1, archive);
      // Armazenar tamanho do archive
      fseek(archive, 0, SEEK_END);
      iM = ftell(archive);
      fseek(archive, 0, SEEK_SET);
      // Tramsferir conteudo para arquivo temporario
      for (int i = 0; i < iM; ++i) {
        a = fgetc(archive);
        x = ftell(archive);
        // Pular conteudo do arquivo que foi marcado como excluido
        if( (x > iX) && (x <= (iX+length)) ){
          continue;
        }
        fputc(a, tmp);
      }
      break;
    } 
    fseek(archive, iT, SEEK_SET);
    a = fgetc(archive);
    iF += bb*512;
  }
  fclose(archive);
  fclose(tmp);
  // Substitui anterior pelo temporario
  remove(archiveFileName);
  rename("temp.bin", archiveFileName);
  return 0;
}

// Adiciona arquivo para o archiver
int add(char *archiveName, char *fileName) {
  FILE *archive, *file;
  char *archiveFileName = malloc(strlen(archiveName)+strlen(".arc")+1);
  strcpy(archiveFileName, archiveName);
  strcat(archiveFileName, ".arc");

  int a, count = 0;
  unsigned short bb;
  unsigned short s = 0; 
  unsigned char b1, b2;

  file = fopen(fileName, "rb");
  if (file == NULL) {
    puts("Impossivel abrir arquivo.");
    exit(1);
  }
  //Calcula tamanho necessario (multiplo de 512)
  fseek(file, 0L, SEEK_END);
  double sum = (ftell(file)+1+strlen(fileName));
  unsigned short fileSize = (unsigned short) ceil(sum/512);
  fclose(file);

  file = fopen(fileName, "rb");
  if (file == NULL) {
    puts("Impossivel abrir arquivo.");
    exit(1);
  }

  archive = fopen(archiveFileName, "rb+");
  if (archive == NULL) {
    puts("Impossivel abrir archiver.");
    fclose(file);
    exit(1);
  }
  // Incrementa no contador de arquivos gravados
  fseek(archive, 6, SEEK_SET);
  a = fgetc(archive);
  fseek(archive, -1, SEEK_CUR);
  fputc(a+1, archive);

  a = fgetc(archive);
  // Caso o arquivo a ser gravado seja o primeiro
  if(a == 0) {
    fseek(archive, -1, SEEK_CUR);
    fputc(1, archive);
    fputc(0, archive);
    b1 = *((unsigned char*)(&fileSize));
    fputc(b1, archive);
    b2 = *((unsigned char*)(&fileSize) + 1);
    fputc(b2, archive);
    fputc(0, archive);
    fseek(archive, 512, SEEK_SET);
  // Caso nao seja o primeiro
  } else {
    count = 1;
    do {
      count++;
      a = fgetc(archive);
      b1 = fgetc(archive);
      b2 = fgetc(archive);
      // Incrementa o tamanho do arquivo na contagem se o arquivo nao tenha sido excluido
      if(a == 0)
        s += b1 | b2 << 8;

      a = fgetc(archive);
    } while(a > 0);
    // Armazena os valores na tabela e pula para a posicao onde o arquivo sera gravado
    fseek(archive, -1, SEEK_CUR);
    fputc(count, archive);
    fputc(0, archive);
    b1 = *((unsigned char*)(&fileSize));
    fputc(b1, archive);
    b2 = *((unsigned char*)(&fileSize) + 1);
    fputc(b2, archive);
    fseek(archive, 0, SEEK_SET);
    fseek(archive, 512, SEEK_CUR);
    fseek(archive, s*512, SEEK_CUR);
  }
  // Arazena o nome do arquivo e seu tamanho
  fputc(strlen(fileName), archive);
  for (int i = 0; i < strlen(fileName); ++i)
    fputc(fileName[i], archive);
  // Copia o arquivo
  while ( (a = fgetc(file)) != EOF )
    fputc(a, archive);
  fclose(file);
  fclose(archive);
  return 1;
}

// Cria um novo arquivo junto do cabeçalho
int create(char *name) {
  char *fileName = malloc(strlen(name)+strlen(".arc")+1);
  strcpy(fileName, name);
  strcat(fileName, ".arc");

  FILE *file = fopen(fileName, "wb+");

  // Armazenar data de criacao do archive
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  fputc(tm.tm_year, file);
  fputc(tm.tm_mon + 1, file);
  fputc(tm.tm_mday, file);
  fputc(tm.tm_hour, file);
  fputc(tm.tm_min, file);
  fputc(tm.tm_sec, file);

  fputc(0, file); // Zerar numero de arquivos presentes
  fputc(0, file); // Zerar id do primeiro arquivo

  fclose(file);
  return 1;
}

void help() {
  printf("\e[1mGerenciador de Archives\nFeito para a disciplina ST562\e[0m\n");
  printf("\n\e[1mCriacao de archive:\e[0m\n");
  printf("\e[3m./archiver archive-name -c\e[0m\n\tCria novo archive.\n");
  printf("\e[3m./archiver archive-name -c file.ext\e[0m\n\tCria novo archive e adiciona um arquivo.\n");
  printf("\e[3m./archiver archive-name -c file1.ext file2.ext...\e[0m\n\tCria novo archive e adiciona varios arquivos dentro.\n");
  printf("\n\e[1mAdicao de arquivos:\e[0m\n");
  printf("\e[3m./archiver archive-name -a file.ext\e[0m\n\tAdiciona um arquivo ao archive.\n");
  printf("\e[3m./archiver archive-name -a file1.ext file2.ext...\e[0m\n\tAdiciona varios arquivos ao archive.\n");
  printf("\n\e[1mListagem de arquivos:\e[0m\n");
  printf("\e[3m./archiver archive-name -l\e[0m\n\tLista os arquivos presentes no archive.\n");
  printf("\n\e[1mExtracao de arquivos:\e[0m\n");
  printf("\e[3m./archiver archive-name -e file.ext\e[0m\n\tExtrai um arquivo do archive sem remove-lo.\n");
  printf("\e[3m./archiver archive-name -e file1.ext file2.ext...\e[0m\n\tExtrai varios arquivos do archive sem remove-lo.\n");
  printf("\n\e[1mRemocao de arquivos:\e[0m\n");
  printf("\e[3m./archiver archive-name -r file.ext\e[0m\n\tRemove um arquivo do archive.\n");
  printf("\e[3m./archiver archive-name -r file1.ext file2.ext...\e[0m\n\tRemove varios arquivos do archive.\n\n");
}

int main(int argc, char *argv[]) {
  if(argc < 3) {
    help();
    return 0;
  }
  switch(argv[2][1]) {
    case 'c':
      create(argv[1]);
      for (int i = 3; i < argc; ++i)
        add(argv[1], argv[i]);
      break;
    case 'a':
      for (int i = 3; i < argc; ++i)
        add(argv[1], argv[i]);
      break;
    case 'l':
      list(argv[1]);
      break;
    case 'e':
      for (int i = 3; i < argc; ++i)
        extract(argv[1], argv[i]);
      break;
    case 'r':
      for (int i = 3; i < argc; ++i)
        subtract(argv[1], argv[i]);
      break;
    default: help(); break;
  } return 0;
}
