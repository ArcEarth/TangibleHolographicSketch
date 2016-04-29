#Tangiable Holographic Sketch
This is a WIP project <br/>
if you encounter a problem, email Arc.

#Build and Dependencies
##Tracking Hardware
* Vicon Mocap
* Leap Motion
* Kinect 2 for Windows/XBox One

##Platform/Compilier
* Windows 8.1+
* Visual Studio 2015 (VC++ 14)

##SDK and Library and credits
* FBX SDK 2016+ [download](http://download.autodesk.com/us/fbx_release_older/2016.1.2/fbx20161_2_fbxsdk_vs2015_win.exe)
* Kinect SDK 2.0+ [download](http://www.microsoft.com/en-us/download/details.aspx?id=44561)
* Leap SDK 2.31+ [download](http://1drv.ms/1NYFRGk)
+ Eigen 3.2.6+ (included)
+ DirectXTK (included) 
+ GSL, C++ Core Guidelines Support Library, by Microsoft (included)
+ tinyXML2 (included)
+ tinyObjLoader (included)
+ Wink Signals (included)
+ iterator_range.h, by Google (included)

##Define following MARCOS in [Envirument Variable](http://superuser.com/questions/949560/how-do-i-set-system-environment-variables-in-windows-10) $(PATH) 
* $(LeapSDK_Root) D:\SDKs\LeapSDK
* $(KINECTSDK20_DIR) C:\Program Files\Microsoft SDKs\Kinect\v2.0_1409\
* $(FBX_SDK_ROOT) C:\Program Files\Autodesk\FBX\FBX SDK\2016.1.2
