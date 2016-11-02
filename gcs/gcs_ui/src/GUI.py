#!/usr/bin/env python

#ROS imports
import rosbag
import rospy
from std_msgs.msg import Int32, String
from geometry_msgs.msg import Point

#PyQt Imports
import sys
from PyQt4 import QtGui, QtCore

#PyGraph Imports
import time
import pyqtgraph as pg
import numpy as np

#Global Variables TODO
global app
global colors
global topics 
colors = ['y','r','b','g','p']
topics = ["/counter", "/Quad_RPY"]


#defenitions TODO
topicName = "/counter"
other_topicname = "/Quad_RPY"

#Arrays for Graph
num = 0

class Curve():
  def __init__(self, color):
    self.color = color
    self.dataY = []
    self.dataX = []
    self.curve1 = pg.PlotCurveItem(self.dataX, self.dataY, pen=self.color, name = 'Data')
    # curve2 = pg.PlotCurveItem(time, counter2, pen='r', name = 'redLine')
    self.curve1.setClickable(True, width=10)

  def getCurve(self):
    return self.curve1

  def update(self):
    self.curve1.setData(self.dataX, self.dataY, pen = self.color)

  def getDataY(self):
    return self.dataY

  def getDataX(self):
    return self.dataX 

  def getColor(self):
    return color

  def appendDataY(self, data):
    self.dataY.append(data)

  def appendDataX(self, data):
    self.dataX.append(data)  

  def Copy(self, curve):
    self.dataX = curve.getDataX
    self.dataY = curve.getDataY
    self.color = curve.getColor
    self.curve1 = pg.PlotCurveItem(self.dataX, self.dataY, pen=self.color, name = 'Data')
    self.curve1.setClickable(True, width=10)


class RosPlot():
  def __init__(self, num, name):
    global colors 
    self.amount = num
    self.plots = []
    self.perservedCurves = []
    self.name = name
    for i in range(num):
      self.plots.append(Curve(colors[i]))
    #self.plotWidget = None
    self.createPlot()
  
  def createPlot(self):
    self.plotWidget = pg.PlotWidget(labels = {'left': 'Units (u)', 'bottom': 'Sample (n)'},title = self.name)
    self.plotWidget.addLegend()
    self.plotWidget.showGrid(x = True, y = True, alpha = 0.420)
    for i in range(self.amount):
      self.plotWidget.addItem(self.plots[i].getCurve())

  def getPlot(self):
    return self.plotWidget  

  def deletePlot(self):
    # for i in range(self.amount):
    #   self.perservedCurves[i].Copy(self.plots[i])
    #   self.plots[i].deleteLater()
    #   self.plots[i] = None

    self.plotWidget.deleteLater()
    self.plotWidget = None

  def appendDataY(self, data, num):
    self.plots[num].appendDataY(data)

  def appendDataX(self, data, num):
    self.plots[num].appendDataX(data)

  # def getDataY(self):
  #   return self.dataY

  # def getDataX(self):
  #   return self.dataX

  def update(self):
    for i in range(self.amount):
      self.plots[i].update()





class RosBag():

  def __init__(self):
    self.initBag()

  def changeName(self, newName):
    self.bagName = newName
    

  def initWBag(self):
    #WritableBag
    self.bag = rosbag.Bag(self.bagName + '.bag', 'w')

  def initRBag(self):
    #ReadableBag
    self.bag = rosbag.Bag(self.bagName + '.bag')  

  def writeBag(self, msg):
    self.bag.write('r', msg)

  def closeBag(self):
    self.bag.close()

  def showBag(self):
    self.initRBag()
    for topic, msg, t in self.bag.read_messages(topics=['r']):
     print msg.data  
    self.closeBag()

  def initBag(self):
   self.bag = rosbag.Bag('null.bag', 'w') 


class RosSubscriber():

  def __init__(self, name):
    self.topic = name
    self.initNode(name)
    self.rosBag = RosBag()
    if self.topic == "/counter":
      self.plotWidget = RosPlot(1, 'Counter - Debugging')
    elif self.topic == "/Quad_RPY":
      self.plotWidget = RosPlot(3, 'QuadRPY')
    #else:


    self.num = 0 

  def pushed(self):
    self.recording = not self.recording  

  def changeBagName(self, newName):
    self.rosBag.changeName(newName + "-" + self.topic[1:])
    

  def initWBag(self):
    #WritableBag
    self.rosBag.initWBag()

  def writeBag(self, msg):
    self.rosBag.writeBag(msg)

  def showBag(self):
    self.rosBag.showBag()

  def closeBag(self):
    self.rosBag.closeBag()  

  def update(self): 
    # plotWidget.plot(time, counter, pen='b',name = 'blueLine' ,clear=True)
    # plotWidget.plot(time, counter2, pen='r', name = 'redLine')

    self.plotWidget.update()
    # curve2.setData(time, counter2, pen = 'r')
    app.processEvents()


  def callback(self, msg):
    if self.topic == "/counter":
      self.plotWidget.appendDataY(msg.data, 0)
      self.plotWidget.appendDataX(self.num, 0)
    elif self.topic == "/Quad_RPY":
      self.plotWidget.appendDataY(msg.x, 0)
      self.plotWidget.appendDataX(self.num, 0)
      self.plotWidget.appendDataY(msg.y, 1)
      self.plotWidget.appendDataX(self.num, 1)
      self.plotWidget.appendDataY(msg.z, 2)
      self.plotWidget.appendDataX(self.num, 2)
    #else:


    self.num +=1 
    if self.recording:
      self.writeBag(msg)

  def initNode(self, name):
    rospy.init_node('gs_gui')

    #Different Type of Allowed Nodes
    if self.topic == "/counter":
      self.sub = rospy.Subscriber(name, Int32, self.callback)
    elif self.topic == "/Quad_RPY":
      self.sub = rospy.Subscriber(name, Point, self.callback)
    else:
      print("not a valid topic")  
        
    self.recording = False  

  def getPlot(self):
    return self.plotWidget.getPlot()   

  def createPlot(self):
    self.plotWidget.createPlot()  

  def deletePlot(self):
    self.plotWidget.deletePlot()  

