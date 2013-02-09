#include <stdio.h>
#include "count_agent_NewEvent.h"
#include "jvmti.h"

/**
 * This is the JNI native function that will be executed for every call to the
 * count.agent.NewEvent.nativeNewEvent(Object) method.
 *
 * @param env the pointer to the JVM environment struct.
 * @param newEventClass this is the struct that describes the count.agent.NewEvent Jave class.
 * @param newEventObject this is the struct that describes the new object instance that have just been created.
 */
JNIEXPORT void JNICALL Java_count_agent_NewEvent_nativeNewEvent(JNIEnv *env, jclass newEventClass, jobject newEventObject) {

    printf("New Event\n");
}

/**
 * This function is called when the JVM starts and is used to initialise the JVMTI agent.
 *
 * @param jvm the pointer to the struct that provides access to all the JVM specific functions.
 * @param options this is a string that can be set when attaching an agent to the JVM
 *                  e.g. "java -agentlib:count-agent=some,option,string" would mean the string "some,option,string"
 *                  would be pointed to by the options argument.
 * @param reserved should not be used.
 */
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
	printf("Agent Started\n");

	return JNI_OK;
}   
