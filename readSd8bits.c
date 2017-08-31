/*
Lee una tarjeta sD sin formato (datos en hex) y guarda dos archivos, uno con los datos en hex y otro en decimal.
Los datos en la sD vienen en 1 byte
Ej: (h = hexadecimal)
ADC10MEM = 10 1100 1101 = 717 = h2CD -> 10bits
msb = ADC10MEM>>2 = 10 1100 11 = hB3 -> 8bits
Leo 1 byte (en hex) y lo guardo -> B3
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 512*400 // Bytes a leer
// Datos = bytes
// 512 bytes = 1 sector = 256 bytes/canal = 256 datos/canal (8 bits por dato)
// A 16kHz/canal 1 sector = 16ms
// 400 sectores = 1 medicion = 6,4 segundos
int i;
char can[0]; //Para diferenciar si se grabo 1 canal o 2
char aux_sector[10];
// Read from one sector
int main(void){
  printf("1 canal = 1\n2 canales = 2\nCuantos canales se grabaron? ");
  scanf("%s",can);
  FILE *volume; //Origen
  FILE *out_hex; //Datos en hex
  FILE *out_dec; //Datos en dec
  int k = 0;
  long long sector;
  char buf[BUFFER_SIZE] = {0};
    
  //for(sector = 0;sector<1;sector++){ //No hace falta este loop
    printf("1 sector = 512 bytes = 512 datos = 16 ms (dos canales) o 32 ms (un canal) \n1 minuto = 3750 sectores, 1 hora = 225000 sectores\nLeer a partir del sector: ");
    scanf("%s",aux_sector);
    sector = atol(aux_sector);
    //volume = fopen("\\\\.\\E:", "r");
    volume = fopen("ejemplo1.IMA", "r");
    setbuf(volume, NULL);       // Disable buffering
    if(!volume){
      printf("Cant open Drive\n");
      return 1;
    }
    if(fseek(volume, sector*512, SEEK_SET) != 0){ // Sector-aligned offset
      printf("Can't move to sector\n");
      return 2;
    }
    // read what is in sector and put in buf //
    fread(buf, sizeof(*buf), BUFFER_SIZE, volume);
    fclose(volume);
    out_hex = fopen("hex.dat","w");
    // guardo el dato hex en archivo //

    for(k=0;k<BUFFER_SIZE+1;k++){
    	fprintf(out_hex,"%x\n",buf[k]);
    }
    fclose(out_hex);
    
    char aux[8] = {0};
    char aux_msb[2] = {0};
    long int valor_msb, valor_lsb, valor;
    char * pEnd;
    out_hex = fopen("hex.dat", "r");
    out_dec = fopen("dec.dat","w");
    int j = 1;
    for(k=1;k<(BUFFER_SIZE+1)/2;k++){
      fscanf(out_hex,"%s",aux);
      int len=strlen(aux);
      if(len>1){ //medio tecnico, hay una diferencia en como escribe el hexadecimal que lee si es mayor o menor a 512
        char *aux_2 = &aux[len-2]; //Me quedo con los 3 digitos que importan, al ppio pone muchas f's
        strncpy(aux_msb,aux_2,2); //Los dos primeros son los msb
        aux_msb[2] = '\0';
      }else{
        char *aux_2 = &aux[len-1]; //Me quedo con los 2 digitos que importan
        strncpy(aux_msb,aux_2,1); //Los dos primeros son los msb
        //aux_msb[2] = '\0';
        aux_msb[1] = '\0';
      }
      valor = strtol(aux_msb,&pEnd,16); //Paso de hex a decimal
      if(can[0] == '1'){ // Si 1 canal escribo 1 columna
        fprintf(out_dec,"%ld\n",valor);
      }else{ // Si dos canales los escribo en columnas distintas
        if(j){
          fprintf(out_dec,"%ld\t",valor);
          j=0;
        }else{
          fprintf(out_dec,"%ld\n",valor);
          j=1;
        }
      }
    }
    fclose(out_hex);
    fclose(out_dec);
    
    return 0;
}
