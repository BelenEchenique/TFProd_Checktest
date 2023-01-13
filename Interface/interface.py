# Para crear .exe :  pyinstaller -F --noconsole interface.py
# Para crear .exe :  pyinstaller --noconsole --noconfirm interface.py --log-level=DEBUG
# Para actualizar .exe :  pyinstaller --noconfirm interface.spec 

# --hidden-import=sys 
# --hidden-import=datetime.datetime 
# --hidden-import=time 
# --hidden-import=tkinter.tix.WINDOW 
# --hidden-import=csv 
# --hidden-import=os
# --hidden-import=PyQt5.QtWidgets 
# --hidden-import=PyQt5.QtCore 
# --hidden-import=PyQt5.QtGui 
# --hidden-import=serial 
# --hidden-import=pydrive2.auth.GoogleAuth 
# --hidden-import=pydrive2.drive.GoogleDrive 
# --hidden-import=google-api-python-client

# PyDrive starter : https://pythonhosted.org/PyDrive/quickstart.html
# Para correr .exe en pc producción: mover client_secrets.json a carpeta dist
# Si hay problemas con PyDrive: 
#       1. Entrar a interface.spec y pegar from PyInstaller.utils.hooks import collect_data_files
#       2. En interface.spec cambiar datas=[] por datas=collect_data_files("googleapiclient")
#       3. ACTUALIZAR .exe con .spec

import sys
from datetime import datetime
from tkinter.tix import WINDOW

from ctest import *
import csv, os

from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *


def init_vars(): 
    # Global variables
    global names, test_showvalues, test_sendvalues, tp_dict, tp_connected, tp_disconnected, dir
    global FESclient, FESnum, tolerances, headers
    global parameters, testpoints, tpx, test_expectedvalue, testtypes, min, max, averageFES, averageWF, default_tolerances, FESres

    headers            = ["Testpoint", "Descripción", "Valor esperado", "Valor obtenido", "Evaluación", "% tolerancia","Seleccionar tests"]
    FESclient          = [""]
    FESnum             = None
    tolerances         = [str(x) for x in range(1,30+1)]
    parameters, testpoints, tpx, test_expectedvalue, testtypes, min, max, averageFES, averageWF, default_tolerances, FESres = [[], [], [], [], [], [], [], [], [], [], []]

    test_showvalues = []
    test_sendvalues = ['T']
    tp_dict = {}
    tp_disconnected, tp_connected = [[], []]
    names = []
    # dir = 'E:/Respaldo/Escritorio/ProyectosBelen/Interface'
    dir = ''
    
def read_default_values(): 
    global names
    ### Abrir archivo default_data.csv
    try: 
        file = open(f'{dir}default_values.csv', encoding='utf-8')
    except Exception as e: 
        return f'{e}' 
    csvreader = csv.reader(file)
    aux = 0
    i = 0

    for row in csvreader:
        if row[0] != "" and aux == 0:
            if row[0] != "Testpoint":
                parameters.append(row)
                tpx.append(row[0])
                testpoints.append(row[1])
                testtypes.append(row[3])
                min.append(row[4])
                max.append(row[5])
                averageFES.append(row[6])
                averageWF.append(row[7])
                default_tolerances.append(row[8])
                FESres.append(["0"])
                tp_dict[i] = row[9]
                if row[2] == "CONECTADO": 
                    tp_connected.append(i)
                else:
                    tp_disconnected.append(i)
                i += 1
        elif row[0] == "Nombres":
            names = [""] + row[1:]
            aux = 2
        elif row[2].isnumeric() == True and "R" in row[0]:
                row[3] = '\n' 
        else: 
            aux += 1

    for i in range(len(min)):
        code = tp_dict[i] 
        if i in tp_connected: 
            code = chr(ord(code)+1)
        if min[i] != "-" and max[i] != "-" and averageFES[i] == "-":
            test_expectedvalue.append([f'{min[i]}V - {max[i]}V', f'{min[i]}V - {max[i]}V'])
            test_showvalues.append(f'{min[i]}V - {max[i]}V')
            test_sendvalues.append([f'{code}','a0\n',f'l{int(1000*float(min[i]))}\n',f'h{int(1000*float(max[i]))}\n'])
        elif min[i] != "-" and max[i] == "-" and averageFES[i] == "-":
            test_expectedvalue.append([f'>{min[i]}V', f'>{min[i]}V'])
            test_showvalues.append(f'>{min[i]}V')
            test_sendvalues.append([f'{code}','a0\n',f'l{int(1000*float(min[i]))}\n','h0\n'])
        elif min[i] == "-" and max[i] != "-" and averageFES[i] == "-":
            test_expectedvalue.append([f'<{max[i]}V', f'<{max[i]}V'])
            test_showvalues.append(f'<{max[i]}V')
            test_sendvalues.append([f'{code}',f'a0\n',f'l0\n',f'h{int(1000*float(max[i]))}\n'])
        elif min[i] == "-" and max[i] == "-" and averageFES[i] != "-":
            test_expectedvalue.append([f'{averageFES[i]}V', f'{averageWF[i]}V'])
            if averageFES[i] != averageWF[i]:
                test_showvalues.append(f'{averageFES[i]}V, {averageWF[i]}V')
            else:
                test_showvalues.append(f'{averageFES[i]}V')
            test_sendvalues.append([f'{code}',f'a{int(1000*float(averageFES[i]))}\n','l0\n','h0\n'])
        else:
            test_expectedvalue.append([f'Rango incorrecto', f'Rango incorrecto'])
            test_showvalues.append(f'Rango incorrecto')
    test_sendvalues.append('Y')
    return ''

