import serial, time
import csv, os
from pydrive2.auth import GoogleAuth, RefreshError
from pydrive2.drive import GoogleDrive

ANALYSIS = '1'
FAIL     = '0'

def saveToDrive(filename, newline, dir):
    name = ''
    gauth = GoogleAuth()    
    gauth.LoadCredentialsFile("gdrive-credentials.json")
    try: 
        if gauth.credentials is None:
                gauth.LocalWebserverAuth()
        elif gauth.access_token_expired:
            try:
                gauth.Refresh()
                return f'Error 1 {e}' 
            except RefreshError:
                os.remove("gdrive-credentials.json")
                gauth.LoadCredentialsFile("gdrive-credentials.json")
                gauth.LocalWebserverAuth()
        else:
            gauth.Authorize()

        # Save the current credentials to a file
        gauth.SaveCredentialsFile("gdrive-credentials.json")
    except Exception as e:
        return f'Error 1 {e}' 
    try:
        drive = GoogleDrive(gauth)
        file_list = drive.ListFile({'q': "'1LibwIkSsgjNK4ZPrHh56Z5pfbI1_fyR_' in parents and trashed=false"}).GetList()
        all_tests = 0
        this_test = 0
        for file in file_list:
            if file['title'] == 'AllTests.csv' and all_tests == 0:
                all_tests = 1
                new_file = drive.CreateFile({'title': file['title'], 'id': file['id']})
                file_content = new_file.GetContentString()
                new_content = f'{file_content}\n{",".join(newline)}'
                new_file.SetContentString(new_content)
                new_file.Upload()  
                try: 
                    os.remove(f'{dir}Reportes/AllTests.csv')
                except: 
                    pass
                file.GetContentFile(file['title'])

            elif filename[:filename.find('_')] in file['title']:
                this_test += 1
        name = createNewVersion(drive, f'{filename}', this_test)
        if all_tests == 0: 
            print("No master file")
            createMasterFile(drive, newline, dir)    

    except Exception as e: 
        return f'Error {e}'
    return name

def createNewVersion(drive, filename, version):
    if version != 0:
        try: 
            os.rename(f'{filename}.csv', f'{filename} ({version}).csv')
            file = drive.CreateFile({"mimeType": "text/csv", "parents": [{"id": "1LibwIkSsgjNK4ZPrHh56Z5pfbI1_fyR_"}]})
            file.SetContentFile(f'{filename} ({version}).csv')
            file.Upload()
            return f'{filename} ({version})'
        except Exception as e: 
            return f'Error version 0 {e}'
    else: 
        try: 
            file = drive.CreateFile({"mimeType": "text/csv", "parents": [{"id": "1LibwIkSsgjNK4ZPrHh56Z5pfbI1_fyR_"}]})
            file.SetContentFile(f'{filename}.csv')
            file.Upload()
            # os.rename(f'{filename}.csv', f'{dir}/{filename}.csv')
            return filename
        except Exception as e: 
            return f'Error version 1 {e}'
    
def createMasterFile(drive, newline, dir):
    # testpoints = []
    headers = ['Fecha', 'N de serie']
    with open(f'{dir}default_values.csv', encoding='utf-8') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if row[0] == '':
                break
            # testpoints.append(row[0])
            headers = [*headers, *['Tolerancia', 'Min', 'Max', 'Real']]    
    
    if not os.path.exists(f"{dir}AllTests.csv"):
        with open('AllTests.csv', 'a') as f_object:
            writer_object = csv.writer(f_object)
            writer_object.writerow(headers)
            f_object.close()
    with open('AllTests.csv', 'a') as f_object:
        writer_object = csv.writer(f_object)
        writer_object.writerow(newline)
        f_object.close()
    
    file = drive.CreateFile({"mimeType": "text/csv", "parents": [{"id": "1LibwIkSsgjNK4ZPrHh56Z5pfbI1_fyR_"}]})
    file.SetContentFile('AllTests.csv')
    file.Upload()

