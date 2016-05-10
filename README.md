#Tangiable Holographic Sketch
This is a WIP project <br/>
if you encounter a problem, email Arc.

#Build 
##Clone and pull this repository
> git clone https://github.com/ArcEarth/TangibleHolographicSketch.git<br/>
> git submodule init DirectXTK<br/>
> git submodule update<br/>

##Tracking Hardware (All optional)
* Vicon Mocap
* Leap Motion
* Kinect 2 for Windows/XBox One

##Platform/Compilier
* Windows 8.1+, best with Windows 10
* Visual Studio 2015 (VC++ 14), best with Update 2

##SDK and Library and credits
* Kinect SDK 2.0+ (optional) [download](http://www.microsoft.com/en-us/download/details.aspx?id=44561)
* Leap SDK 2.31+ (optional) [download](http://1drv.ms/1NYFRGk)
* FBX SDK 2016+ (included)
+ Eigen 3.2.6+ (included)
+ DirectXTK (git submodule) 
+ GSL, C++ Core Guidelines Support Library, by Microsoft (included)
+ tinyXML2 (included)
+ tinyObjLoader (included)
+ Wink Signals (included)
+ iterator_range.h, by Google (included)

##Use Leap Motion or Microsoft Kinect 2
Define following MARCOS in [Envirument Variable](http://superuser.com/questions/949560/how-do-i-set-system-environment-variables-in-windows-10) $(PATH) 
* $(LeapSDK_Root) D:\SDKs\LeapSDK
* $(KINECTSDK20_DIR) C:\Program Files\Microsoft SDKs\Kinect\v2.0_1409\
