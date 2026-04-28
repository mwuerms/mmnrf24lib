# mmnrf24lib
nrf24 lib

## include/add to STM32CubeIDE

1. create new STM32 Project
2. add this folder "common" as linked folder
   1. right click on project -> New -> Folder
   2. Advanced -> Link to alternate location (Linked Folder)
   3. type "../common"
   4. Finish
3. add this folder "common" to 
   1. right click on project -> Properties
   2. C/C++ General -> Path and Symbols -> add
   3. Workspace -> choose the folder "common"
   4. will result in "/${ProjName}/common"
4. check that this folder "common" will get compiled
   1. right click on folder "common" -> Properties
   2. C/C++ Build -> uncheck option "Exclude resource from Build"
5. use #include "../common/HEADER-FILE.H"
