/*
Lee una tarjeta sD sin formato y guarda dos archivos, uno con los datos en hex y otro en decimal.
Los datos en la sD vienen en 2 bytes: most significant byte (msb) y least significant byte (lsb).
El dato completo es la concatenacion de los dos.
Ej: (h = hexadecimal)
ADC10MEM = 10 1100 1101 = 717 = h2CD -> 10bits
msb = ADC10MEM>>2 = 10 1100 11 = hB3 -> 8bits
lsb =  01 = h01 -> 2bits
Leo los 2 bytes (en hex) y los guardo concatenados -> B301
Leo el string y lo separo -> B3, 01
Transformo en decimal, multiplico el msb por 4 (por los 2 bits que lo habia corrido) y los sumo.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 512*400 // Bytes a leer
// Datos = bytes/2
// 512 bytes = 1 sector = 256 bytes/canal = 128 datos/canal (10 bits por dato)
// A 8kHz/canal 1 sector = 16ms
// 400 sectores = 1 medicion = 6,4 segundos
// El 400 viene del programa de adquisicion del MSP430
int i;
char *can; //Para diferenciar si se grabo 1 canal o 2
char aux_sector[10];
// Read from one sector
int main(int argc, char *argv[]){
    char *letras;       // Numero de medicion
    letras = argv[1];   // Sector inicial
    can = "2";          // Cantidad de canales
    FILE *volume;       // Origen
    FILE *out_hex;      // Datos en hex
    FILE *out_dec;      // Datos en dec
    int k = 0;
    long long sector;
    char buf[BUFFER_SIZE] = {0};
    
    sprintf(aux_sector,"%s",letras);
    sector = atol(aux_sector);              // Numero de sector inicial
    //volume = fopen("\\\\.\\E:", "r");     // Leer directo de tarjeta
    //volume = fopen("ejemplo1.IMA", "r");  // Leer desde un archivo (simulacion)
    volume = fopen("output.raw", "r");      // Leer desde archivo raw
    setbuf(volume, NULL);                   // Disable buffering
    if(!volume){
        printf("Cant open Drive\n");
        return 1;
    }
    if(fseek(volume, sector*400*512, SEEK_SET) != 0){ // Sector-aligned offset
        printf("Can't move to sector\n");
        return 2;
    }
    // Lee BUFFER_SIZE cantidad de datos y los guarda en buf
    fread(buf, sizeof(*buf), BUFFER_SIZE, volume);
    fclose(volume);

    // A partir de aca es la conversión y guardado de datos
    out_hex = fopen("hex.dat","w");
    int lsb = 0; //El primer byte es msb
    
    // Guardo dos bits concatenados por fila: msb+lsb
    for(k=0;k<BUFFER_SIZE;k++){
        if(lsb){
            fprintf(out_hex,"%x\n",buf[k]);
            lsb = 0;
        }else{
            fprintf(out_hex,"%x",buf[k]);
            lsb = 1;
        }
    }
    fclose(out_hex);
    
    char aux[8] = {0};
    char aux_msb[2] = {0};
    long int valor_msb, valor_lsb, valor;
    char * pEnd;
    out_hex = fopen("hex.dat", "r");
    out_dec = fopen("dec.dat","w");
    int j = 1;
    for(k=1;k<(BUFFER_SIZE)/2+1;k++){
        fscanf(out_hex,"%s",aux);
        int len = strlen(aux);
        if(len>2){                      //medio tecnico, hay una diferencia en como escribe el hexadecimal que lee si es mayor o menor a 512
            char *aux_2 = &aux[len-3];  //Me quedo con los 3 digitos que importan, al ppio pone muchas f's
            strncpy(aux_msb,aux_2,2);   //Los dos primeros son los msb
            aux_msb[2] = '\0';
        }else{
            char *aux_2 = &aux[len-2];  //Me quedo con los 2 digitos que importan
            strncpy(aux_msb,aux_2,1);   //Los dos primeros son los msb
            aux_msb[1] = '\0';
        }
        char *aux_lsb = &aux[len-1];                    //El ultimo digito es el lsb
        valor_msb = pow(2,2)*strtol(aux_msb,&pEnd,16);  //Paso de hex a decimal y hago bitshift
        valor_lsb = strtol(aux_lsb,&pEnd,16);           //Paso de hex a decimal
        valor = valor_msb+valor_lsb;
        if(j){
            fprintf(out_dec,"%ld\t",valor);
            j = 0;
        }else{
            fprintf(out_dec,"%ld\n",valor);
            j = 1;
        }
    }
    fclose(out_hex);
    fclose(out_dec);
    return 0;
}