def initSerialCom(num = 'COM6'):
    global ser
    ser = serial.Serial()
    ser.port = num
    ser.baudrate = 230400
    ser.timeout = 1          #non-block read
    ser.xonxoff = False      #disable software flow control
    ser.rtscts = True        #enable hardware (RTS/CTS) flow control
    ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
    ser.writeTimeout = 2     #timeout for write

def close_serial_port():
    ser.close()  
    
def readFES(tols):
    try:
        ser.open()

    except serial.serialutil.SerialException:
        ser.close()  
        print("Could not open Serial Port")
        return 0, "Error: No se pudo abrir puerto serial"

    except:
        ser.close()  
        return 0, "Error: Conectar ToolFES"

    if ser.isOpen():
        try:
            ser.flushInput() #flush input buffer, discarding all its contents
            ser.flushOutput()#flush output buffer, aborting current output 
            final_response = []

            # Connection
            if "!" in tols: 
                state = "FES"
                ser.write(bytes(tols, 'utf-8'))
                if tols == "!!\n":
                    for i in range(5):
                        time.sleep(0.4)
                        response = ser.readline().split()
                        if len(response) == 3 and response not in final_response:
                            final_response.append(response)
                elif tols == "!_\n":
                    while True: 
                        response = ser.readline().split()
                        if len(response) == 0:
                            ser.close()
                            return state, "FES desconectado"
                else: 
                    time.sleep(2)
                    response = ser.readline().split()
                    if len(response) > 0:
                        ser.close()
                        return state, response
                    else: 
                        return state, "Error: Conexión a FES falló"
            # Settings
            elif tols[0] == 'Z':
                for i in range(len(tols)):
                    ser.write(bytes(tols[i], 'utf-8'))
                    time.sleep(0.01)
                ser.close() 
                return FAIL, []  

            elif tols[0] == 'T': 
                ser.write(bytes(tols[0], 'utf-8'))
                for i in range(1, len(tols)-1):
                    for j in range(len(tols[i])):
                        ser.write(bytes(tols[i][j], 'utf-8'))
                        time.sleep(0.01)
                ser.write(bytes(tols[-1], 'utf-8'))
                ser.close() 
                return FAIL, []  
            # Calibration
            elif tols[0] == 'V' or tols[0] == 'V\n':
                ser.write(bytes(tols[0], 'utf-8'))
                for i in range(20): 
                    response = ser.readline().split()
                    if len(response) > 0:
                        ser.close()
                        return [], f'{response[0].decode("utf-8")} {response[1].decode("utf-8")}'
                return [], "Error: calibration failed"
            
            # Set vrefs
            elif 'm' in tols[0] or 'n' in tols[0]:
                ser.write(bytes(tols, 'utf-8'))
                response = ser.readline().split()
                time.sleep(0.01)
                ser.close()
                return 
                
            # Testpoints
            else: 
                state = ANALYSIS
                i = 0
                p = 0
                while i < len(tols):
                    time.sleep(0.01)
                    ser.write(bytes(tols[i], 'utf-8'))
                    response = ser.readline().split()
                    # print(response)
                    if tols[i] in ['S', 'S\n', 'X', 'X\n', 'W', 'W\n', 'V', 'v']:
                        time.sleep(0.5)
                        i += 1
                    elif tols[i] == 'N' or tols[i] == 'N\n':
                        final_response.append(response)
                        ser.close()
                        return state, final_response
                    elif response == []:
                        ser.close()
                        return FAIL, "Error: Falló conexión a ToolFES"
                    elif "TP" in response[0].decode("utf-8"):
                        if len(response) > 2:
                            if any(string in response[2].decode("utf-8") for string in ['M', 'B', "SB", "F"]) or "F" in response[1].decode("utf-8") or p > 10:
                                final_response.append(response)
                                i += 1
                            p += 1
                        else:
                            p += 1
            ser.close()
            return state, final_response

        except Exception as e1:
            print("error communicating...: " + str(e1))

    else:
        print("No se pudo abrir el puerto")

