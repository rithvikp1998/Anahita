#include <ros/ros.h>

#include <gate.h>
#include <single_buoy.h>
#include <torpedo.h>
#include <marker_dropper.h>
#include <octagon.h>
#include <line.h>

#include <straight_server.h>
#include <move_sideward_server.h>
#include <move_downward_server.h>
#include <depth_stabilise.h>

#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>

#include <motion_layer/anglePIDAction.h>
#include <motion_layer/rollPIDAction.h>
#include <motion_layer/pitchPIDAction.h>
#include <motion_layer/forwardPIDAction.h>
#include <motion_layer/sidewardPIDAction.h>
#include <motion_layer/upwardPIDAction.h>

#include <std_msgs/String.h>
#include <boost/thread.hpp>

#include <task_handler.h>
#include <navigation_handler.h>

using namespace std;

void spinThread() {
    ROS_INFO("Spinning");
    ros::spin();
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "master");
    ros::NodeHandle nh;
    ros::Publisher task_pub = nh.advertise<std_msgs::String>("/current_task", 1); 
    ros::Time::init();

    taskHandler th(30); // time out 15 seconds

    navigationHandler nav_handle;

    boost::thread spin_thread(&spinThread);
    
    singleBuoy single_buoy;
    lineTask line;
    gateTask gate_task;
    Torpedo torpedo;
    Octagon octagon;

    moveSideward move_sideward(0);
    moveStraight move_straight(0);
    moveDownward move_downward(0);
    depthStabilise depth_stabilise;

    actionlib::SimpleActionClient<motion_layer::forwardPIDAction> forwardPIDClient("forwardPID");
    actionlib::SimpleActionClient<motion_layer::upwardPIDAction> upwardPIDClient("upwardPID");
    actionlib::SimpleActionClient<motion_layer::sidewardPIDAction> sidewardPIDClient("sidewardPID");
    actionlib::SimpleActionClient<motion_layer::anglePIDAction> anglePIDClient("turnPID");

    motion_layer::sidewardPIDGoal sidewardPIDGoal;
    motion_layer::forwardPIDGoal forwardPIDGoal;
    motion_layer::upwardPIDGoal upwardPIDGoal;
    motion_layer::anglePIDGoal anglePIDGoal;

    /////////////////////////////////////////////

    // Random code to test

    /////////////////////////////////////////////

    // ros::Duration(60).sleep();

    nh.setParam("/set_reference_yaw", true);
    nh.setParam("/set_local_yaw", true);

    if (nav_handle.find("line", 20)) {
        if (!line.setActive(true)) {
            ROS_INFO("Line Task Failed");
            // line.setActive(false);
            // return 1;
        }
        line.setActive(false);
    }

    nh.setParam("/set_local_yaw", true);

    depth_stabilise.setActive(true, "reference");

    nh.setParam("/current_task", "red_buoy");
    ROS_INFO("Current task: Red Buoy");

    move_straight.setThrust(50);
    move_straight.setActive(true, "local");

    ros::Duration(3).sleep();

    move_straight.setActive(false, "local");

    if (!th.isDetected("red_buoy", 20)) {
        ROS_INFO("Unable to detect red buoy");
        // return 1;
    }
    depth_stabilise.setActive(false, "reference");

    bool red_buoy = false;

    nh.setParam("/use_reference_yaw", true);
    red_buoy = single_buoy.setActive(true);
    if (!red_buoy) {
        ROS_INFO("Red Buoy Failed");
    }
    else {
        ROS_INFO("Completed the first buoy task, Now let's move on to the second");
    }
    single_buoy.setActive(false);
    nh.setParam("/use_reference_yaw", false);
        
    ///////////////////////////////////////////////

    nh.setParam("/current_task", "yellow_buoy");
    ROS_INFO("Current task: Yellow Buoy");

    move_sideward.setThrust(50);
    move_sideward.setActive(true, "reference");
    depth_stabilise.setActive(true, "reference");

    ros::Duration(10).sleep();

    ROS_INFO("Finding Yellow Buoy....");
    if (!th.isDetected("yellow_buoy", 15)){
        ROS_INFO("Failed to detect yellow buoy");
        // move_sideward.setActive(false, "reference");
        // return 1;
    }
    ROS_INFO("Yellow Buoy Detected");
    move_sideward.setActive(false, "reference");
    depth_stabilise.setActive(false, "reference");

    nh.setParam("/use_reference_yaw", true);
    if (!single_buoy.setActive(true)) {
        ROS_INFO("Yellow Buoy Failed");
        single_buoy.setActive(false);
        return 1;
    }
    single_buoy.setActive(false);
    ROS_INFO("Yellow Buoy done");
    
    //////////////////////////////////////////////

    nh.setParam("/current_task", "green_buoy");
    ROS_INFO("Current task: Green Buoy");

    // nh.setParam("/use_reference_yaw", true);
    depth_stabilise.setActive(true, "reference");

    // ROS_INFO("Waiting for anglePID server to start.");
    // anglePIDClient.waitForServer();

    // ROS_INFO("anglePID server started, sending goal.");
    // anglePIDGoal.target_angle = -75;
    // anglePIDClient.sendGoal(anglePIDGoal);

    // th.isAchieved(-75, 2, "angle");
    // anglePIDClient.cancelGoal();
    // nh.setParam("/use_reference_yaw", false);

    nh.setParam("/pwm_yaw", 20);

    if (!th.isDetected("green_buoy", 20)) {
        ROS_INFO("Unable to detect green buoy");
        // return 1;
    }
    nh.setParam("/pwm_yaw", 0);
    ROS_INFO("Green Buoy Detected");
    nh.setParam("/set_local_yaw", true);

    move_straight.setThrust(50);

    move_straight.setActive(true, "local");
    ros::Duration(5).sleep(); // configurable 
    move_straight.setActive(false, "local");

    ROS_INFO("Finding Green Buoy...");
    depth_stabilise.setActive(false, "reference");
    
    if (!single_buoy.setActive(true)) {
        ROS_INFO("Green Buoy Failed");
        // single_buoy.setActive(false);
        // return 1;
    }
    single_buoy.setActive(false);
    ROS_INFO("Green Buoy Done");

    //////////////////////////////////////////////

    nh.setParam("/use_reference_yaw", true);
    depth_stabilise.setActive(true, "reference");
    ROS_INFO("Waiting for anglePID server to start.");
    anglePIDClient.waitForServer();

    ROS_INFO("anglePID server started, sending goal.");
    anglePIDGoal.target_angle = 0;
    anglePIDClient.sendGoal(anglePIDGoal);

    th.isAchieved(0, 2, "angle");
    anglePIDClient.cancelGoal();
    nh.setParam("/use_reference_yaw", false);
    nh.setParam("/set_local_yaw", true);

    //////////////////////////////////////////////

    move_sideward.setThrust(50);
    move_sideward.setActive(true, "reference"); // until gate is detected
    ros::Duration(7).sleep(); // time depends, better to have a node telling when the gate is detected

    move_sideward.setActive(false, "reference");
    depth_stabilise.setActive(false, "reference");

    bool gate_found = false;

    if (nav_handle.find("line", 20)) {
        gate_found = true;
        if (!line.setActive(true)) {
            ROS_INFO("Line Task Failed");
            // line.setActive(false);
            // return 1;
        }
        line.setActive(false);
    }
    else {
        ROS_INFO("Nav Handle: Line not found, for gate task");    

        nh.setParam("/current_task", "gate_front");
        ROS_INFO("Current Task: Gate Front");

        depth_stabilise.setActive(true, "reference");

        if (!th.isDetected("gate_front", 15)) {
            ROS_INFO("Unable to detect Gate");
            // return 1;
        }
        ROS_INFO("Gate detected");

        depth_stabilise.setActive(false, "reference");

        gate_found = true;
        if (!gate_task.setActive(true)) {
            ROS_INFO("Gate Task Unsuccessful");
            // gate_task.setActive(false);
            // return 1;
        }
        gate_task.setActive(false);
    }

    if (gate_found) {
        depth_stabilise.setActive(true, "reference");

        nh.setParam("/current_task", "gate_bottom");        
        nh.setParam("/use_local_yaw", true);

        move_straight.setThrust(50);
        move_straight.setActive(true, "local");

        ros::Duration(6).sleep();

        if (!th.isDetected("gate_bottom", 30)) {
            ROS_INFO("Unable to detect gate's bottom");
            // move_straight.setActive(false, "local");
            // return 1;
        }
        move_straight.setActive(false, "local");

        depth_stabilise.setActive(false, "reference");
    }

    ///////////////////////////////////////////////////

    // Gate-Torpedo Transition

    depth_stabilise.setActive(true, "reference");

    nh.setParam("/pwm_yaw", 20);

    nh.setParam("/current_task", "green_torpedo");
    ROS_INFO("Current task: Green Torpedo");

    ros::Duration(4).sleep();

    th.isDetected("green_torpedo", 15);
    nh.setParam("/set_local_yaw", true);

    depth_stabilise.setActive(false, "reference");

    ROS_INFO("Green Torpedo detected");

    double found_torpedo = false;

    if (nav_handle.find("line", 20)) {
        found_torpedo = true;
        ROS_INFO("Found the line for Torpedo");
        if (!line.setActive(true)) {
            ROS_INFO("Line Task Failed");
            // line.setActive(false);
            // return 1;
        }
        line.setActive(false);
        ROS_INFO("Completed the line task for torpedo");
    }
    else {
        ROS_INFO("Nav Handle: Line not found for torpedo task");

        found_torpedo = th.isDetected("green_torpedo", 10);
        if (!found_torpedo) {
            ROS_INFO("Green Torpedo Not detected");
            return 1;
        }
        else {
            nh.setParam("/set_local_yaw", true);
        }

    }

    if (found_torpedo) {
        nh.setParam("/current_task", "green_torpedo");
        ROS_INFO("Current task: Green Torpedo");

        ROS_INFO("Finding Green Torpedo....");

        if (!th.isDetected("green_torpedo", 5)) {
            ROS_INFO("Unable to detect green torpedo");
            // return 1;
        }

        torpedo.setActive(true);
        torpedo.setActive(false);

        // nh.setParam("/current_task", "red_torpedo");
        // ROS_INFO("Current task: Red Torpedo");

        // move_sideward.setThrust(-50);
        // move_sideward.setActive(true, "local");

        // if (!th.isDetected("red_torpedo", 5)) {
        //     ROS_INFO("Unable to detect Red Torpedo");
        //     move_sideward.setActive(false, "local");
        //     return 1;
        // }

        // move_sideward.setActive(false, "local");

        // torpedo.setActive(true);
        // torpedo.setActive(false);
    }

    /////////////////////////////////////////////////////

    // Gate-MarkerDropper Transition

    // nh.setParam("/use_reference_yaw", true);
    // depth_stabilise.setActive(true, "reference");
    // ROS_INFO("Waiting for anglePID server to start.");
    // anglePIDClient.waitForServer();

    // ROS_INFO("anglePID server started, sending goal.");
    // anglePIDGoal.target_angle = -60;
    // anglePIDClient.sendGoal(anglePIDGoal);

    // th.isAchieved(-60, 2, "angle");
    // anglePIDClient.cancelGoal();
    // nh.setParam("/use_reference_yaw", false);

    // depth_stabilise.setActive(false, "reference");

    // if (nav_handle.find("line", 20)) {
    //     if (!line.setActive(true)) {
    //         ROS_INFO("Line Task Failed");
    //         line.setActive(false);
    //         return 1;
    //     }
    //     line.setActive(false);
    // }
    // anglePIDGoal.target_angle = 175;
    // anglePIDClient.sendGoal(anglePIDGoal);

    // th.isAchieved(175, 2, "angle");
    // anglePIDClient.cancelGoal();
    // nh_.setParam("/use_local_yaw", false);

    // if (nav_handle.find("line")) {
    //     if (!line.setActive(true)) {
    //         ROS_INFO("Line Task Failed");
    //         line.setActive(false);
    //         return 1;
    //     }
    //     line.setActive(false);
    // }
    // else {
    //     ROS_INFO("Nav Handle: Line not found");    
    // }

    // move_sideward.setThrust(-50);
    // move_sideward.setActive(true, "local");
    // ros::Duration(3).sleep();
    // move_sideward.setActive(false, "local");

    // move_straight.setThrust(-50);
    // move_straight.setActive(true, "local");
    // if (!th.isDetected("line", 10)) {
    //     ROS_INFO("line not detected");
    //     move_straight.setActive(false, "local");
    //     return 1;
    // }
    // ROS_INFO("Line Detected");
    // move_straight.setActive(false, "local");

    // if (!line.setActive(true)) {
    //     ROS_INFO("Line Task Failed");
    //     line.setActive(false);
    //     return 1;
    // }
    // line.setActive(false);

    // ROS_INFO("Completed the Line task");

    // // After finishing torpedo task turn 120 degree anticlockwise and then move straight
    // ROS_INFO("Master layer, anglePID server started, sending goal.");

    // nh.setParam("/use_reference_yaw", true);

    // anglePIDGoal.target_angle = -90;
    // anglePIDClient.sendGoal(anglePIDGoal);

    // if (!th.isAchieved(-90, 2, "angle")) {
    //     ROS_INFO("Not Rotated 90 degrees");
    //     anglePIDClient.cancelGoal();
    //     return 1;
    // }

    // anglePIDClient.cancelGoal();
    // nh.setParam("/use_reference_yaw", false);

    // move_straight.setThrust(75);
    // move_straight.setActive(true, "current");
    // ros::Duration(6).sleep();
    // move_straight.setActive(false, "current");

    /////////////////////////////////////////////////////

    // MarkerDropper

    // nh.setParam("/current_task", "marker_dropper_front");
    // ROS_INFO("Current task: Marker Dropper Front");

    // if (!th.isDetected("marker_dropper_front", 5)) {
    //     ROS_INFO("Marker Dropper Detected");
    //     return 1;
    // }

    // MarkerDropper md;
    // md.setActive(true);
    // md.setActive(false);

    /////////////////////////////////////////////////////

    // Torpedo - Octagon transition

    move_sideward.setThrust(-50);
    move_sideward.setActive(true, "local");
    ros::Duration(7).sleep();
    move_sideward.setActive(false, "local");

    if (nav_handle.find("line", 20)) {
        if (!line.setActive(true)) {
            ROS_INFO("Line Task Failed");
            // line.setActive(false);
            // return 1;
        }
        line.setActive(false);
    }
    else {
        ROS_INFO("Nav Handle: Line not found");    
    }

    /////////////////////////////////////////////////////

    // Marker Dropper - Octagon transition

    // nh.setParam("/current_task", "line");
    // ROS_INFO("Current task: Line");

    // move_straight.setThrust(50);
    // move_straight.setActive(true, "local");

    // if (!th.isDetected("line", 10)) {
    //     ROS_INFO("Line not detected");
    //     move_straight.setActive(false, "local");
    //     return 1;
    // }
    // move_straight.setActive(false, "local");

    // if (!line.setActive(true)) {
    //     ROS_INFO("Line not done");
    //     line.setActive(false);
    //     return 1;
    // }
    // line.setActive(false);

    // nh.setParam("/set_local_yaw", true);

    /////////////////////////////////////////////////////////

    // Octagon

    nh.setParam("/current_task", "octagon");
    ROS_INFO("Current task: Octagon");

    move_straight.setThrust(50);
    move_straight.setActive(true, "current");

    if (!th.isDetected("octagon", 10)) {
        ROS_INFO("Not able to detect octagon");
        // move_straight.setActive(false, "current");
        // return 1;
    }

    // if (nav_handle.find("octagon")) {
    //     if (!octagon.setActive(true)) {
    //         ROS_INFO("Line Task Failed");
    //         octagon.setActive(false);
    //         return 1;
    //     }
    //     octagon.setActive(false);
    // }
    // else {
    //     ROS_INFO("Nav Handle: Octagon not found");    
    // }

    move_straight.setActive(false, "current");

    octagon.setActive(true);
    octagon.setActive(false);
    
    /////////////////////////////////////////////////////
   
    nh.setParam("/kill_signal", true);

    spin_thread.join();

    return 0;
}
