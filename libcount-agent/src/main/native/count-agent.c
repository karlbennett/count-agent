#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
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
 * Get the name of the supplied jclass in the standard Java package notation. e.g. java.lang.String
 *
 * @param jvmti a pointer to the JVMTI struct to allow access to the error API.
 * @param klass the class to get the class name for.
 * @return a string containing the class name.
 */
char* getClassName(jvmtiEnv *jvmti, jclass klass) {

    jvmtiError error;

    char *signature = NULL;
    error = (*jvmti)->GetClassSignature(jvmti, klass, &signature, NULL);

    // Check to see if the supplied class is an array, if it is the class name should have an array suffix "[]".
    char *signatureSuffix = strchr(signature, '[') ? "[]" : "";

    // Find the index of the 'L' character that denotes a fully qualified class.
    char *signatureMinusPrefix = strchr(signature, 'L');

    // We only support class names for now.
    if (NULL == signatureMinusPrefix) return NULL;

    signatureMinusPrefix++; // Then move passed it to get to the class name.

    // Find the semicolon so that we can remove it from the end of the string.
    char *semicolon = strrchr(signatureMinusPrefix, ';');

    // Get the size of the className string by calculating the difference between the semicolon and signatureMinusPrefix
    // pointer values and adding 1 for the null character.
    jlong size = (semicolon - signatureMinusPrefix) + 1;

    // Allocate memory for the className string.
    char *className = NULL;
    error = (*jvmti)->Allocate(jvmti, size, (unsigned char**) &className);
    handleError(jvmti, error, "Unable to generic signature.");

    // Populate the classANme string and set it's null character.
    strncpy(className, signatureMinusPrefix, size - 1);
    className[size - 1] = '\0';

    // Now that we are finished with the signature string we should clean it up.
    error = (*jvmti)->Deallocate(jvmti, (unsigned char*) signature);
    handleError(jvmti, error, "Unable to deallocate signature.");

    // Replace the '/''s in the className with '.''s.
    char *forwardSlash = NULL;
    while (forwardSlash = strrchr(className, '/')) *forwardSlash = '.';

    return className;
}

/**
 * Print a CSV line containing the supplied operation (ADD, DELETE) and class name.
 *
 * @param operation the operation that was carried out on the class.
 * @param className the name of the class.
 */
void printCSV(char *operation, char *className) {

    struct timeval now;
    gettimeofday(&now, NULL);

    printf("%d,%s,%s\n", now.tv_usec, operation, className);
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


/**
 * A callback that will be registered to the JVMTI_EVENT_OBJECT_FREE event which will call it when ever a tagged object
 * has been freed.
 *
 * @param jvmti a pointer to the JVMTI struct to allow access to the error API.
 * @param tag   the tag of the object that has been freed.
 */
void JNICALL objectFreeCallBack(jvmtiEnv *jvmti, jlong tag) {

    printf("Object Freed\n");
}

/**
 * A callback that will be registered to the JVMTI_EVENT_VM_OBJECT_ALLOC event which will call it when ever an object is
 * constructed that can't be instrumented.
 *
 * @param jvmti  a pointer to the JVMTI struct to allow access to the error API.
 * @param env    the pointer to the JVM environment struct.
 * @param thread a struct that represents the thread that the object has been created in.
 * @param object a struct that represents the object that has been allocated.
 * @param klass  a struct that represents the class of the allocated object.
 * @param size   the size of the allocated object.
 */
void JNICALL objectAllocCallBack(jvmtiEnv *jvmti, JNIEnv *env, jthread thread, jobject object, jclass klass, jlong size) {

    char *className = getClassName(jvmti, klass);

    if (className) {

        printCSV("ADD", className);

        jvmtiError  error = (*jvmti)->Deallocate(jvmti, (unsigned char*) className);
        handleError(jvmti, error, "Unable to deallocate className.");
    }
}


/**
 * This function is called when the JVM starts and is used to initialise the JVMTI agent.
 *
 * @param jvm      the pointer to the struct that provides access to all the JVM specific functions.
 * @param options  this is a string that can be set when attaching an agent to the JVM
 *                  e.g. "java -agentlib:count-agent=some,option,string" would mean the string "some,option,string"
 *                  would be pointed to by the options argument.
 * @param reserved should not be used.
 * @return the JVMTI agent start statuc code.
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

    // Allocate the callbacks struct, this will hold the callback functions that will be passed into JVMTI.
    jvmtiEventCallbacks callbacks;
    (void)memset(&callbacks, 0, sizeof(callbacks));

    // Asign the callbacks.
    callbacks.ObjectFree = &objectFreeCallBack; // JVMTI_EVENT_OBJECT_FREE
    callbacks.VMObjectAlloc = &objectAllocCallBack; // JVMTI_EVENT_VM_OBJECT_ALLOC

    // Add the callbacks.
    error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, (jint)sizeof(callbacks));
    handleError(jvmti, error, "Cannot set jvmti callbacks");


    // Enable the object free and allocation events.
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_OBJECT_FREE, (jthread)NULL);
    handleError(jvmti, error, "Cannot set event notification");

    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_OBJECT_ALLOC, (jthread)NULL);
    handleError(jvmti, error, "Cannot set event notification");

    printf("Agent Initialised\n");

	return JNI_OK;
}   