class Gui(QtGui.QWidget):

    def __init__(self):
        global topics
        super(Gui, self).__init__()   #Calls parent constructor
        self.subscriber = []
      
        for topic in topics:
          self.subscriber.append(RosSubscriber(topic))

        self.initUI()                     #Calls bottom

    def pushed(self):
      global topics
      self.recording = not self.recording

      for n in range(len(topics)):
        self.subscriber[n].pushed()    

    def handleButton(self, text):
      global topics

      if not self.recording:   #actually means if recording since it checks before the boolean is inverted
        for n in range(len(topics)):
          self.subscriber[n].changeBagName(self.textbox.text())
          self.subscriber[n].initWBag()

      self.pushed()
      print "Pushed"

      self.button.setText('Stop')
      self.button.setStyleSheet("background-color:#CCCCCC;")

      if not self.recording:
        for n in range(len(topics)): 
          self.subscriber[n].closeBag()
        #self.subscriber.showBag()
        self.button.setText('Record')
        self.button.setStyleSheet("background-color:#FFFFFF;")

      QtGui.QApplication.processEvents()
      
    def itemChanged(self, item):
      if item.checkState():
        print item.text()
        print item.row()

      if item.checkState():
        self.subscriber[item.row()].createPlot()
        self.vbox.addWidget(self.subscriber[item.row()].getPlot())


      if not item.checkState(): 
        self.vbox.removeWidget(self.subscriber[item.row()].getPlot())
        self.subscriber[item.row()].deletePlot()
    

    def update(self):
      global topics

      for n in range(len(topics)):
        self.subscriber[n].update()  
        
    def initUI(self):
      global topics

      # #Menu
      # menubar = self.menuBar()
      # topicsMenu = menubar.addMenu('&Topics')


      #Button
      self.recording = False
      self.button = QtGui.QPushButton('Record', self)
      self.button.setStyleSheet("background-color:#FFFFFF;")
      
      #TextBox
      self.textbox = QtGui.QLineEdit('TypeBagNameHere',self)

      ####ComboCheckBox TODO [FIX]

      model = QtGui.QStandardItemModel()

      for topic in topics:                   
        item = QtGui.QStandardItem('%s' % topic)
        check = QtCore.Qt.Checked
        item.setCheckState(check)
        item.setCheckable(True)
        model.appendRow(item)

      model.itemChanged.connect(self.itemChanged)  

      self.listView = QtGui.QListView()
      self.listView.setModel(model)
      self.listView.setGeometry(40,80,100,100)
    
      #graph
      # plotWidget.addItem(curve2)
      # plotWidget.plot(time, counter, pen='b',name = 'blueLine' ,clear=True)
      # plotWidget.plot(time, counter2, pen='r', name = 'redLine')

      #addes all of the widgets to a VBox
      self.vbox = QtGui.QVBoxLayout()
      self.vbox.addWidget(self.button)
      self.vbox.addWidget(self.textbox)
      self.vbox.addWidget(self.listView)

      for n in range(len(topics)):
        self.vbox.addWidget(self.subscriber[n].getPlot())


      #puts the vbox on the window
      self.setLayout(self.vbox)       
      
      #sends value on display when button is pressed
      self.button.clicked.connect(lambda: self.handleButton(self.textbox.text()))

      print "START"    

class Window(QtGui.QMainWindow):
  def __init__(self):
    super(Window, self).__init__()
    self.initUI()

  def update(self):
     self.gui.update()

  def initUI(self):               
     #Menu Actions
     exitAction = QtGui.QAction(QtGui.QIcon('exit.png'), '&Exit', self)        
     exitAction.setShortcut('Ctrl+Q')
     exitAction.setStatusTip('Exit application')
     exitAction.triggered.connect(QtGui.qApp.quit)

     #topicAction = QtGui.QAction(QtGui.QIcon('exit.png'), '&Add/Edit Topics', self)

     #Make a MenuBar
     menubar = self.menuBar()        
     menubar.setToolTip('This is a <b>QWidget</b> for MenuBar')

     #Menu Options
     fileMenu = menubar.addMenu('&File')
     #editMenu = menubar.addMenu('&Edit')

     #Add Actions 
     fileMenu.addAction(exitAction)
     #editMenu.addAction(topicAction)
     toolbar = self.addToolBar('Exit')

     # create the widget here
     self.gui = Gui() 
     self.setCentralWidget(self.gui)
     self.setGeometry(1200, 1200, 600, 1000)
     self.setWindowTitle('#Ground Station GUI#')
     self.show()

def main():
  #Sets up window
  global app
  app = QtGui.QApplication(sys.argv)
  gui = Window()

  timer = QtCore.QTimer()
  timer.timeout.connect(gui.update)
  timer.start(100)


  sys.exit(app.exec_())  


if __name__ == "__main__":
  main()   