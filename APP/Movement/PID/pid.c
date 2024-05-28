#include "pid.h"
#include "my_lib.h"

// float Kp = 35, Ki = 0.4, Kd = 12;
float Kp = 35, Ki = 0.1, Kd = 5.4;
float error = 0, P = 0, I = 0, D = 0, PID_value = 0;
float previous_error = 0;

void PidData_Clear(void)
{
    I = 0;
    D = 0;
    PID_value = 0;
    previous_error = 0;
}

void PidData_Set(float error, float value)
{
    I = 0;
    D = 0;
    PID_value = value;
    previous_error = error;
}

void Calculate_pid(float inputError)
{
    error = inputError;
    P = error;
    I = I + error;
    D = error - previous_error;

    I = constrain_float(I, -200, 200); // 积分限幅

    PID_value = (Kp * P) + (Ki * I) + (Kd * D);
    PID_value = constrain_float(PID_value, -180, 180);

    previous_error = error;
}
