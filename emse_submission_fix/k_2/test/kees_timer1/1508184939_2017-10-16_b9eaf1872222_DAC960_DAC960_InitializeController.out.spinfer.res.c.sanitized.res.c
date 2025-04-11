static bool DAC960_InitializeController(DAC960_Controller_T *Controller)
{
  if (DAC960_ReadControllerConfiguration(Controller) && DAC960_ReportControllerConfiguration(Controller) && DAC960_CreateAuxiliaryStructures(Controller) && DAC960_ReadDeviceConfiguration(Controller) && DAC960_ReportDeviceConfiguration(Controller) && DAC960_RegisterBlockDevice(Controller))
    {

      init_timer(&Controller->MonitoringTimer);
      Controller->MonitoringTimer.expires = jiffies + DAC960_MonitoringTimerInterval;
      Controller->MonitoringTimer.data = (unsigned long)Controller;
      Controller->MonitoringTimer.function = DAC960_MonitoringTimerFunction;
      add_timer(&Controller->MonitoringTimer);
      Controller->ControllerInitialized = true;
      return true;
    }
  return false;
}