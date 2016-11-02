#!/usr/bin/env python
# license removed for brevity
import rospy
from std_msgs.msg import Int32

def counter():
    number = 0
    pub = rospy.Publisher('counter', Int32, queue_size=10)
    rospy.init_node('talker', anonymous=True)
    rate = rospy.Rate(1) # 1hz
    while not rospy.is_shutdown():
        pub.publish(number)
	number = number + 1
        rate.sleep()

if __name__ == '__main__':
    try:
        counter()
    except rospy.ROSInterruptException:
        pass

