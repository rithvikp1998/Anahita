#include <torpedo.h>

Torpedo::Torpedo(): forwardPIDClient("forwardPID"), sidewardPIDClient("sidewardPID"), 
                        anglePIDClient("turnPID"), upwardPIDClient("upwardPID"), th(120) {}

Torpedo::~Torpedo() {}

bool Torpedo::setActive(bool status) {

    if (status) {

        nh_.setParam("/use_local_yaw", true);
        nh_.setParam("/use_reference_yaw", false);
        nh_.setParam("/enable_pressure", false);

        ROS_INFO("Waiting for sidewardPID server to start, task torpedo.");
        sidewardPIDClient.waitForServer();

        ROS_INFO("sidewardPID server started, sending goal, task torpedo.");
        sidewardPIDgoal.target_distance = 0;
        sidewardPIDClient.sendGoal(sidewardPIDgoal);

        ///////////////////////////////////////////////////

        ROS_INFO("Waiting for upwardPID server to start, task torpedo.");
        upwardPIDClient.waitForServer();

        ROS_INFO("upwardPID server started, sending goal, task torpedo.");
        upwardPIDgoal.target_depth = 0; // for gazebo
        upwardPIDClient.sendGoal(upwardPIDgoal);

        ///////////////////////////////////////////////////

        ROS_INFO("Waiting for anglePID server to start, task torpedo.");
        anglePIDClient.waitForServer();

        ROS_INFO("anglePID server started, sending goal, task torpedo.");

        anglePIDGoal.target_angle = 0;
        anglePIDClient.sendGoal(anglePIDGoal);

        /////////////////////////////////////////////////////

        ROS_INFO("ForwardPID Client sending goal, task torpedo.");        
        forwardPIDgoal.target_distance = 12;
        forwardPIDClient.sendGoal(forwardPIDgoal);

        if (!th.isAchieved(12, 2, "forward")) {
            ROS_INFO("Unable to achieve forward in time limit");
            nh_.setParam("/kill_signal", true);		
            // return false;	
	    }	
        ROS_INFO("Firing torpedo");

        ros::Duration(2).sleep();
	
        // fire the torpedo
        nh_.setParam("/torpedo", 1);
        ros::Duration(1).sleep();
        nh_.setParam("/torpedo", 0);

        forwardPIDClient.cancelGoal();
        ROS_INFO("Killing the thrusters");
	    nh_.setParam("/kill_signal", true);

        ROS_INFO("ForwardPID Client sending goal again, task torpedo.");        
        forwardPIDgoal.target_distance = 25;
        forwardPIDClient.sendGoal(forwardPIDgoal);

        if (!th.isAchieved(25, 4, "forward")) {
            ROS_INFO("Unable to achieve forward in time limit");
            nh_.setParam("/kill_signal", true);
            // return false;	
	    }
        ROS_INFO("Torpedo Finished");
    }
    else {

        nh_.setParam("/use_local_yaw", false);
        
        forwardPIDClient.cancelGoal();
        upwardPIDClient.cancelGoal();
        sidewardPIDClient.cancelGoal();
        anglePIDClient.cancelGoal();
        ROS_INFO("Closing Torpedo");
    }

    return true;
}
