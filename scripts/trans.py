#! /usr/bin/env python
import rospy
from autoware_msgs.msg import CloudClusterArray
import std_msgs
#from cv_bridge import CvBridge, CvBridgeError
#from sensd	or_msgs.msg import Image


class trans:
    def __init__(self):
        self.a_pub=rospy.Publisher("/cloud_clusters",CloudClusterArray,queue_size=100)
        self.a_sub=rospy.Subscriber("/detection/lidar_detector/cloud_clusters",CloudClusterArray,self.callback)
    def callback(self,msg):
        self.a_pub.publish(msg)
            
if __name__=='__main__':
    try:
        rospy.init_node("trans")	
        rospy.loginfo("Starting trans node")
        trans()
        rospy.spin()
    except KeyboardInterrupt:
        print "Shutting down trans node"
