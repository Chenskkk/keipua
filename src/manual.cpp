#include <ros/ros.h> 
#include <serial/serial.h> 
#include <std_msgs/String.h> 
#include <std_msgs/Empty.h> 
#include <geometry_msgs/Twist.h>
serial::Serial ser; //声明串口对象 
char tmp_X[9];
char tmp_Y[9];
char angular_flag;
//回调函数 
void write_callback(const std_msgs::String::ConstPtr& msg) 
{ 
    ROS_INFO_STREAM("Writing to serial port" <<msg->data); 
    ser.write(msg->data);   //发送串口数据 
} 
static void TwistCallback(const geometry_msgs::TwistConstPtr &msg)
{
    //last_command_time_ = ros::WallTime::now();
    static double linear_speed;
    static double angular_speed;
    static int X=250;
    static int Y=234;    
    angular_speed = msg->angular.z;
    linear_speed = msg->linear.x;

    if(angular_speed<-0.3) angular_flag=-0.3;
    if(angular_speed>0.3) angular_flag=0.3;
    X=(int)(linear_speed*200+250);
    Y=(int)(angular_speed*234/4.0+234);
    ROS_INFO("\n linear: [%f], angular:[%f]", linear_speed,angular_speed);
    sprintf(tmp_X, "%d", X);
    sprintf(tmp_Y, "%d", Y);
    //ROS_INFO("Writing to serial port" << tmp_linear);
    ser.write("X");
    ser.write(tmp_X);
    ser.write("e");
    ser.write("Y");
    ser.write(tmp_Y);
    ser.write("e");
}
int main (int argc, char** argv) 
{ 
    //初始化节点 
    ros::init(argc, argv, "serial_example_node"); 
    //声明节点句柄 
    ros::NodeHandle nh; 
    //订阅主题，并配置
    ros::Subscriber write_sub = nh.subscribe("keipuA/cmd_vel", 1000, TwistCallback);
    //发布主题 
    ros::Publisher read_pub = nh.advertise<std_msgs::String>("read", 1000); 

    try 
    { 
    //设置串口属性，并打开串口 
        ser.setPort("/dev/ttyUSB0");
        //ser.setPort("/dev/ttyACM0");
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