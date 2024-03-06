with open('sniffer-data.txt', 'r') as archivo:
    for linea in archivo:
        linea = linea.strip()
        header = int(linea[:2],16)
        if (header & 8) > 0:
            print (linea)
        
        
