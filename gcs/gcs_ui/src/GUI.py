#!/usr/bin/env python

#ROS imports
import rosbag
import rospy
from std_msgs.msg import Int32, String

#PyQt Imports
import sys
from PyQt4 import QtGui, QtCore

#PyGraph Imports
import time
import pyqtgraph as pg
import numpy as np

#Global Variables
global app
global curve1, curve2
global bag
global bagName
global pressed
global counter , counter2, time
global num

global topicName
global topicAmount

topicName = "counter"
topicAmount = 1

num = 0
counter =  []
counter2 = []
time = []
pressed = False
# bag = rosbag.Bag('test.bag', 'w')

def initWBag():
  global bag, bagName
  bag = rosbag.Bag(bagName + '.bag', 'w')

def initBag():
  global bag, bagName
  bag = rosbag.Bag(bagName + '.bag')


def update():
  global counter,counter2, time, curve1, curve2, app, num 
  # plotWidget.plot(time, counter, pen='b',name = 'blueLine' ,clear=True)
  # plotWidget.plot(time, counter2, pen='r', name = 'redLine')
  curve1.setData(time, counter, pen = 'b')
  # curve2.setData(time, counter2, pen = 'r')
  app.processEvents()

def writeBag(msg):
  bag.write('number', msg)


def showBag():
  global bag
  bag.close()
  initBag()
  for topic, msg, t in bag.read_messages(topics=['number']):
    print msg  
  bag.close()

def callback(msg):
  global pressed, counter, counter2, time, num
  counter.append(msg.data)
  counter2.append(msg.data + 5)
  time.append(num)
  num +=1 
  if pressed:
    writeBag(msg)



class Example(QtGui.QWidget):
    global topicName

    sub = rospy.Subscriber(topicName, Int32, callback)

    def __init__(self):
        super(Example, self).__init__()   #Calls parent constructor
        
        self.initUI()                     #Calls bottom

    def handleButton(self, text):
      global pressed, bagName

      if not pressed:
        bagName = self.textbox.text()
        initWBag()

      pressed = not pressed
      print "Pushed"

      self.button.setText('Stop')
      self.button.setStyleSheet("background-color:#CCCCCC;")

      if not pressed: 
        showBag()
        self.button.setText('Record')
        self.button.setStyleSheet("background-color:#FFFFFF;")

      QtGui.QApplication.processEvents()
        
    def initUI(self):
      global curve1, curve2, counter, counter2, time

      #Button
      self.button = QtGui.QPushButton('Record', self)
      self.button.setStyleSheet("background-color:#FFFFFF;")
      
      #TextBox
      self.textbox = QtGui.QLineEdit('TypeBagNameHere',self)

      #ComboCheckBox
      areas = ["Data one", "Data two", "Data three", "Data four"]
      model = QtGui.QStandardItemModel(5, 1)# 5 rows, 1 col

      firstItem = QtGui.QStandardItem("---- Select Data ----")  
      firstItem.setBackground(QtGui.QBrush(QtGui.QColor(200, 200, 200)))
      firstItem.setSelectable(False)
      model.setItem(0, 0, firstItem)

      for i in range (0,4): 
        item = QtGui.QStandardItem(areas[i])
        item.setFlags(QtCore.Qt.ItemIsUserCheckable | QtCore.Qt.ItemIsEnabled)
        item.setData(QtCore.Qt.Unchecked, QtCore.Qt.CheckStateRole)
        model.setItem(i + 1, 0, item)

      self.comboBox = QtGui.QComboBox()  
      self.comboBox.setModel(model) 

      curve1 = pg.PlotCurveItem(time, counter, pen='b', name = 'blueLine')
      # curve2 = pg.PlotCurveItem(time, counter2, pen='r', name = 'redLine')
      curve1.setClickable(True, width=10)
      plotWidget = pg.PlotWidget(labels = {'left': 'Units (u)', 'bottom': 'Time (s)'},title = 'Quad Values')
      plotWidget.addLegend()
      plotWidget.showGrid(x = True, y = True, alpha = 0.420)
      plotWidget.addItem(curve1)
      # plotWidget.addItem(curve2)
      # plotWidget.plot(time, counter, pen='b',name = 'blueLine' ,clear=True)
      # plotWidget.plot(time, counter2, pen='r', name = 'redLine')

      #addes all of the widgets to a VBox
      vbox = QtGui.QVBoxLayout()
      vbox.addWidget(self.button)
      vbox.addWidget(self.textbox)
      vbox.addWidget(self.comboBox)
      vbox.addWidget(plotWidget)


      #puts the vbox on the window
      self.setLayout(vbox)       
      
      #sends value on display when button is pressed
      self.button.clicked.connect(lambda: self.handleButton(self.textbox.text()))
      
      #sets up the screen size and name
      self.setGeometry(900, 900, 750, 500)
      self.setWindowTitle('#Ground Station GUI#')
      self.show()

      rospy.init_node('gs_gui')
      print "START"    


def main():
  #Sets up window
  global app
  app = QtGui.QApplication(sys.argv)
  ex = Example()

  timer = QtCore.QTimer()
  timer.timeout.connect(update)
  timer.start(100)

  sys.exit(app.exec_())  


if __name__ == "__main__":
  main()   