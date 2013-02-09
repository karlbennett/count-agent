count-agent
===========

A JVMTI agent that counts the creation and deletion of all objects within the attached JVM.

###### Usage
    LD_LIBRARY_PATH=/apth/to/agent/lib java -agentlib:count-agent JavaClass