### Inicio aplicación
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setMinimumSize(1100, 600)
        self.setWindowTitle("Test eléctrico para FES")
        
        # Set buttons
        self.setRunButton()
        self.setReportButton()
        self.setConnectButton()
        self.setCalibButton()
        self.setVrefButton()

        self.err_default_values()
            
        # Init variables 
        self.FESnumber    = None
        self.name         = None
        self.ToolFESvals  = [None] * len(testpoints)
        self.FESvalues    = None
        self.FESconnected = "none"
        self.actualFES    = ""
        self.finalResult  = 0
        self.FESfound     = [""]
        self.port         = 0
        self.tols         = []
        self.chosenFES    = ''
        self.search       = 0

        # Set table
        self.createTable()

        # Set inputs
        self.inputName = QComboBox()
        self.inputName.addItems(names)
        self.inputFESnums = QComboBox()
        self.inputFESnums.addItems(self.FESfound)

        # Set initial layouts
        self.setLayout()
    def err_default_values(self): 
        while True: 
            err_code = read_default_values()
            if 'default_values.csv' in err_code : 
                alert = QMessageBox()
                alert.setText(err_code)
                alert.setWindowTitle("Alerta")
                alert.exec()
            else: 
                break

    def setRunButton(self):
        self.button = QPushButton("Testear", self)
        self.button.resize(150, 50)
        self.button.released.connect(self.RUN_button_was_released) 

    def setConnectButton(self):
        self.button2 = QPushButton(f'Buscar FES', self)
        self.button2.resize(150, 50)
        self.button2.clicked.connect(self.BLESearch_button_was_released) 

    def setReportButton(self):
        self.button3 = QPushButton(f'Generar reporte', self)
        self.button3.resize(150, 250)
        self.button3.released.connect(self.REPORT_button_was_clicked) 
    
    def setCalibButton(self):
        self.button4 = QPushButton(f'Calibrar herramienta', self)
        self.button4.resize(150, 2500)
        self.button4.released.connect(self.CALIBRATION_button_was_clicked) 
    
    def setVrefButton(self):
        self.button5 = QPushButton(f'Voltajes de referencia', self)
        self.button5.resize(150, 2500)
        self.button5.released.connect(self.VREF_button_was_clicked) 

    def BLESearch_button_was_released(self):  
        # Selección de puerto si esque no hay
        if not self.port: 
            text , pressed = QInputDialog.getText(window, "Puerto COM", "Ingresar puerto COM para ToolFES",
                                                QLineEdit.Normal, "")
            if pressed:
                self.port = 'COM' + text
                initSerialCom(self.port)
                _, error = readFES(test_sendvalues)
            
            else:  
                error = 'Error: no port selected'
        
            if "Error" in error:
                self.port = 0
                alert = QMessageBox()
                alert.setText(error)
                alert.setWindowTitle("Alerta")
                alert.exec() 
                return 

        self.FESconnected   = "searching"
        self.button2.setText("Buscando FES . . .")
        self.button2.repaint()
        self.FESfound     = [""]

        state, newFES = readFES("!!\n")
        if "Error" in newFES:
            self.port = 0
            alert = QMessageBox()
            alert.setText(newFES)
            alert.setWindowTitle("Alerta")
            self.button2.setText("Connect to FES")
            self.button2.repaint()
            alert.exec() 
            return

        self.finalResult = 0
        self.setLayout()

        for line in newFES:
            if line[0].decode("utf-8")[:-1] not in self.FESfound and 'F' in line[0].decode("utf-8"):
                self.FESfound.append(line[0].decode("utf-8")[:-1])
                FESclient.append(line[2].decode("utf-8"))

        if len(self.FESfound) > 1:
            self.FESconnected   = "connecting"
            self.button2.setText("Buscar denuevo")
            self.button2.repaint()

        elif len(self.FESfound) == 1:
            alert = QMessageBox()
            alert.setText('No se encontraron FES')
            alert.setWindowTitle("Alerta")
            alert.exec()
            self.FESconnected   = "none"
            self.button2.setText("Buscar denuevo")
            self.button2.repaint()

        self.setLayout()

    def RUN_button_was_released(self):
        print("Run button was released")
        success = 1

        # 1. Check if access to port is OK
        if not self.port: 
            # Init serial COM
            text , pressed = QInputDialog.getText(window, "Puerto COM", "Ingresar puerto COM para ToolFES",
                                                QLineEdit.Normal, "")
            if pressed:
                self.port = 'COM' + text
                initSerialCom(self.port)
                _, error = readFES(test_sendvalues)

            else: 
                error = 'Error: no port selected'

            if "Error" in error:
                window.port = 0
                alert = QMessageBox()
                alert.setText(error)
                alert.setWindowTitle("Alerta")
                alert.exec() 
                return

        # 2. Run disconnected tests --> verify name is selected
        self.name      = self.inputName.currentText()
        self.FESnumber = self.inputFESnums.currentText()

        if len(self.name) > 0 and len(self.FESnumber) > 0: 
            if self.name == 'Other':
                text , pressed = QInputDialog.getText(window, "Insert name", "Text: ",
                                        QLineEdit.Normal, "")
                if pressed:
                    self.name = text
                    names.insert(1, self.name)
                    self.inputName.clear()
                    self.inputName.addItems(names)
                    self.inputName.repaint()
            
            success = self.disconnected_tests()

        if not success: 
            return 
        elif len(self.FESnumber) == 0:
            alert = QMessageBox()
            alert.setText('Debe buscar un FES')
            alert.setWindowTitle("Alerta")
            alert.exec()
            return
        elif len(self.name) == 0:
            alert = QMessageBox()
            alert.setText('Debe ingresar su nombre')
            alert.setWindowTitle("Alerta")
            alert.exec()
            return
        
        # 3. Connect to FES    
        # if self.continuar == True: 
        self.connectFES(self.search)
        self.FESnumber = self.chosenFES

        # 4. Run connected tests
        if self.FESconnected == "yes": 
            # Confirmar que el FES esté conectado
            _, snumber = readFES("N")
            try:
                target = 'X' if 'FES' in snumber[0][0].decode("utf-8") else 'W' 
                self.snumber = snumber
            except: # reset variables
                self.FESnumber    = None
                self.FESvalues    = None
                self.name         = None
                self.FESfound     = [""]
                self.FESconnected = "none"
                self.actualFES    = ""
                self.createTable()
                self.inputFESnums.clear()
                self.inputFESnums.addItems(self.FESfound)
                self.finalResult  = 0
                self.setLayout()
                self.checkError()
                return 

        if  self.continuar == True:
            self.connected_tests(target)
        else: 
            self.FESvalues.append([b'TP6', b'F', b'F'])
            self.FESvalues.append([b'TPA', b'F', b'F'])
            self.tols.append(f'0%')
            self.tols.append(f'0%')
            self.addDataToTable()
            self.button.setText("Testear")
            self.setWindowTitle("Test eléctrico para FES")

        state, response = readFES("!_\n")
        self.FESconnected = "none"
        self.actualFES    = ""
        self.search = 1

    def REPORT_button_was_clicked(self):
        print("Report button was clicked")  
        try:
            serialname = (self.snumber[0][0].decode("utf-8")[3:])
            print(serialname)
            target = 0 if 'FES' in serialname else 1 
        except:
            serialname = f'FES{self.FESnumber}'
            target = 0
            pass
        if self.FESvalues != None:
            csv_name = serialname if 'FES' not in serialname[0:3] else "NoFESnumber"
            now = datetime.now()
            
            filename = f'{serialname}_{now.strftime("%d")}{now.strftime("%m")}{now.strftime("%Y")}_{self.finalResult}'
            newline = [f'{now.strftime("%d")}/{now.strftime("%m")}/{now.strftime("%Y")}', csv_name] # Para el archivo maestro 
            
            with open(f'{filename}.csv', 'w', encoding='UTF8') as f:
                writer = csv.writer(f)
                writer.writerow(["Date", datetime.now()])
                writer.writerow(["Name", self.name]) 
                writer.writerow(["FES number", serialname])
                writer.writerow(headers[1:-1])
                
                for i in range(len(self.FESvalues)):
                    # Setear evaluación
                    if  self.FESvalues[i][2].decode("utf-8").rstrip('\x00') == "B":
                        evaluation = "Dentro del rango"
                    elif self.FESvalues[i][2].decode("utf-8").rstrip('\x00') == "M":
                        evaluation = "Fuera del rango"
                    else: 
                        evaluation = "No aplica"

                    # Agregar información al .csv dependiendo de si es un FES o WF. 
                    append_tol = 0

                    if self.tols[0] == 'X' or self.tols[0] == 'W': 
                        writer.writerow([testtypes[i], test_expectedvalue[i][target], self.FESvalues[i][1].decode("utf-8").rstrip('\x00'), evaluation, f'{self.tols[i+1]}']) # VERIFICAR "%"
                        append_tol = self.tols[i+1][:-1]
                    else:
                        writer.writerow([testtypes[i], test_expectedvalue[i][target], self.FESvalues[i][1].decode("utf-8").rstrip('\x00'), evaluation, f'{self.tols[i]}'])   # VERIFICAR "%"
                        append_tol = self.tols[i][:-1]

                    newline.append(append_tol)
                    if min[i] != "-" and max[i] != "-" and averageFES[i] == "-":
                        newline.append(str(float(min[i])*(1 - int(append_tol)/100)))
                        newline.append(str(float(max[i])*(1 + int(append_tol)/100)))
                    elif min[i] != "-" and max[i] == "-" and averageFES[i] == "-":
                        newline.append(str(float(min[i])*(1 - int(append_tol)/100)))
                        newline.append('-')                    
                    elif min[i] == "-" and max[i] != "-" and averageFES[i] == "-":
                        newline.append('-')
                        newline.append(str(float(max[i])*(1 + int(append_tol)/100)))                    
                    elif min[i] == "-" and max[i] == "-" and averageFES[i] != "-":
                        newline.append(str(float(test_expectedvalue[i][target][:-1])*(1 - int(append_tol)/100)))
                        newline.append(str(float(test_expectedvalue[i][target][:-1])*(1 + int(append_tol)/100)))
                    else:
                        newline.append('')
                        newline.append('')
                    newline.append(self.FESvalues[i][1].decode("utf-8").rstrip('\x00'))

            # Carga del archivo a drive 
            name = saveToDrive(filename, newline, dir)
            if 'Error' in name:
                alert = QMessageBox()
                alert.setText(f'{name}')
                alert.setWindowTitle("Alerta")
                alert.exec()
                if 'Error 1' in name:
                    return

            os.replace(f'{name}.csv',  f'{dir}Reportes/{name}.csv')
            try: 
                os.remove(f'{dir}Reportes/AllTests.csv')
            except Exception as e: 
                pass
            os.replace(f'AllTests.csv', f'{dir}Reportes/AllTests.csv')

            msg = QMessageBox()
            msg.setWindowTitle('Reporte generado')
            msg.setIcon(QMessageBox.Question)
            msg.setText(f'Seleccionar YES si quieres hacer un nuevo test.\nSeleccionar NO si quieres repetir este test.')
            msg.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
            retval = msg.exec()

            if retval == QMessageBox.Yes:
                self.FESnumber    = None
                self.name         = None
                self.FESvalues    = None
                self.FESconnected = "none"
                self.actualFES    = ""
                self.FESfound     = [""]
                FESclient         = [""]
                self.createTable()
                self.inputFESnums.clear()
                self.inputFESnums.addItems(self.FESfound)
                self.finalResult  = 0
                self.setLayout()
        else:
            alert = QMessageBox()
            alert.setText('Correr primero el programa')
            alert.setWindowTitle("Alerta")
            alert.exec()
    
    def CALIBRATION_button_was_clicked(self):
        if not self.port: 
            text , pressed = QInputDialog.getText(window, "Puerto COM", "Ingresar puerto COM para ToolFES",
                                                QLineEdit.Normal, "")
            if pressed:
                self.port = 'COM' + text
                initSerialCom(self.port)
                _, error = readFES(test_sendvalues)
            
            else:  
                error = 'Error: no port selected'
        
            if "Error" in error:
                self.port = 0
                self.checkError(error)
                return 
                
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Question)
        msg.setWindowTitle("Iniciar calibración")
        msg.setText(f'¿Seguro/a que quiere iniciar la calibración?') 
        msg.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
        retval = msg.exec()
        
        if retval ==  QMessageBox.No: 
            return
        
        self.button4.setText("Calibrando . . .")
        self.button4.repaint()
        _, error = readFES('V')
        self.button4.setText("Calibrar")
        self.button4.repaint()

        if "Error" in error: 
            self.checkError(error)
            return
            
        alert = QMessageBox()
        alert.setText("Herramienta calibrada")
        alert.setWindowTitle("Completado")
        alert.exec()
    
    def VREF_button_was_clicked(self):
        if not self.port: 
            text , pressed = QInputDialog.getText(window, "Puerto COM", "Ingresar puerto COM para ToolFES",
                                                QLineEdit.Normal, "")
            if pressed:
                self.port = 'COM' + text
                initSerialCom(self.port)
                _, error = readFES(test_sendvalues)
            
            else:  
                error = 'Error: no port selected'
        
            if "Error" in error:
                self.port = 0
                self.checkError(error)
                return 
                
        # Set Vrefs
        dialog = InputDialog()
        if dialog.exec():
            Vref1, Vref2 = dialog.getInputs()
            readFES(f'm{Vref1}\n') 
            readFES(f'n{Vref2}\n') 

    def disconnected_tests(self):
        self.setLayout()

        self.FESvalues = []
        state = ['']*len(testpoints)
        self.continuar = True
        self.tols = []

        for i in tp_disconnected:
            if self.table_checks[i].isChecked() and self.continuar:
                state[i], response = readFES([f'{tp_dict[i]}{self.table_tols[i].currentText()}\n'])
                if "Error" in response:
                    self.checkError()
                    return 0
                if response[0][1].decode("utf-8") == "nan":
                    self.checkError("Debe calibrar la herramienta")
                    return 0

                self.FESvalues.append(response[0])
                if response[0][2].decode("utf-8") == "M":
                    self.continuar = False
                self.tols.append(f'{self.table_tols[i].currentText()}%')

            elif not self.continuar and self.table_checks[i].isChecked(): 
                state[i], response = readFES([f'{tp_dict[i]}100\n'])
                if "Error" in response:
                    print("error :(")
                    self.checkError()
                    return 0
                self.FESvalues.append(response[0])
                self.tols.append(f'0%')
            
            else: 
                _ , response = readFES([f'{tp_dict[i]}150\n'])
                if "Error" in response:
                    print("error:(")
                    self.checkError()
                    return 0
                state[i] = ANALYSIS
                self.FESvalues.append(response[0])
                self.tols.append(f'0%')

        if "Error" in self.FESvalues:
            self.checkError()
            return 0
        return 1

    def connected_tests(self, target):
        self.setLayout()
        _, error = readFES(['T', f'DP{int(1000*float(averageWF[3]))}\nL0\nH0\n','Y'] if target == 'W' else ['T',f'DP{int(1000*float(averageFES[3]))}\nL0\nH0\n','Y']) 

        state = ['']*len(testpoints)
    
        self.tols.insert(0,target)
        for i in tp_connected:
            if self.table_checks[i].isChecked() and self.continuar:
                state[i], response = readFES([f'{tp_dict[i]}{self.table_tols[i].currentText()}\n'])
                self.tols.append(f'{self.table_tols[i].currentText()}%')
                self.FESvalues.append(response[0])
                if response[0][2].decode("utf-8") == "M":
                    self.continuar = False

            elif not self.continuar and self.table_checks[i].isChecked():
                state[i], response = readFES([f'{tp_dict[i]}100\n'])
                if "Error" in response:
                    self.checkError()
                    return 
                self.FESvalues.append(response[0])
                self.tols.append(f'0%')
                
            else:
                _ , response = readFES([f'{tp_dict[i]}150\n'])
                if "Error" in response:
                    self.checkError()
                    return 
                state[i] = ANALYSIS
                self.FESvalues.append(response[0])
                self.tols.append(f'0%')
        
        if "Error" in self.FESvalues:
            self.checkError()
            return 

        if FAIL not in state:
            self.addDataToTable()
            self.button.setText("Testear")
            self.setWindowTitle("Test eléctrico para FES")
        else: 
            self.checkError()

    def selectFES(self):
        self.chosenFES = self.FESnumber
        if self.chosenFES != None and (self.actualFES != self.chosenFES or self.FESconnected  == "connecting"):
            self.actualFES = self.chosenFES
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Question)
            msg.setText(f'¿Conectarse a {self.chosenFES}?')
            msg.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
            retval = msg.exec()
            return retval

    def connectFES(self, search=0):
        client = 0
        if search: 
            state, newFES = readFES("!!\n")
            if "Error" in newFES:
                self.port = 0
                alert = QMessageBox()
                alert.setText(newFES)
                alert.setWindowTitle("Alerta")
                self.button2.setText("Connect to FES")
                self.button2.repaint()
                alert.exec() 
                return

            self.finalResult = 0
            self.setLayout()

            for line in newFES:
                if self.FESnumber == line[0].decode("utf-8")[:-1]:
                    client = line[2].decode("utf-8")[1]
            if not client: 
                self.FESnumber = None
                alert = QMessageBox()
                alert.setText('Debe buscar un FES')
                alert.setWindowTitle("Alerta")
                alert.exec()
                return
            
        retval = self.selectFES()
        if retval == QMessageBox.Yes:
            if search:  
                a, newFES = readFES(f"!{client}\n")
            else: 
                a, newFES = readFES(f"!{FESclient[self.FESfound.index(self.chosenFES)][1]}\n")

            if newFES == "Falló conexión a FES":
                msg = QMessageBox()
                msg.setIcon(QMessageBox.Information)
                msg.setText(f'Falló conexión a {self.chosenFES}.')
                msg.setInformativeText(f'¿Correr sin conectarse a FES?')
                msg.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
                retval = msg.exec()
            elif len(newFES) > 0:
                self.actualFES = self.chosenFES
                self.FESconnected = "yes"
                self.labelnum = QLabel(f"FES: {self.FESnumber}")
                self.labelnum.repaint()   
                self.FESnumber = self.chosenFES
                self.button.setText("Testear")
                self.button.repaint()
                self.FESfound = [self.FESnumber]
                self.inputFESnums.clear()
                self.inputFESnums.addItems(self.FESfound)
                self.inputFESnums.repaint()

        else: 
            if self.FESconnected == "no":
                self.button.setText(f'Correr sin conectarse a FES')
                self.button.repaint()   
            self.button2.setText("Buscar FES")
            self.button2.repaint()   
            self.setLayout()
        
        self.search = 0
        
    def setLayout(self):
        layout1 = QHBoxLayout()
        layout2 = QHBoxLayout()
        layout3 = QVBoxLayout()

        layout4 = QHBoxLayout()
        layout5 = QHBoxLayout()
        layout7 = QVBoxLayout()

        self.layout6 = QVBoxLayout()
        self.layout7 = QVBoxLayout()
        self.layout8 = QVBoxLayout()

        layoutTable  = QVBoxLayout()

        # Layout 1: Name
        if self.name == None:
            self.labelname = QLabel('INGRESAR NOMBRE:')
        else:
            self.labelname = QLabel(f'Nombre: {self.name}')
        self.labelname.setAlignment(Qt.AlignCenter)

        layout2.addWidget(self.labelname, 1)
        layout2.addWidget(self.inputName, 2)

        # Layout 2: FES Serial Number
        if self.FESnumber == None:
            self.labelnum = QLabel(f'INGRESAR FES:')
        else:
            self.labelnum = QLabel(f"FES: {self.FESnumber}")
        self.labelnum.setAlignment(Qt.AlignCenter)

        layout1.addWidget(self.labelnum, 1)
        self.inputFESnums.clear()
        self.inputFESnums.addItems(self.FESfound)
        layout1.addWidget(self.inputFESnums, 2)

        # Layout 3: Layout 1 and 2 vertically positioned
        layout3.addLayout(layout1)
        layout3.addLayout(layout2)
        
        # Layout 7: OK button and Loading vertically positioned

        if self.FESconnected == "none": 
            layout7.addWidget(self.button2)     
        elif self.FESconnected == "connecting" or self.FESconnected == "yes":
            self.button2.setText(f'Buscar denuevo')
            layout7.addWidget(self.button2) 
        layout7.addWidget(self.button)

        # Layout 4: Layout 4 and buttons horizontally positioned
        layout4.addLayout(layout3, 3)
        layout4.addLayout(layout7, 1)

        # Layout table
        layoutTable.addWidget(self.table) 

        # Layout 5: button 3
        layout5.addWidget(self.button5, 1)
        layout5.addWidget(self.button4, 1)
        layout5.addWidget(self.button3, 2)
        
        # Layout 8: evaluation
        if self.finalResult == "A":
            self.label = QLabel()
            self.label.setStyleSheet("border :3px solid black;")
            self.label.setStyleSheet("background-color: green ")
            self.label.move(100, 100)   
            self.label.resize(100, 5)
            self.layout8.addWidget(self.label)
        elif self.finalResult == "R": 
            self.label = QLabel()
            self.label.setStyleSheet("border :3px solid black;")
            self.label.setStyleSheet("background-color: red ")
            self.label.move(100, 100)   
            self.label.resize(100, 5)
            self.layout8.addWidget(self.label)
        else: 
            pass        
        
        # Layout 6: final layout
        self.layout6.addLayout(layout4)
        self.layout6.addLayout(layoutTable)
        self.layout6.addLayout(self.layout8 )
        self.layout6.addLayout(layout5)

        container = QWidget()
        container.setLayout(self.layout6)
        self.setCentralWidget(container)

    def createTable(self):
    # Crea Tabla
        self.table = QTableWidget()
        self.table.setRowCount(len(testpoints))
        self.table.setColumnCount(7)
        hfont = QFont()
        hfont.setBold(True)
        self.table.setHorizontalHeaderLabels(headers) 
        for i in range(len(headers)):
            self.table.horizontalHeaderItem(i).setFont(hfont)

        self.table_tols = []
        self.table_checks = []
        
        for j in range(len(testpoints)):
            self.table_tols.append(QComboBox())
            self.table_checks.append(QCheckBox())

            # Testpoints
            item = QTableWidgetItem(f"{testpoints[j]}")
            self.table.setItem(j,0, item)

            # Test names
            item = QTableWidgetItem(f"{testtypes[j]}")
            item.setTextAlignment(Qt.AlignCenter)
            self.table.setItem(j,1, item)
            
            # Test expected values
            item = QTableWidgetItem(f"{test_showvalues[j]}")
            item.setTextAlignment(Qt.AlignCenter)
            self.table.setItem(j,2, item)
                
            # Test tolerances
            if default_tolerances[j] != tolerances[0]:
                self.table_tols[j].addItems([default_tolerances[j]] + tolerances)
            else:
                self.table_tols[j].addItems(tolerances)
            self.table.setCellWidget(j,5, self.table_tols[j])

            self.table_tols[j].addItems(tolerances)
            self.table.setCellWidget(j,5, self.table_tols[j])

            # Test checkboxes
            self.table_checks[j].setStyleSheet("padding: 65px;")
            self.table_checks[j].setChecked(True)
            self.table.setCellWidget(j,6, self.table_checks[j])

        self.table.move(0,0)

    def addDataToTable(self):
        for j in range(len(self.FESvalues)):
            num, FESdata, FESevaluation = [self.FESvalues[j][0].decode("utf-8")[2], 
                                        self.FESvalues[j][1].decode("utf-8"), 
                                        self.FESvalues[j][2].decode("utf-8")]
            # Add evaluation
            if self.FESconnected == "yes" or j in tp_disconnected:
                # Add values
                value = QTableWidgetItem("NO REALIZADO" if FESdata == "F" else ("-" if FESdata == "N" else FESdata))
                if FESdata == "F": 
                    value.setForeground(QBrush(QColor(210, 0, 0)))
                self.table.setItem(j,3, value)

                # Add evaluation
                if FESevaluation == "SB" or FESevaluation == "N":
                    evaluation = QTableWidgetItem(f" ")
                    evaluation.setForeground(QBrush(QColor(0, 0, 0)))
                    FESres[j] = "SB"
                elif FESevaluation == "F":
                    evaluation = QTableWidgetItem(f" ")
                    evaluation.setForeground(QBrush(QColor(210, 0, 0)))
                    FESres[j] = "M"
                elif FESevaluation == "M":
                    evaluation = QTableWidgetItem(f"Fuera del rango")
                    evaluation.setForeground(QBrush(QColor(210, 0, 0)))
                    FESres[j] = "M"
                elif FESevaluation == "B":
                    FESres[j] = "B"
                    evaluation = QTableWidgetItem(f"Dentro del rango")
                    evaluation.setForeground(QBrush(QColor(0, 160, 0)))
                self.table.setItem(j,4,evaluation)

                if 'F' in self.FESnumber:
                    self.table.setItem(j,2,QTableWidgetItem(f'{test_expectedvalue[j][0]}'))
                else:
                    self.table.setItem(j,2,QTableWidgetItem(f'{test_expectedvalue[j][1]}'))
            else: 
                value = QTableWidgetItem("NO REALIZADO")
                value.setForeground(QBrush(QColor(210, 0, 0)))
                evaluation = QTableWidgetItem(f" ")
                evaluation.setForeground(QBrush(QColor(210, 0, 0)))
                self.table.setItem(j,3, value)
                self.table.setItem(j,4, evaluation)
                FESres[j] = "M"

        self.finalResult = "R" if "M" in FESres else "A"
        self.setLayout()

    def checkError(self, error="ToolFES desconectada :("):
        close_serial_port()
        alert = QMessageBox()
        alert.setText(error)
        alert.setWindowTitle("Alerta")
        self.button2.setText("Buscar FES")
        self.button2.repaint()
        alert.exec()

class InputDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.first = QLineEdit(self)
        self.second = QLineEdit(self)
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, self);

        layout = QFormLayout(self)
        layout.addRow("VREF 1 (mV)", self.first)
        layout.addRow("VREF 2 (mV)", self.second)
        layout.addWidget(buttonBox)

        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)

    def getInputs(self):
        return (self.first.text(), self.second.text())

init_vars()

app = QApplication(sys.argv)
window = MainWindow()

# Init serial COM
text , pressed = QInputDialog.getText(window, "Puerto COM", "Ingresar puerto COM para ToolFES",
                                    QLineEdit.Normal, "")
if pressed:
    puerto = 'COM' + text
    initSerialCom(puerto)
    _, error = readFES(test_sendvalues)
else: 
    error = 'Error: no se seleccionó puerto'

window.setWindowModality(Qt.ApplicationModal)
window.show()

if not os.path.exists(f'{dir}Reportes'): 
    os.makedirs(f'{dir}Reportes')

if "Error" in error:
    window.port = 0
    alert = QMessageBox()
    alert.setText(error)
    alert.setWindowTitle("Alerta")
    alert.exec() 
else:
    window.port = puerto

app.exec()

