# -*- coding: utf-8 -*-
"""
Created on Wed Sep 14 18:09:21 2016

Llama al programa de lectura de la tarjeta sd, de manera de levantar los datos.

Antes de correr se deben modificar:
    - ave: nombre identificativo del ave
    - año, dia, mes, hour, minuto: Hora de inicio de la medicion
    - ttot: tiempo total de medicion. Por defecto 7 horas.
    - dt_adq,tmed: SOLO SI SE HA MODIFICADO EL PROGRAMA DE GRABADO DE DATOS

@author: juan
"""

import glob
from subprocess import call
import time
import sys
from scipy.io import wavfile
from datetime import datetime, timedelta

os.chdir('/home/juan/Documentos/Campo/Data/')
folder = os.getcwd() #Nombre de la carpeta donde esta el .py

ave = 'Test'    # Nombre del ave
side = 'left'   # Lado de insercion electrodos
año = 2017      # Datos de momento de inicio de la medicion
mes = 8
dia = 31
hour = 13
minuto = 26
segundos = 00
time_obj = datetime(int(año), int(mes), int(dia), int(hour), int(minuto), int(segundos))

fecha = time_obj.strftime("%d-%m-%Y")
hora = time_obj.strftime("%H:%M:%S")

inicio = fecha+' '+hora
bits = '10'
canales = 'músculo y sonido'

dt_adq = 62*10**-6              # Intervalo entre mediciones. Modificar unicamente
                                # si se ha modificado el programa de MSP (TACCR0)

frec_adq = 1/(2*dt_adq)         # Frecuencia de adquisición (2 canales)

tmed  = 400.0 * 128.0 /frec_adq # Duracion de cada medicion:
                                # Se miden 2 canales -> 256 bytes por sector para c/u
                                # Cada med = 10bits -> 2 bytes -> 128 mediciones por canal por sector
                                # Cada medicion es de 400 sectores -> 400*128 = #total de mediciones por canal
ttot = 10*60                       # Tiempo total de medicion en segundos
fin = time_obj + timedelta(seconds = ttot)
cantidad = int(ttot/tmed)       # Cantidad de mediciones. Este valor se debe modificar. IMPORTANTE

programa = 'read'               # Programa compilado a partir de readSd.c
destino_raw = os.path.join(folder,ave + '_' + fecha + '_' + hora + '-raw')
destino_wavs = os.path.join(folder,ave + '_' + fecha + '_' + hora + '-wavs')

try:
    os.mkdir(destino_raw)
    os.mkdir(destino_wavs)
except:
    # Si el destino ya existe puede que se corra peligro de sobreescribir datos
    end = input('Alguno de los directorios de destino ya existe. Continuar? [s/n]\n')
    if end == 'n':
        sys.exit(0)

#Metadata
meta = open('metadata.txt','a')
meta.write('DATOS DE LA GRABACIÓN\nAve: '+ave+'\nGrabación continua de '+ canales +
', canales a '+ str(int(frec_adq)) +' por canal, con resolución de '+ bits + ' bits\n')
meta.write('\nCada medición dura '+ str(tmed)[:4] +' segundos\nInicio: '+inicio)
meta.write('\nTiempo total de medición: '+ str(ttot) +' segundos = ' + str(ttot/3600)[:4] + ' horas.')
meta.write('\nNúmero de mediciones: ' + str(cantidad))
meta.write('\nHora fin:'+ fin.strftime("%d-%m-%Y %H:%M:%S"))
meta.close()
os.rename('metadata.txt', destino_raw+'/metadata.txt')

time = datetime(int(año), int(mes), int(dia), int(hour), int(minuto), int(segundos))
dp = 10
porcentaje = dp
for j in range(cantidad):
    completion = j/cantidad*100
    if completion > porcentaje:
        print('%.0f%% completado ( = %.0f mediciones)' %tuple((porcentaje,j)))
        porcentaje += dp
    # Ejecuta el programa de lectura de 1 medicion (ie 400 sectores). La salida
    # del programa es un file dec.dat con los valores de los dos canales
    call(["./"+programa, str(int(j))]) #Agregar arg al programa?
    
    dat_file = glob.glob(os.path.join(folder,'dec.dat'))[0]
    datos = np.loadtxt('dec.dat')
    sonido = [x[0] for x in datos]
    vs = [x[1] for x in datos]
    date = time.strftime("%d-%m-%Y")
    hms = time.strftime("%H:%M:%S")
    wavname_s = ave + '_' + date + '_' + hms + '_sonido.wav'
    wavname_vs = ave + '_' + date + '_' + hms + '_vs' + side + '.wav'
    # Los guardo como vienen, SIN NORMALIZAR
    wavfile.write(os.path.join(destino_wavs,wavname_s),int(frec_adq),np.asarray(sonido-mean(sonido), dtype = np.int16))
    wavfile.write(os.path.join(destino_wavs,wavname_vs),int(frec_adq),np.asarray(vs-mean(vs), dtype = np.int16))
    
    if len(dat_file) > 2:
        # Paranoiqueo que pasa si no encuentra dec.dat
        os.rename(dat_file, destino_raw + '/medicion'+str(j))
    
    time = time + timedelta(seconds = tmed)
    