#! /usr/bin/env python
import rospy
import cv2
import numpy as np
from darknet_ros_msgs.msg import BoundingBoxes
import std_msgs
#from cv_bridge import CvBridge, CvBridgeError
#from sensd	or_msgs.msg import Image


class sign_detector:
    def __init__(self):
        self.sign_pub=rospy.Publisher("/sign_stop",std_msgs.msg.Bool,queue_size=1)
        self.cone_pub=rospy.Publisher("/num_of_cone",std_msgs.msg.Int16,queue_size=1)
        self.yolo_sub=rospy.Subscriber("/darknet_ros/bounding_boxes",BoundingBoxes,self.callback_yolo)
    def callback_yolo(self,msg):
        stop=0
        num_of_cone=0
        for i in range(len(msg.bounding_boxes)):
            if(msg.bounding_boxes[i].Class=="stop"):
                stop=1
                rospy.loginfo(msg.bounding_boxes[i].Class) 
                self.sign_pub.publish(stop)
            elif(msg.bounding_boxes[i].Class=="cone"):
                num_of_cone=num_of_cone+1
        rospy.loginfo(str(num_of_cone))
        self.cone_pub.publish(num_of_cone)
            
if __name__=='__main__':
    try:
        rospy.init_node("sign_detector")	
        rospy.loginfo("Starting sign_detector node")
        sign_detector()
        rospy.spin()
    except KeyboardInterrupt:
        print "Shutting down sign_detector node"
