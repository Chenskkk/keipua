#! /usr/bin/env python
import rospy
import cv2
import numpy as np
import std_msgs
from cv_bridge import CvBridge, CvBridgeError
from sensor_msgs.msg import Image
from autoware_msgs.msg import DetectedObjectArray,TrafficLight
lower_green=np.array([41,40,40])
upper_green=np.array([90,255,255])
# lower_red=np.array([170,100,150])
# upper_red=np.array([179,255,255])
lower_red=np.array([170,40,40])
upper_red=np.array([179,255,255])
x=0
y=0
height=0
width=0
detected=0
class image_converter:
    def __init__(self):
        self.sign_pub=rospy.Publisher("/sign_stop",std_msgs.msg.Bool,queue_size=1)
        self.cone_pub=rospy.Publisher("/num_of_cone",std_msgs.msg.Int16,queue_size=1)
        self.image_pub=rospy.Publisher("cv_bridge_image", Image ,queue_size=1)
        self.tlr_pub=rospy.Publisher("/light_color",TrafficLight,queue_size=1)
        self.bridge=CvBridge()
        self.image_sub=rospy.Subscriber("/image_raw",Image,self.callback)
        self.yolo_sub=rospy.Subscriber("/detection/image_detector/objects",DetectedObjectArray,self.callback_yolo)
    def callback(self,data):
        global x
        global y
        global height
        global width
        global detected
        try:
            cv_image=self.bridge.imgmsg_to_cv2(data, "bgr8")
        except CvBridgeError as e:
            print e
        #  cv2.imshow("Image window",cv_image)
        img_trl=cv_image[y-3:y+height+3,x-3:x+width+3]
        hsv=cv2.cvtColor(img_trl,cv2.COLOR_BGR2HSV)
        if(detected):            
            mask_r=cv2.inRange(hsv,lower_red,upper_red)
            mask_r=cv2.dilate(mask_r,None,iterations=2)
            mask_r=cv2.erode(mask_r,None,iterations=2)
            mask_g=cv2.inRange(hsv,lower_green,upper_green)
            mask_g=cv2.dilate(mask_g,None,iterations=2)
            mask_g=cv2.erode(mask_g,None,iterations=2)
            # cv2.imshow("mask",mask_r)
            # cv2.imshow("mask_g",mask_g)
            res_r=cv2.bitwise_and(img_trl,img_trl,mask=mask_r)
            res_g=cv2.bitwise_and(img_trl,img_trl,mask=mask_g)
            cv2.imshow("res_r",res_r)
            cv2.imshow("res_g",res_g)
            cnts_g=cv2.findContours(mask_g.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)[-2]
            cnts_r=cv2.findContours(mask_r.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)[-2]

            center=None
            if len(cnts_g)>0:
                self.tlr_pub.publish(traffic_light=1)
                rospy.loginfo("light color: Green") 
            elif len(cnts_r)>0:
                self.tlr_pub.publish(traffic_light=0)
                rospy.loginfo("light color: Red") 
            else:
                self.tlr_pub.publish(traffic_light=2)
                rospy.loginfo("light color: No signal detected")
                # c=max(cnts,key=cv2.contourArea)
                # ((x,y),radius)=cv2.minEnclosingCircle(c)
                # M=cv2.moments(c)
                # center=(int(M["m10"]/M["m00"]),int(M["m01"]/M["m00"]))
                # if radius >2:
                #     cv2.circle (img_trl,(int(x),int(y),int(radius),(0,255,255),2))
                #     cv2.circle(img_trl,center,5,(0,0,255),-1)
            detected=0

        cv2.waitKey(5)
        #try:
        #    self.image_pub.publish(self.bridge.cv2_to_imgmsg(cv_image,"bgr8"))
        #except CvBridgeError as e:
        #    print e
    def callback_yolo(self,msg):
        global x
        global y
        global height
        global width
        global detected
        stop=0
        num_of_cone=0        
        for i in range(len(msg.objects)):
            if(msg.objects[i].label=="traffic light" and msg.objects[i].height>msg.objects[i].width):
                detected=1
                height=msg.objects[i].height
                width=msg.objects[i].width
                x=msg.objects[i].x
                y=msg.objects[i].y
            elif(msg.objects[i].label=="cone"):
                num_of_cone=num_of_cone+1
            elif(msg.objects[i].label=="stop"):
                stop=1
                rospy.loginfo(msg.objects[i].label) 
                self.sign_pub.publish(stop)
        rospy.loginfo("num of cone(s):"+str(num_of_cone))
        self.cone_pub.publish(num_of_cone)
        rospy.loginfo("detecting~~~")
        if detected==0:
            self.tlr_pub.publish(traffic_light=2)
            
if __name__=='__main__':
    try:
        rospy.init_node("tlr_detector")	
        rospy.loginfo("*****Starting tlr_detector node*****")
        rospy.loginfo("******Waiting for yolo detector******")
        image_converter()
        rospy.spin()
    except KeyboardInterrupt:
        print "Shutting down tlr_detector node"
        cv2.destroyAllWindows()
