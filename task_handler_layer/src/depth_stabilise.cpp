#include <depth_stabilise.h>

depthStabilise::depthStabilise() : upwardPIDClient("upwardPID"), move_straight(0) {}

depthStabilise::~depthStabilise() {
}

void depthStabilise::setActive(bool status, std::string type) {
    if (status) {
        nh.setParam("/enable_pressure", true);
        if (type == "reference") {
            nh.setParam("/use_reference_depth", true);
        }
        spin_thread = new boost::thread(boost::bind(&depthStabilise::spinThread, this));
    }
    else {
 
        nh.setParam("/enable_pressure", false);
        nh.setParam("/use_reference_depth", false);
        
        upwardPIDClient.cancelGoal();
        
        spin_thread->join();
        nh.setParam("/kill_signal", true);
    }
}   

void depthStabilise::spinThread() {

    ROS_INFO("Waiting for upwardPID server to start, depth stabilise");
    upwardPIDClient.waitForServer();

    ROS_INFO("upwardPID server started, sending goal.");
    upward_PID_goal.target_depth = 0;
    upwardPIDClient.sendGoal(upward_PID_goal);        
}
