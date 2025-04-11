static bool 
DAC960_InitializeController(DAC960_Controller_T *Controller)
{
  if (DAC960_ReadControllerConfiguration(Controller) &&
      DAC960_ReportControllerConfiguration(Controller) &&
      DAC960_CreateAuxiliaryStructures(Controller) &&
      DAC960_ReadDeviceConfiguration(Controller) &&
      DAC960_ReportDeviceConfiguration(Controller) &&
      DAC960_RegisterBlockDevice(Controller))
    {
      /*
	Initialize the Monitoring Timer.
      */
      setup_timer(&Controller->MonitoringTimer,
                  DAC960_MonitoringTimerFunction, (unsigned long)Controller);
      Controller->MonitoringTimer.expires =
	jiffies + DAC960_MonitoringTimerInterval;
      add_timer(&Controller->MonitoringTimer);
      Controller->ControllerInitialized = true;
      return true;
    }
  return false;
}