#include <ros/ros.h> 
#include <serial/serial.h>  
#include <std_msgs/String.h> 
#include <std_msgs/Empty.h> 
#include <geometry_msgs/TwistStamped.h>
#include <autoware_msgs/DetectedObjectArray.h>
serial::Serial ser; //声明串口对象 
char tmp_X[9];
char tmp_Y[9];
char angular_flag;
int  obst_flag;
int obst_x;
int obst_y;
int num_of_obst;
//回调函数 
void write_callback(const std_msgs::String::ConstPtr& msg) 
{ 
    ROS_INFO_STREAM("Writing to serial port" <<msg->data); 
    ser.write(msg->data);   //发送串口数据 
} 
static void DetectCallback(const autoware_msgs::DetectedObjectArray &msg)
{
    num_of_obst=0;
    for (int i=0;i<msg.objects.size();++i)

    {
        if(msg.objects[i].pose.position.x>0 && msg.objects[i].pose.position.x<2.5)
        {
            if(msg.objects[i].pose.position.y>-0.5 && msg.objects[i].pose.position.y<0.5)
            {
                num_of_obst++;
                std::cout<< msg.objects[i].pose.position.y <<std::endl;
                if(msg.objects[i].pose.position.y>-0.2 && msg.objects[i].pose.position.y<0.2)
                {
                    obst_flag=1;

                }
                else
                {
                    obst_flag=2;
                    obst_y=msg.objects[i].pose.position.y;
                    obst_x=msg.objects[i].pose.position.x;
                }
            }
        }
        if(num_of_obst==0)
        {
            obst_flag=0;
        }
        ROS_INFO_STREAM("    "<<obst_flag);
       //std::cout << (char)obst_flag << std::endl;
    }
}
static void TwistCallback(const geometry_msgs::TwistStampedConstPtr &msg)
{
    static double linear_speed;
    static double angular_speed;
    static int X=250;
    static int Y=234;    


    //ROS_INFO("Writing to serial port" << tmp_linear);
    if(obst_flag==0)
    {
        angular_speed = msg->twist.angular.z;
        linear_speed = msg->twist.linear.x;

        if(angular_speed<-0.3) angular_speed=-0.3;
        if(angular_speed>0.3) angular_flag=0.3;
        if(linear_speed>0.5) X=370;
        else X=250;
        Y=(int)(angular_speed*250.0/2.0+234);
        ROS_INFO("\n linear: [%f], angular:[%f]", linear_speed,angular_speed);
        sprintf(tmp_X, "%d", X);
        sprintf(tmp_Y, "%d", Y);
      ser.write("X");
      ser.write(tmp_X);
      ser.write("e");
      ser.write("Y");
      ser.write(tmp_Y);
      ser.write("e");
    }
    else if(obst_flag==1)
    {
      ser.write("X");
      ser.write("250");
      ser.write("e");
      ser.write("Y");
      ser.write("234");
      ser.write("e");
    }
    else if(obst_flag==2)
    {
        ser.write("X");
        ser.write("250");
        ser.write("e");
        ser.write("Y");
        ser.write("234");
        ser.write("e");
    }
}
int main (int argc, char** argv) 
{ 
    //初始化节点 
    ros::init(argc, argv, "serial_example_node"); 
    //声明节点句柄 
    ros::NodeHandle nh; 
    //订阅主题，并配置
    ros::Subscriber lidar_det_sub = nh.subscribe("/detection/lidar_detector/objects",1000,DetectCallback);
    ros::Subscriber write_sub = nh.subscribe("twist_cmd", 1000, TwistCallback);
    //发布主题 
    ros::Publisher read_pub = nh.advertise<std_msgs::String>("read", 1000); 

    try 
    { 
    //设置串口属性，并打开串口 
        ser.setPort("/dev/ttyUSB0");
        ser.setBaudrate(9600); 
        serial::Timeout to = serial::Timeout::simpleTimeout(1000); 
        ser.setTimeout(to); 
        ser.open(); 
    } 
    catch (serial::IOException& e) 
    { 
        ROS_ERROR_STREAM("Unable to open port "); 
        return -1; 
    } 

    //检测串口是否已经打开，并给出提示信息 
    if(ser.isOpen()) 
    { 
        ROS_INFO_STREAM("Serial Port initialized"); 
    } 
    else 
    { 
        return -1; 
    } 

    //指定循环的频率 
    ros::Rate loop_rate(5); 
    while(ros::ok()) 
    { 
        if(ser.available()){ 
            ROS_INFO_STREAM("Reading from serial port\n"); 
            std_msgs::String result; 
            result.data = ser.read(ser.available()); 
            ROS_INFO_STREAM("Read: " << result.data); 
            read_pub.publish(result);    
        } 
        ros::spinOnce(); 
        loop_rate.sleep(); 

    } 
} 