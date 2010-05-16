This is the DoIP (Domotics-over-IP) reference implementation by Pixelspark. It includes libraries that can be used for interfacing with the DoIP protocols, as well as some implementations of device gateways and end-user applications. It contains the following components:

- EPFramework (EP/EP): cross-platform reference implementation of DoIP protocol 
- TJFabric: fabric server which allows logic to be added to a DoIP network through scripting
- TJLEDEPServer: device gateway that links LED lighting devices (amBX, USP3, ...) to a DoIP network
- TJBluetoothServer: device gateway for Bluetooth devices
- TJRemote: iPhone remote for DoIP
- TJTrayRemote: OS X remote for DoIP

More information on the libraries can be found at http://developer.pixelspark.nl

The libraries can be built on OS X through the XCode project files. Under Linux, the scons build system is used. Note that you should either
modify these scripts to be independent or create a top-level SConstruct file to work. On Windows, you need to create a Visual Studio (2008, express
will also work) and add the .vcproj files as usual.

This DoIP implementation relies on CoreSpark (TJShared/TJScript/TJNP/TJScout in particular). CoreSpark is licensed under the LGPL. For more information, see http://developer.pixelspark.nl

The Libraries/ folder contains some third-party libraries used in several places; please read their licenses before using. When building, make sure
it is in the include path (as well as the Core/ directory, by the way).
