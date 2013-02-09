#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "count_agent_NewEvent.h"
#include "jvmti.h"


/**
 * Handle any JVMTI errors by printing their message then terminating.
 *
 * @param jvmti       a pointer to the JVMTI struct to allow access to the error API.
 * @param errorNumber the number of the error.
 * @param message     an optional error message
 */
void handleError(jvmtiEnv *jvmti, jvmtiError errorNumber, const char *message) {

    if (errorNumber == JVMTI_ERROR_NONE) return; // If this is not an error then don't handle it.

    char *errorString = NULL;

    (void)(*jvmti)->GetErrorName(jvmti, errorNumber, &errorString); // Get the error string.

    if (NULL == errorString) errorString = "Unknown";

    if (NULL == message) message = "";

    fprintf(stderr, "ERROR: JVMTI: %d(%s): %s\n", errorNumber, errorString, message);

    exit(3);
}


/**
 * This is the JNI native function that will be executed for every call to the
 * count.agent.NewEvent.nativeNewEvent(Object) method.
 *
 * @param env            the pointer to the JVM environment struct.
 * @param newEventClass  this is the struct that describes the count.agent.NewEvent Jave class.
 * @param newEventObject this is the struct that describes the new object instance that have just been created.
 */
JNIEXPORT void JNICALL Java_count_agent_NewEvent_nativeNewEvent(JNIEnv *env, jclass newEventClass, jobject newEventObject) {

    printf("New Event\n");
}

void JNICALL objectAllocCallBack(jvmtiEnv *jvmti, JNIEnv *env, jthread thread, jobject object, jclass klass, jlong size) {
}


/**
 * This function is called when the JVM starts and is used to initialise the JVMTI agent.
 *
 * @param jvm      the pointer to the struct that provides access to all the JVM specific functions.
 * @param options  this is a string that can be set when attaching an agent to the JVM
 *                  e.g. "java -agentlib:count-agent=some,option,string" would mean the string "some,option,string"
 *                  would be pointed to by the options argument.
 * @param reserved should not be used.
 */
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
	printf("Agent Started\n");

	jvmtiError error; // Any errors we encounter will be stored here.

    // Get the JVMTI environment struct so that we can configure the agent.
    jvmtiEnv *jvmti;
    (void)(*jvm)->GetEnv(jvm, (void**)&jvmti, JVMTI_VERSION_1_0);

    // Allocate a capabilities struct that will be used to request the JVMTI capabilities the agents requires.
    jvmtiCapabilities capabilities;
    (void)memset(&capabilities, 0, sizeof(capabilities));

    // Indicate the required capabilities.
    capabilities.can_tag_objects  = 1;
    capabilities.can_generate_object_free_events  = 1;
    capabilities.can_get_source_file_name  = 1;
    capabilities.can_generate_vm_object_alloc_events  = 1;

    // Attempt to enable the required capabilities.
    error = (*jvmti)->AddCapabilities(jvmti, &capabilities);
    handleError(jvmti, error, "Unable to get necessary JVMTI capabilities.");


    printf("Agent Initialised\n");

	return JNI_OK;
}   
