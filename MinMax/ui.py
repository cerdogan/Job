##
# @file ui.py
# @author Can Erdogan
# @date Jul 21, 2015
# @brief Python QT interface to play chess.
##

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4 import QtGui, QtCore
import sys
from subprocess import call
import os.path
import time
 
prevClicked = 0
lastItem = 0
board = []
table = 0

##################################################################################################
def handle_item_clicked(tableItem): 

    global lastItem
    if lastItem == tableItem:
        return

    # Save the state of the first click
    global prevClicked 
    if prevClicked == 0:
        prevClicked = 1
        lastItem = tableItem

    # Move the item from the previous cell to this one
    else:

        # Make the move in gui
        prevClicked = 0
        tableItem.setForeground(lastItem.foreground())
        tableItem.setText(lastItem.text())
        lastItem.setText("");
        lastIndex = (7 - lastItem.row()) * 8 + lastItem.column()
        index = (7 - tableItem.row()) * 8 + tableItem.column()
        board[index] = board[lastIndex]
        board[lastIndex] = 0
        time.sleep(0.1)
        
        # Make the call to the chess
        stateFile = open('state', 'w')
        for item in board:
           stateFile.write("%d " % item)
        stateFile.close()
        call(['./a.out', '-ui'])
 
        # Wait for the command
        while not os.path.exists('command'):
            time.sleep(0.1)
        commandFile = open('command', 'r')
        line = commandFile.readline()
        data = line.split()
        board[int(data[1])] = board[int(data[0])]
        board[int(data[0])] = 0

        # Draw the new location for the opponent
        global table
        px = 7 - int(data[0])/8
        py = int(data[0])%8
        px2 = 7 - int(data[1])/8
        py2 = int(data[1])%8
        table.item(px2,py2).setText(table.item(px,py).text())
        table.item(px2,py2).setForeground(table.item(px,py).foreground())
        table.item(px,py).setText("")

##################################################################################################
def main():  

    # Initialize state
    global board
    board = [0 for x in range(64)]
    for i in range(8):
			board[i+8] = i+1
			board[i+8*6] = i+1+16
    for i in range(8):
			board[i] = i+9
			board[i+56] = i+25

    app = QApplication(sys.argv)
    global table
    table = QTableWidget()
    table.itemClicked.connect(handle_item_clicked)
    
    # initiate table
    table.setWindowTitle("Simple Chess")
    table.resize(600, 600)
    table.setRowCount(8)
    table.setColumnCount(8)
    table.setHorizontalHeaderLabels(('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'))
    table.setVerticalHeaderLabels(('8', '7', '6', '5', '4', '3', '2', '1'))
    
    # Set the colors
    count = 0
    font = QFont("Courier New Bold", 20)
    table.setFont(font)
    for i in range(8):
        for j in range(8):
            newitem = QtGui.QTableWidgetItem("")
            if i % 2 == 0 and j % 2 == 1 or i % 2 == 1 and j % 2 == 0 :
                newitem.setBackground(QtGui.QColor(200,200,150))
            else: 
                newitem.setBackground(QtGui.QColor(255,255,255))
            newitem.setTextAlignment(Qt.AlignCenter)
            newitem.setFlags(QtCore.Qt.ItemIsEnabled)
            if i > 5:
                newitem.setForeground(QtGui.QColor(255,0,0))
            elif i < 3:
                newitem.setForeground(QtGui.QColor(0,0,255))
            table.setItem(i, j, newitem)

    # set data
    for i in range(8):
        table.item(1,i).setText("P")
        table.item(6,i).setText("P")
        
    table.item(0,0).setText("R")
    table.item(0,7).setText("R")
    table.item(7,0).setText("R")
    table.item(7,7).setText("R")
    table.item(0,1).setText("K")
    table.item(0,6).setText("K")
    table.item(7,1).setText("K")
    table.item(7,6).setText("K")
    table.item(0,2).setText("B")
    table.item(0,5).setText("B")
    table.item(7,2).setText("B")
    table.item(7,5).setText("B")
    table.item(0,3).setText("Q")
    table.item(7,3).setText("Q")
    table.item(0,4).setText("W")
    table.item(7,4).setText("W")
 
 
    # Set background colors
    for i in range(8):
        table.setColumnWidth(i, 70)
        table.setRowHeight(i, 70)

    # show table
    table.show()
    return app.exec_()
 
##################################################################################################
if __name__ == '__main__':
    main()
