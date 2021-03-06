#include <roll_PID_server.h>

rollPIDAction::rollPIDAction(std::string name) :
    as_(nh_, name, false),
    action_name_(name), roll("ROLL")
{
    // register the goal and feeback callbacks
    as_.registerGoalCallback(boost::bind(&rollPIDAction::goalCB, this));
    as_.registerPreemptCallback(boost::bind(&rollPIDAction::preemptCB, this));
    goal_ = 0;

    // subscribe to the data topic of interest
    sub_ = nh_.subscribe("/mavros/imu/roll", 1, &rollPIDAction::callBack, this);

    roll.setPID(2.0, 0, 0.65, 1);

    as_.start();
    ROS_INFO("roll_PID_server Initialised");
}

rollPIDAction::~rollPIDAction(void)
{
}

void rollPIDAction::goalCB()
{
    ROS_INFO("Inside goal callback");
    goal_ = as_.acceptNewGoal()->target_roll;

    roll.setReference(goal_);
    ROS_INFO("Goal set as referecnce");

    // publish info to the console for the user
    ROS_INFO("%s: Executing, got a target of %f", action_name_.c_str(), goal_);
}

void rollPIDAction::preemptCB()
{
    ROS_INFO("%s: Preempted", action_name_.c_str());
    // set the action state to preempted
    as_.setPreempted();
}

void rollPIDAction::callBack(const std_msgs::Float32::ConstPtr& msg)
{
    // make sure that the action hasn't been canceled
    if (!as_.isActive()) {
        return;
    }

    roll.errorToPWM(msg->data);

    feedback_.current_roll = msg->data;
    as_.publishFeedback(feedback_);

    // if (msg->data <= goal_ + 0.2 && msg->data >= goal_ - 0.2) {
        // ROS_INFO("%s: Succeeded", action_name_.c_str());
        // set the action state to succeeded
        // as_.setSucceeded(result_);
    // }

    nh_.setParam("/pwm_roll", roll.getPWM());
}
