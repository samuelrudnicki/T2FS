#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/disk.h"
#include "../include/t2fs.h"
#include "../include/apidisk.h"

//gcc -o testePath testPath.c ../src/disk.c ../lib/apidisk.o -Wall -ggdb && ./testePath
//gcc -m32 -o testePath testPath.c ../src/disk.c ../src/t2fs.c ../lib/apidisk.o -Wall -ggdb && ./testePath

void printFAT(int sector) {
    int j;
    unsigned char buffer[SECTOR_SIZE];
    printf("\n");
    read_sector(superBlock.pFATSectorStart + sector,buffer);
    for (j = 0; j < 256; j++){
        printf("%x ",buffer[j]);
    }
    printf("\n");
}

void printDataSector(int clusterNo) {
    int j;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    readCluster(clusterNo, buffer);
    printf("\n");
    for (j = 0; j < 1024; j++){
        printf("%x ",buffer[j]);
    }
    printf("\n");
}
void printDataCluster(int clusterNo) {
    int j;
    int clusterByteSize = sizeof(unsigned char)*SECTOR_SIZE*superBlock.SectorsPerCluster;
    unsigned char* buffer = malloc(clusterByteSize);
    buffer = readDataCluster(clusterNo);
    printf("\n");
    for (j = 0; j < 1024; j++){
        printf("%c",buffer[j]);
    }
    printf("\n");

}

void printFolders(int clusterNo) {
    int i;
    int folderSize = ( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) );
    struct t2fs_record* folderContent = malloc(sizeof(struct t2fs_record)*( (SECTOR_SIZE*superBlock.SectorsPerCluster) / sizeof(struct t2fs_record) ));
    folderContent = readDataClusterFolder(clusterNo);
    for(i = 0; i < folderSize; i++) {
        printf("\nTYPEVAL: %x\n", folderContent[i].TypeVal);
        printf("NAME: %s\n", folderContent[i].name);
        printf("BYTESFILESIZE: %x\n", folderContent[i].bytesFileSize);
        printf("CLUSTERSFILESIZE: %x\n", folderContent[i].clustersFileSize);
        printf("FIRSTCLUSTER: %x\n", folderContent[i].firstCluster);
    }
}


int main() {
    int i;
    int cluster;
    char** testeToken;
    int testeTokenSize;
    char * testeAbsolute;
    char * testeLink;
    init_disk();

    printFolders(2);

    printf("\n\n****INICIANDO NA PASTA ROOT****\n\n");

    printf("\nRetorno do pathToCluster './file1.txt': %d\n", pathToCluster("./file1.txt"));


    printf("\nRetorno do pathToCluster '/dir1': %d\n", pathToCluster("/dir1"));

    printFolders(5);

    printf("\nRetorno do pathToCluster './dir1/file1.txt': %d\n", pathToCluster("./dir1/file1.txt"));

    printf("\nRetorno do pathToCluster './file1.txt/..': %d\n", pathToCluster("./file1.txt/.."));

    printf("\nRetorno do pathToCluster './dir1/file3.txt': %d\n", pathToCluster("./dir1/file3.txt"));

    printFAT(0);
    if(findFATOpenCluster(&cluster) != 0) {
        printf("\nERRO!\n");
    }

    printf("\nCluster disponivel na FAT: %d\n", cluster);

    writeInFAT(11,END_OF_FILE);

    printf("\nEscrevendo EOF no cluster 11\n");

    if(findFATOpenCluster(&cluster) != 0) {
        printf("\nERRO!\n");
    }

    printf("\nCluster disponivel na FAT: %d\n", cluster);

    writeInFAT(11,0);

    printf("\nVoltando a zero no cluster 11\n");

    testeTokenSize = tokenizePath("./dir1/dir2/../dir3///////////", &testeToken);

    for(i = 0; i < testeTokenSize; i++) {
        printf("\n%s\n", testeToken[i]);
    }


    toAbsolutePath("../b/c/d/e/f/g/h/i/../j/./k///","/aaa/bbb",&testeAbsolute);

    printf("\nTESTE ABSOLUTE: %s\n", testeAbsolute);

    printf("\n***************TESTE separatePath***************");

    char * saidaUm;
    char * saidaDois;
    separatePath("/ddd/ccc/aaaaa", &saidaUm, &saidaDois);

    printf("\nSaida Um: %s\nTamanho da saida um: %d", saidaUm, strlen(saidaUm));
    printf("\n\nSaida Dois: %s\nTamanho da saida dois: %d", saidaDois, strlen(saidaDois));
    printf("\n");

    printf("\n***************TESTE chdir2***************");

    printf("\nDiretorio atual: %s\n", currentPath.absolute);
    int mediz;
    mediz=chdir2("./dir1/file1.txt");
    printf("\nAlterando para:'./dir1/file1.txt' %d\n",mediz);
    printf("Diretorio alterado: %s\nCluster atual: %d\n", currentPath.absolute, currentPath.clusterNo);
    
    printf("\nAlterando para:'.././aa/b/../cb'\n");    
    chdir2(".././aa/b/../cb");
    printf("Diretorio alterado, porem inexistente: %s\n", currentPath.absolute);

    printf("\nVoltando para o raiz:\n");
    chdir2("../../");
    printf("Diretorio alterado de volta para a raiz: %s\nCluster atual: %d\n", currentPath.absolute, currentPath.clusterNo);

    printf("\n\n***************TESTE MakeDir***************\n");

    printf("\n*****Fazendo o direito 'abra' na Root.\n");
    mkdir2("../abra");
    printf("***Folders da ROOT:\n");
    printFolders(2);
    printf("\n***Mudando para o diretorio recem criado '/abra'\n");
    chdir2("/abra");
    printf("Folders do direitorio '/abra':\n");
    printFolders(currentPath.clusterNo);
    chdir2("../");
    printf("\n***Deletando o diretorio recem criado '/abra'\n");
    printFAT(0);
    rmdir2("./abra");
    printf("***Folders da ROOT:\n");
    printFolders(currentPath.clusterNo);
    printf("***Folde do diretorio que foi deletado '/abra':\n");
    printFolders(11);
    printFAT(0);

    printf("\n\n*****Fazendo o direito 'abra2' na Dir1.\n");
    mkdir2("../dir1/abra2");
    chdir2("../dir1");
    printf("\n**Folders do diretorio '/dir1':");
    printFolders(currentPath.clusterNo);
    printf("\n\n**Folders do direitorio Abra2 criado no /dir1:");
    chdir2("./abra2");
    printFolders(currentPath.clusterNo);

    printf("\n\n\n**Criando um Diretorio 'abra3' na Raiz:\n");
    mkdir2("../../abra3");
    printf("\n**Folders da raiz:");
    chdir2("../../");
    printFolders(currentPath.clusterNo);
    printf("\n**Folders do direitorio Abra3:");
    chdir2("./abra3");
    printFolders(currentPath.clusterNo);
    chdir2("../");

    printf("\n\n***Deletando o diretorio recem criado '/abra3 da Raiz'\n");
    rmdir2("./abra3");
    printf("\n***Folders da Raiz:\n");
    printFolders(currentPath.clusterNo);

    link("/link1",&testeLink);
    printf("\n%s\n",testeLink);

    return 0;
}