struct switch {
   char* id;   
   int gpio;
};

void toggleSwitch(int number);
void initializeSwitches(switch item);
